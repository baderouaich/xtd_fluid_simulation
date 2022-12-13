#include "main_form.hpp"

using namespace xtd;
using namespace xtd::drawing;
using namespace xtd::forms;
using namespace xtd_fluid_simulation;

main_form::main_form() :
  m_animation(new animation()),
  m_fluid(new Fluid()),
  m_previous_mouse_position(0, 0),
  m_velocity(0.0f, 0.0f)
{
  text("Fluid Simulation");
  client_size({ Fluid::N * Fluid::SCALE + 200, Fluid::N * Fluid::SCALE });
  minimum_client_size(client_size());
  maximum_client_size(client_size());
  maximize_box(false);

  //back_color(xtd::drawing::color::light_gray);
  //fore_color(color::cyan);

  m_animation->parent(*this);
  m_animation->location({ 0, 0 });
  m_animation->size({ Fluid::N * Fluid::SCALE, Fluid::N * Fluid::SCALE });
  m_animation->dock(dock_style::left);
  m_animation->back_color(color::black);
  m_animation->frames_per_second(60);
  m_animation->start();
  m_animation->updated += animation_updated_event_handler(*this, &main_form::on_animation_update);
  m_animation->paint += paint_event_handler(*this, &main_form::on_animation_draw);
  m_animation->mouse_move += mouse_event_handler(*this, &main_form::on_animation_mouse_move);
  
  m_vlayout.parent(*this);
  m_vlayout.dock(dock_style::right);
  m_vlayout.width(200);
  m_vlayout.padding(forms::padding(2));

  m_bcp_label.parent(m_vlayout);
  m_bcp_label.text("Background Color:");
  m_cp_background_color.parent(m_vlayout);
  m_cp_background_color.width(180);
  m_cp_background_color.color(m_animation->back_color());
  m_cp_background_color.color_picker_changed += [&](object& sender, const color_picker_event_args& e){
    m_animation->back_color(e.color());
  };

  m_cp_label.parent(m_vlayout);
  m_cp_label.text("Fluid Color:");
  m_cp_fluid_color.parent(m_vlayout);
  m_cp_fluid_color.width(180);
  m_cp_fluid_color.color(m_fluid->get_color());
  m_cp_fluid_color.color_picker_changed += [&](object& sender, const color_picker_event_args& e) {
    m_fluid->set_color(e.color());
  };


  m_tb_label.parent(m_vlayout);
  m_tb_label.text("Motion Speed:");
  m_tb_speed.parent(m_vlayout);
  m_tb_speed.width(180);
  m_tb_speed.value(m_fluid->get_speed());
  m_tb_speed.minimum(m_fluid->get_min_speed());
  m_tb_speed.maximum(m_fluid->get_max_speed());
  m_tb_speed.value_changed += [&] {
    m_fluid->set_speed(m_tb_speed.value());
  };
  
  m_vlx_label.parent(m_vlayout);
  m_vlx_label.text("Velocity X:");
  m_tb_velocity_x.parent(m_vlayout);
  m_tb_velocity_x.width(180);
  m_tb_velocity_x.minimum(-100);
  m_tb_velocity_x.maximum(100);
  m_tb_velocity_x.value(0);
  m_tb_velocity_x.value_changed += [&] {
    m_velocity.x(m_tb_velocity_x.value() * 0.05f);
  };

  m_vly_label.parent(m_vlayout);
  m_vly_label.text("Velocity Y:");
  m_tb_velocity_y.width(180);
  m_tb_velocity_y.parent(m_vlayout);
  m_tb_velocity_y.minimum(-100);
  m_tb_velocity_y.maximum(100);
  m_tb_velocity_y.value(0);
  m_tb_velocity_y.value_changed += [&] {
    m_velocity.y(m_tb_velocity_y.value() * 0.05f);
  }; 
  
  m_density_label.parent(m_vlayout);
  m_density_label.text("Density (dye amount):");
  m_density_label.width(180);
  m_tb_density.parent(m_vlayout);
  m_tb_density.width(180);
  m_tb_density.minimum(500);
  m_tb_density.maximum(3000);
  m_tb_density.value(1000);


  m_auto_density_label.parent(m_vlayout);
  m_auto_density_label.text("Automatic Density:");
  m_auto_density_label.width(180);
  m_sb_auto_density.parent(m_vlayout);
  m_sb_auto_density.auto_check(true);
  m_sb_auto_density.checked(true);


  m_btn_reset.parent(m_vlayout);
  m_btn_reset.width(180);
  m_btn_reset.text("Reset to defaults");
  m_btn_reset.click += [&] {
    m_cp_background_color.color(color::black);
    m_cp_fluid_color.color(color::cyan);
    m_tb_speed.value(7);
    m_tb_velocity_x.value(0);
    m_tb_velocity_y.value(0);
    m_tb_density.value(1000);
    m_sb_auto_density.checked(false);
  };

}

void main_form::on_animation_update(object& sender, const animation_updated_event_args& e) {
  const float delta_time = e.elapsed_milliseconds() / 1000.0f;

  // If left mouse button is pressed (over the animation), add some of dye at that location
  if (m_animation->mouse_buttons() == mouse_buttons::left)
  {
    // Add some of dye in held location
    m_fluid->AddDensity(static_cast<int>(m_mouse_position.x() / Fluid::SCALE), static_cast<int>(m_mouse_position.y() / Fluid::SCALE), static_cast<float>(m_tb_density.value()));
    // note that the position bellow is from m_animation not the main form; equiv: m_animation->mouse_position()
    // Apply Mouse Drag Velocity to simulate fluid movement
    const float amount_x = static_cast<float>(m_mouse_position.x()) - m_previous_mouse_position.x();
    const float amount_y = static_cast<float>(m_mouse_position.y()) - m_previous_mouse_position.y();
    m_fluid->AddVelocity(static_cast<int>(m_mouse_position.x() / Fluid::SCALE), static_cast<int>(m_mouse_position.y() / Fluid::SCALE), amount_x, amount_y);
    m_previous_mouse_position = m_mouse_position;
  }

  // Apply user input velocity 
  if(m_velocity != m_velocity.empty)
    for (int j = 0; j < Fluid::N; ++j)
    {
      for (int i = 0; i < Fluid::N; ++i)
      {
        const int x = i * Fluid::SCALE;
        const int y = j * Fluid::SCALE;
        m_fluid->AddVelocity(x, y, m_velocity.x() * delta_time , m_velocity.y() * delta_time);
      }
    }

  // Add automatic density at center if switch_button is on
  if (m_sb_auto_density.checked()) {
    const int center_x = (m_animation->width() / 2) / Fluid::SCALE;
    const int center_y = (m_animation->height() / 2) / Fluid::SCALE;
    m_fluid->AddDensity(center_x, center_y, static_cast<float>(m_tb_density.value()));
    m_fluid->AddVelocity(center_x, center_y, random(-3.0f, 3.0f), random(-3.0f, 3.0f));
  }

  // Update Fluid
  m_fluid->Update(delta_time);
}

void main_form::on_animation_draw(object& sender, paint_event_args& e) {
  graphics& gfx = e.graphics();
  // TODO: graphics settings to enhance performance
  //gfx.page_unit(graphics_unit::pixel); // using pixel mode
  //gfx.pixel_offset_mode(drawing2d::pixel_offset_mode::high_speed); // as fast as possible
  //gfx.smoothing_mode(drawing2d::smoothing_mode::high_speed); // as fast as possible
  gfx.clear(m_animation->back_color());

  // Draw fluid particles (as small rectangles, with different alpha color)
  static drawing::solid_brush particle_brush(m_fluid->get_color());
  for (int j = 0; j < Fluid::N; ++j)
  {
    for (int i = 0; i < Fluid::N; ++i)
    {
      const int x = i * Fluid::SCALE;
      const int y = j * Fluid::SCALE;
      particle_brush.color(m_fluid->get_color_at(i, j));
      gfx.fill_rectangle(particle_brush, x, y, Fluid::SCALE, Fluid::SCALE);
    }
  }

}

void main_form::on_animation_mouse_move(object& sender, const mouse_event_args& e) {
  m_mouse_position = e.location();
}

void main_form::main() {
  const std::unique_ptr<main_form> main_form_ptr(new main_form());
  xtd::forms::application::run(*main_form_ptr);
}