/// @file
/// @brief Contains form1 class.
#pragma once
#include <xtd/xtd.h>
#include "fluid.hpp"

/// @brief Represents the namespace that contains application objects.
namespace xtd_fluid_simulation {

  /// @brief Represents the main form.
  class main_form : public xtd::forms::form {
  public:
    /// @brief Initializes a new instance of the form1 class.
    main_form();

    /// @brief The main entry point for the application.
    static void main();

  private: // Update & Draw 
    void on_animation_update(xtd::object& sender, const xtd::forms::animation_updated_event_args& e);
    void on_animation_draw(xtd::object& sender, xtd::forms::paint_event_args& e);

  private: // Events
    void on_animation_mouse_move(xtd::object& sender, const xtd::forms::mouse_event_args& e);

  private:
    std::unique_ptr<xtd::forms::animation> m_animation;
    std::unique_ptr<Fluid> m_fluid;
    xtd::drawing::point m_mouse_position;
    xtd::drawing::point m_previous_mouse_position;
    xtd::drawing::point_f m_velocity;

    xtd::forms::vertical_layout_panel m_vlayout;
    xtd::forms::label m_tb_label;
    xtd::forms::track_bar m_tb_speed;

    xtd::forms::label m_cp_label;
    xtd::forms::color_picker m_cp_fluid_color;

    xtd::forms::label m_bcp_label;
    xtd::forms::color_picker m_cp_background_color;

    xtd::forms::label m_vlx_label;
    xtd::forms::track_bar m_tb_velocity_x;
    xtd::forms::label m_vly_label;
    xtd::forms::track_bar m_tb_velocity_y;

    xtd::forms::label m_density_label;
    xtd::forms::track_bar m_tb_density;

    xtd::forms::label m_auto_density_label;
    xtd::forms::switch_button m_sb_auto_density;

    xtd::forms::button m_btn_reset;


    template<typename T>
    inline static T random(const T& min, const T& max) noexcept {
      static std::random_device seed{};
      static std::default_random_engine gen{ seed() };
      if constexpr (std::is_floating_point_v<T>) {
        return std::uniform_real_distribution<T>(min, max)(gen);
      }
      else {
        return std::uniform_int_distribution<T>(min, max)(gen);
      }
    }
  };
}
