# Native plugin example (Dart) showing how to call the native plugin and display Texture.

This example assumes the native plugin exposes a MethodChannel 'native_glass' with method 'getTextureId' returning
an int64 texture id, and methods 'start'/'stop' to control the render loop.

Use this example as a starting point. The native side is still a skeleton and requires implementation.
