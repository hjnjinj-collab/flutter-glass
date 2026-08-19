import 'dart:typed_data';
import 'dart:ui' as ui;
import 'package:flutter/material.dart';

class LiquidGlassPainter extends CustomPainter {
  final ui.FragmentProgram? program;
  final ui.Image? sceneImage;

  LiquidGlassPainter({required this.program, this.sceneImage});

  @override
  void paint(Canvas canvas, Size size) {
    if (program == null) {
      final bgPaint = Paint()..color = Colors.blueGrey.shade900;
      canvas.drawRect(Offset.zero & size, bgPaint);
      return;
    }

    // Prepare float uniforms: resolution + params
    final floats = Float32List.fromList([
      size.width,
      size.height, // u_resolution
      0.06, // u_refractionStrength
      1.4, // u_sdfScale
      0.08, // u_edgeSoftness
      0.6, // u_glowStrength
    ]);

    ui.Shader shader;
    if (sceneImage != null) {
      // Pass scene image as a sampler uniform to the fragment shader.
      // FragmentProgram.shader accepts samplerUniforms as a list of Images.
      shader = program!.shader(floatUniforms: floats, samplerUniforms: [sceneImage!]);
    } else {
      // Fallback: no scene image yet — run shader without samplers
      shader = program!.shader(floatUniforms: floats);
    }

    final paint = Paint()..shader = shader;
    canvas.drawRect(Offset.zero & size, paint);
  }

  @override
  bool shouldRepaint(covariant LiquidGlassPainter oldDelegate) {
    return oldDelegate.program != program || oldDelegate.sceneImage != sceneImage;
  }
}
