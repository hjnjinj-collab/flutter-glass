import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

void main() {
  runApp(const NativeGlassExample());
}

class NativeGlassExample extends StatefulWidget {
  const NativeGlassExample({Key? key}) : super(key: key);

  @override
  _NativeGlassExampleState createState() => _NativeGlassExampleState();
}

class _NativeGlassExampleState extends State<NativeGlassExample> {
  static const platform = MethodChannel('native_glass');
  int textureId = -1;

  @override
  void initState() {
    super.initState();
    _initPlugin();
  }

  Future<void> _initPlugin() async {
    try {
      final int id = await platform.invokeMethod('getTextureId');
      setState(() {
        textureId = id;
      });
      await platform.invokeMethod('start');
    } on PlatformException catch (e) {
      // ignore
    }
  }

  @override
  void dispose() {
    if (textureId != -1) {
      platform.invokeMethod('stop');
    }
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        backgroundColor: Colors.black,
        body: Center(
          child: textureId >= 0
              ? SizedBox(
                  width: 800,
                  height: 450,
                  child: Texture(textureId: textureId),
                )
              : const Text('Waiting for native texture...', style: TextStyle(color: Colors.white)),
        ),
      ),
    );
  }
}
