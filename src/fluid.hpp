#pragma once
#include <xtd/xtd.h>
#include <array>

namespace xtd_fluid_simulation {
	/// <summary>
	/// Source: https://mikeash.com/pyblog/fluid-simulation-for-dummies.html
	/// </summary>
	class Fluid {
	public:
		Fluid();
		~Fluid();

	public:
		// Update Fluid each frame
		void Update(const float dt) noexcept;

		// Adds density (aka dye) in a specific location in fluid
		void AddDensity(int x, int y, float amount) noexcept;

		// Adds velocity in a specific location in fluid
		void AddVelocity(int x, int y, float amountX, float amountY) noexcept;

		/**
		*	Diffuse is really simple; it just precalculates a value and passes everything off to LinearSolve.
		*	So that means, while I know what it does, I don't really know how,
		*	since all the work is in that mysterious function.
		*
		* @info: Put a drop of soy sauce in some water, and you'll notice that it doesn't stay still, but it spreads out.
		*	This happens even if the water and sauce are both perfectly still. This is called diffusion.
		*	We use diffusion both in the obvious case of making the dye spread out, and also in the less obvious case of making the velocities of the fluid spread out.
		*/
		void Diffuse(int b, float* x, float* x0, float diff, float dt) noexcept;

		/**
		*	this function is mysterious, but it does some kind of solving.
		*	this is done by running through the whole array and setting each
		*	cell to a combination of its neighbors. It does this several times;
		*	the more iterations it does, the more accurate the results,
		*	but the slower things run. In the step function above,
		*	four iterations are used. After each iteration, it resets the
		*	boundaries so the calculations don't explode.
		*
		* @info: I honestly don't know exactly what this function does or how it works. What I do know is that it's used for both diffuse and project.
		* It's solving a linear differential equation of some sort, although how and what is not entirely clear to me.
		*/
		void LinearSolve(int b, float* x, float* x0, float a, float c) noexcept;

		/**
		*	As noted above, this function sets the boundary cells at the outer edges of the this so they perfectly counteract their neighbors.
		*	There's a bit of oddness here which is, what is this b pramaeter? It can be 0, 1, 2, or 3, and each value has a special meaning which is not at all obvious. The answer lies is what kind of data can be passed into this function.
		*
		*	We have four different kinds of data (x, y, and z velocities, plus density) floating around, and any of those four can be passed in to set_bnd. You may notice that this is exactly the number of values this parameter can have, and this is not a coincidence.
		*
		*	Let's look at a boundary cell (horrible ASCII art warning):
		*
		*	+---+---+
		*	|   |^  |
		*	|   |   |
		*	|   |   |
		*	+---+---+
		*	Here we're at the left edge of the this. The left cell is the boundary cell that needs to counteract its neighbor, the right cell. The right cell has a velocity that's up and to the left.
		*	The boundary cell's x velocity needs to be opposite its neighbor, but its y velocity needs to be equal to its neighbor. This will produce a result like so:
		*
		*	+---+---+
		*	|  ^|^  |
		*	| / |   |
		*	|/  |   |
		*	+---+---+
		*	This counteracts the motion of the fluid which would take it through the wall, and preserves the rest of the motion. So you can see that what action is taken depends on which array we're dealing with; if we're dealing with x velocities, then we have to set the boundary cell's value to the opposite of its neighbor, but for everything else we set it to be the same.
		*	That is the answer to the mysterious b. It tells the function which array it's dealing with, and so whether each boundary cell should be set equal or opposite its neighbor's value.
		*
		*	This function also sets corners. This is done very simply, by setting each corner cell equal to the average of its three neighbors.
		*
		* @info: This is short for "set bounds", and it's a way to keep fluid from leaking out of your box.
		* Not that it could really leak, since it's just a simulation in memory, but not having walls really screws up the simulation code.
		* Walls are added by treating the outer layer of cells as the wall. Basically, every velocity in the layer next to this outer layer is mirrored.
		* So when you have some velocity towards the wall in the next-to-outer layer, the wall gets a velocity that perfectly counters it.
		*/
		void SetBoundary(int b, float* x) noexcept;

		/**
		*	This function is also somewhat mysterious as to exactly how it works,
		*	but it does some more running through the dataand setting values,
		*	with some calls to LinearSolve thrown in for fun.
		*
		* @info: Remember when I said that we're only simulating incompressible fluids? This means that the amount of fluid in each box has to stay constant.
		* That means that the amount of fluid going in has to be exactly equal to the amount of fluid going out.
		* The other operations tend to screw things up so that you get some boxes with a net outflow, and some with a net inflow.
		* This operation runs through all the cells and fixes them up so everything is in equilibrium.
		*/
		void Project(float* velocX, float* velocY, float* p, float* div) noexcept;

		/**
		*	This function is responsible for actually moving things around.
		*	To that end, it looks at each cell in turn.In that cell,
		*	it grabs the velocity, follows that velocity back in time,
		*	and sees where it lands.It then takes a weighted average
		*	of the cells around the spot where it lands, then applies
		*	that value to the current cell.
		*
		* @info: Every cell has a set of velocities, and these velocities make things move. This is called advection.
		* As with diffusion, advection applies both to the dye and to the velocities themselves.
		**/
		void Advect(int b, float* d, float* d0, float* velocX, float* velocY, float dt) noexcept;



		//#include <intrin.h>
		//static inline float fast_min(float a, float b) noexcept
		//{
		//	// Branchless SSE min.
		//	_mm_store_ss(&a, _mm_min_ss(_mm_set_ss(a), _mm_set_ss(b)));
		//	return a;
		//}
		//static inline float fast_max(float a, float b) noexcept
		//{
		//	// Branchless SSE max.
		//	_mm_store_ss(&a, _mm_max_ss(_mm_set_ss(a), _mm_set_ss(b)));
		//	return a;
		//}
		//static inline float fast_clamp(float val, float minval, float maxval) noexcept
		//{
		//	// Branchless SSE clamp.
		//	// return minss( maxss(val,minval), maxval );
		//	_mm_store_ss(&val, _mm_min_ss(_mm_max_ss(_mm_set_ss(val), _mm_set_ss(minval)), _mm_set_ss(maxval)));
		//	return val;
		//}


	public:
		/**
		* Converts 2D coords into 1D ( x,y into index )
		*/
		inline static constexpr int IX(int x, int y) noexcept
		{
			// i was using std::clamp, although to optimize things up, minmax clamp is 4.1 times faster than std::clamp -> https://quick-bench.com/q/x7RbIo-YFpEKkvFbbQGqsLREKQQ
			x = std::min(std::max(x, 0), N - 1);
			y = std::min(std::max(y, 0), N - 1);
			return x + (y * N);
		}

	public:
		// Get the color of a particle at a specific location in fluid
		xtd::drawing::color get_color_at(const int i, const int j) noexcept {
			// Get Density 0 -> 255 alpha (background color)
			float density = m_density[IX(i, j)];
			// So that fluid color will not turn into black when adding too much density
			density = std::min(std::max(density, 0.0f), 255.0f); //std::clamp(density, 0.0f, 255.0f);
			// Construct color based on density alpha
			return xtd::drawing::color::from_argb(static_cast<std::uint8_t>(density), m_fluid_color.r(), m_fluid_color.g(), m_fluid_color.b());
		}

		const xtd::drawing::color& get_color() const noexcept { return m_fluid_color; }
		void set_color(const xtd::drawing::color& color) noexcept {
			m_fluid_color = color;
		}

		// Motion speed attr
		void set_speed(const int speed) noexcept { m_speed = speed; }
		constexpr int get_speed() const noexcept { return m_speed; }
		constexpr int get_max_speed() const noexcept { return 20; }
		constexpr int get_min_speed() const noexcept { return 0; }

	public:
		inline static constexpr const int N = 120;    // Number of particles
		inline static constexpr const int SCALE = 5;  // Size of particles (w,h) (the smaller the rect, the more realistic simulation, the more slower performance..)


	private: // No stack over flow please OS!
		std::array<float, N * N> m_fluid_particles; // Fluid particles
		std::array<float, N * N> m_density; // Density (aka amount of dye)
		
		std::array<float, N * N> m_velocity_x; // velocity X for each fluid particle
		std::array<float, N * N> m_velocity_y; // velocity Y for each fluid particle
		
		std::array<float, N * N> m_prev_velocity_x; // previous velocity X
		std::array<float, N * N> m_prev_velocity_y; // previous velocity Y

		int m_speed = 7; // Speed of fluid
		float m_motion_speed = 0.2f; // Motion speed of fluid (speed * delta_time)
		float m_vescosity = 0.0000001f; // Thickness of fluid
		float m_diffusion = 0.000001f; // Diffusion of fluid (the more value, the longer the fluid will keep difussing around)
		int m_iterations = 32; // Number of iterations (the more iterations, the more realistic fluid behavior we get. although frame rate reduces with more iterations...)
		
		xtd::drawing::color m_fluid_color = xtd::drawing::color::cyan;
	};
}
