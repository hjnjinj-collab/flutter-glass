add_library(native_glass_plugin SHARED
  "native_glass.cpp"
  "d3d11_renderer.cpp"
)

target_include_directories(native_glass_plugin PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

set_target_properties(native_glass_plugin PROPERTIES PREFIX "")

target_link_libraries(native_glass_plugin PRIVATE flutter d3d11 d3dcompiler dxgi)

set_target_properties(native_glass_plugin PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
