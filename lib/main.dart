import 'dart:ui' as ui;
import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';

import 'liquid_glass_painter.dart';

void main() {
  runApp(const GlassApp());
}

class GlassApp extends StatelessWidget {
  const GlassApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Impeller Liquid Glass',
      home: Scaffold(
        backgroundColor: Colors.black,
        body: Center(
          child: AspectRatio(
            aspectRatio: 16 / 9,
            child: LiquidGlassDemo(),
          ),
        ),
      ),
    );
  }
}

class LiquidGlassDemo extends StatefulWidget {
  const LiquidGlassDemo({super.key});

  @override
  State<LiquidGlassDemo> createState() => _LiquidGlassDemoState();
}

class _LiquidGlassDemoState extends State<LiquidGlassDemo> {
  ui.FragmentProgram? _program;
  ui.Image? _sceneImage;
  final GlobalKey _sceneKey = GlobalKey();

  @override
  void initState() {
    super.initState();
    _loadProgram();
  }

  Future<void> _loadProgram() async {
    try {
      final prog = await ui.FragmentProgram.fromAsset('assets/shaders/liquid_glass.frag');
      setState(() {
        _program = prog;
      });
      // capture initial scene after program is ready
      WidgetsBinding.instance.addPostFrameCallback((_) => _captureScene());
    } catch (e) {
      // ignore for demo
      //print('Failed to load shader: $e');
    }
  }

  Future<void> _captureScene() async {
    try {
      final renderObject = _sceneKey.currentContext?.findRenderObject();
      if (renderObject is RenderRepaintBoundary) {
        final image = await renderObject.toImage(pixelRatio: ui.window.devicePixelRatio);
        setState(() {
          _sceneImage = image;
        });
      }
    } catch (e) {
      // ignore for demo
    }
  }

  @override
  Widget build(BuildContext context) {
    return RepaintBoundary(
      key: _sceneKey,
      child: Stack(
        fit: StackFit.expand,
        children: [
          // The UI scene that will be sampled by the glass shader
          Container(
            decoration: const BoxDecoration(
              gradient: LinearGradient(
                colors: [Color(0xFF283048), Color(0xFF859398)],
                begin: Alignment.topLeft,
                end: Alignment.bottomRight,
              ),
            ),
            child: Center(
              child: Column(
                mainAxisSize: MainAxisSize.min,
                children: const [
                  Text('Flutter Liquid Glass', style: TextStyle(color: Colors.white, fontSize: 24)),
                  SizedBox(height: 8),
                  Text('Tap to recapture scene', style: TextStyle(color: Colors.white70)),
                ],
              ),
            ),
          ),

          // Glass overlay
          GestureDetector(
            behavior: HitTestBehavior.translucent,
            onTap: _captureScene,
            child: CustomPaint(
              painter: LiquidGlassPainter(
                program: _program,
                sceneImage: _sceneImage,
              ),
            ),
          ),
        ],
      ),
    );
  }
}
