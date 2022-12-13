cmake_minimum_required(VERSION 3.3)

# Project
project(xtd_fluid_simulation)
find_package(xtd REQUIRED)
add_sources(
  src/fluid.hpp
  src/fluid.cpp
  src/main_form.hpp
  src/main_form.cpp
)
target_type(GUI_APPLICATION)
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/Ox /fp:fast /Oi /Ot>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Ofast -march=native>
)

# Install
install_component()
