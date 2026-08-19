# CMakeLists.txt for native_glass Windows plugin (skeleton)
cmake_minimum_required(VERSION 3.10)
project(native_glass_windows)

# This is a skeleton CMakeLists for the Windows plugin. It's intentionally minimal —
# you may prefer to integrate this into your Visual Studio solution or the
# flutter desktop plugin build system.

add_library(native_glass SHARED
  native_glass.cpp
)

# TODO: add include paths and link against D3D11, DXGI, and Flutter libraries
# target_include_directories(native_glass PRIVATE ${FLUTTER_INCLUDE_DIRS})
# target_link_libraries(native_glass PRIVATE d3d11 dxgi flutter)
