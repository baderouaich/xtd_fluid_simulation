#include "fluid.hpp"
#include <cmath>	// std::floor
using namespace xtd_fluid_simulation;

Fluid::Fluid()
	:
	m_fluid_particles{ 0.0f },
	m_density{ 0.0f },
	m_velocity_x{ 0.0f },
	m_velocity_y{ 0.0f },
	m_prev_velocity_x{ 0.0f },
	m_prev_velocity_y{ 0.0f }
{
	std::memset(m_fluid_particles.data(), 0, m_fluid_particles.size());
	std::memset(m_density.data(), 0, m_density.size());
	std::memset(m_velocity_x.data(), 0, m_velocity_x.size());
	std::memset(m_velocity_y.data(), 0, m_velocity_y.size());
	std::memset(m_prev_velocity_x.data(), 0, m_prev_velocity_x.size());
	std::memset(m_prev_velocity_y.data(), 0, m_prev_velocity_y.size());
}

void Fluid::Update(const float dt) noexcept
{
	m_motion_speed = m_speed * dt;
	Diffuse(1, m_prev_velocity_x.data(), m_velocity_x.data(), m_vescosity, m_motion_speed);
	Diffuse(2, m_prev_velocity_y.data(), m_velocity_y.data(), m_vescosity, m_motion_speed);

	Project(m_prev_velocity_x.data(), m_prev_velocity_y.data(), m_velocity_x.data(), m_velocity_y.data());

	Advect(1, m_velocity_x.data(), m_prev_velocity_x.data(), m_prev_velocity_x.data(), m_prev_velocity_y.data(), m_motion_speed);
	Advect(2, m_velocity_y.data(), m_prev_velocity_y.data(), m_prev_velocity_x.data(), m_prev_velocity_y.data(), m_motion_speed);

	Project(m_velocity_x.data(), m_velocity_y.data(), m_prev_velocity_x.data(), m_prev_velocity_y.data());

	Diffuse(0, m_fluid_particles.data(), m_density.data(), m_diffusion, m_motion_speed);
	Advect(0, m_density.data(), m_fluid_particles.data(), m_velocity_x.data(), m_velocity_y.data(), m_motion_speed);
}

void Fluid::AddDensity(int x, int y, float amount) noexcept
{
	this->m_density[IX(x, y)] += amount;
}

void Fluid::AddVelocity(int x, int y, float amountX, float amountY) noexcept
{
	const int index = IX(x, y);

	this->m_velocity_x[index] += amountX;
	this->m_velocity_y[index] += amountY;
}

void Fluid::Diffuse(int b, float* x, float* x0, float diff, float dt) noexcept
{
	const float a = dt * diff * (N - 2) * (N - 2);
	LinearSolve(b, x, x0, a, 1 + SCALE * a);
}

void Fluid::LinearSolve(int b, float* x, float* x0, float a, float c) noexcept
{
	const float cRecip = 1.0f / c;
	for (int k = 0; k < m_iterations; k++)
	{
		for (int j = 1; j < N - 1; j++)
		{
			for (int i = 1; i < N - 1; i++)
			{
				x[IX(i, j)] =
					(x0[IX(i, j)]
						+ a * (x[IX(i + 1, j)]
							+ x[IX(i - 1, j)]
							+ x[IX(i, j + 1)]
							+ x[IX(i, j - 1)]
							)) * cRecip;
			}
		}

		SetBoundary(b, x);
	}
}

void Fluid::SetBoundary(int b, float* x) noexcept
{
	for (int i = 1; i < N - 1; i++)
	{
		x[IX(i, 0)] = b == 2 ? -x[IX(i, 1)] : x[IX(i, 1)];
		x[IX(i, N - 1)] = b == 2 ? -x[IX(i, N - 2)] : x[IX(i, N - 2)];
	}
	for (int j = 1; j < N - 1; j++)
	{
		x[IX(0, j)] = b == 1 ? -x[IX(1, j)] : x[IX(1, j)];
		x[IX(N - 1, j)] = b == 1 ? -x[IX(N - 2, j)] : x[IX(N - 2, j)];
	}

	x[IX(0, 0)] = 0.33f * (x[IX(1, 0)] + x[IX(0, 1)]);
	x[IX(0, N - 1)] = 0.33f * (x[IX(1, N - 1)] + x[IX(0, N - 2)]);
	x[IX(N - 1, 0)] = 0.33f * (x[IX(N - 2, 0)] + x[IX(N - 1, 1)]);
	x[IX(N - 1, N - 1)] = 0.33f * (x[IX(N - 2, N - 1)] + x[IX(N - 1, N - 2)]);

}

void Fluid::Project(float* velocX, float* velocY, float* p, float* div) noexcept
{
	for (int j = 1; j < N - 1; j++) {
		for (int i = 1; i < N - 1; i++) {
			div[IX(i, j)] = -0.5f * (
				velocX[IX(i + 1, j)]
				- velocX[IX(i - 1, j)]
				+ velocY[IX(i, j + 1)]
				- velocY[IX(i, j - 1)]
				) / N;
			p[IX(i, j)] = 0;
		}
	}

	SetBoundary(0, div);
	SetBoundary(0, p);
	LinearSolve(0, p, div, 1, 4);

	for (int j = 1; j < N - 1; j++) {
		for (int i = 1; i < N - 1; i++) {
			velocX[IX(i, j)] -= 0.5f * (p[IX(i + 1, j)]
				- p[IX(i - 1, j)]) * N;
			velocY[IX(i, j)] -= 0.5f * (p[IX(i, j + 1)]
				- p[IX(i, j - 1)]) * N;
		}
	}
	SetBoundary(1, velocX);
	SetBoundary(2, velocY);

}

void Fluid::Advect(int b, float* d, float* d0, float* velocX, float* velocY, float dt) noexcept
{
	float i0, i1, j0, j1;

	const float dtx = dt * (N - 2);
	const float dty = dt * (N - 2);

	float s0, s1, t0, t1;
	float tmp1, tmp2, x, y;

	constexpr float Nfloat = static_cast<float>(N);
	float ifloat, jfloat;
	int i, j;

	for (j = 1, jfloat = 1; j < N - 1; j++, jfloat++)
	{
		for (i = 1, ifloat = 1; i < N - 1; i++, ifloat++)
		{
			const int index = IX(i, j);

			tmp1 = dtx * velocX[index];
			tmp2 = dty * velocY[index];

			x = ifloat - tmp1;
			y = jfloat - tmp2;

			if (x < 0.5f) x = 0.5f;
			if (x > Nfloat + 0.5f) x = Nfloat + 0.5f;
			i0 = std::floor(x);
			i1 = i0 + 1.0f;
			if (y < 0.5f) y = 0.5f;
			if (y > Nfloat + 0.5f) y = Nfloat + 0.5f;
			j0 = std::floor(y);
			j1 = j0 + 1.0f;


			s1 = x - i0;
			s0 = 1.0f - s1;
			t1 = y - j0;
			t0 = 1.0f - t1;


			int i0i = static_cast<int>(i0);
			int i1i = static_cast<int>(i1);
			int j0i = static_cast<int>(j0);
			int j1i = static_cast<int>(j1);

			d[index] =
				s0 * (t0 * d0[IX(i0i, j0i)] + t1 * d0[IX(i0i, j1i)]) +
				s1 * (t0 * d0[IX(i1i, j0i)] + t1 * d0[IX(i1i, j1i)]);

		}
	}
	SetBoundary(b, d);
}

Fluid::~Fluid() {}
