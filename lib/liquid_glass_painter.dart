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

    ui.Shader? shader;

    // Try multiple FragmentProgram invocation names to maintain compatibility across SDKs
    final p = program!;
    try {
      shader = (p as dynamic).shader(floatUniforms: floats, samplerUniforms: sceneImage != null ? [sceneImage!] : null) as ui.Shader;
    } catch (_) {}
    if (shader == null) {
      try {
        shader = (p as dynamic).fragmentShader(floatUniforms: floats, samplerUniforms: sceneImage != null ? [sceneImage!] : null) as ui.Shader;
      } catch (_) {}
    }
    if (shader == null) {
      try {
        shader = (p as dynamic).instantiate(floatUniforms: floats, samplerUniforms: sceneImage != null ? [sceneImage!] : null) as ui.Shader;
      } catch (_) {}
    }

    if (shader == null) {
      // Fallback background
      final bgPaint = Paint()..color = Colors.blueGrey.shade900;
      canvas.drawRect(Offset.zero & size, bgPaint);
      return;
    }

    final paint = Paint()..shader = shader;
    canvas.drawRect(Offset.zero & size, paint);
  }

  @override
  bool shouldRepaint(covariant LiquidGlassPainter oldDelegate) {
    return oldDelegate.program != program || oldDelegate.sceneImage != sceneImage;
  }
}
