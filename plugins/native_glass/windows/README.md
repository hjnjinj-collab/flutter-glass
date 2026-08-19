# Native glass plugin (Windows)

This folder contains a minimal Windows plugin skeleton for a native GPU-rendered liquid glass effect. The initial commit is a skeleton: registration, a TextureEntry placeholder, and a place to implement D3D11 (or ANGLE) rendering.

Build notes (skeleton):
- This is not a finished plugin — it's an integration scaffold to be filled with rendering code.
- On Windows we will use the Flutter desktop plugin APIs and register a texture via TextureRegistrar.
- A working implementation requires adding shader compilation and a GPU render loop.

Planned files:
- native_glass.h / native_glass.cpp        (plugin entry + texture management)
- shaders/liquid_glass.hlsl / .hlsl.glsl   (native shader files)
- CMakeLists.txt (or Visual Studio project guidance)
