import 'dart:io';
import 'package:code_assets/code_assets.dart'; // 🎯 pubspec ထဲက package
import 'package:hooks/hooks.dart'; // 🎯 pubspec ထဲက package

void main(List<String> args) async {
  await build(args, (config, output) async {
    final targetOS = config.config.code.targetOS;
    final packageName = config.packageName;

    // ၁။ Linux Platform ဖြစ်မှ အလုပ်လုပ်မယ်
    if (targetOS == OS.linux) {
      // ၂။ သင့်ရဲ့ .so ဖိုင် ရှိတဲ့ လမ်းကြောင်း (Relative Path)
      // final libPath = Uri.file(
      //   '/home/thancoder/projects/cpp_projects/ffmpeg_lib/build/libmyffmpeg.so',
      // );
      final libPath = config.packageRoot.resolve('src/libmyffmpeg.so');

      if (!await File.fromUri(libPath).exists()) {
        throw Exception(
          'FFmpeg Linux shared library not found at: ${libPath.toFilePath()}',
        );
      }

      // ၄။ Output ထဲကို CodeAsset အနေနဲ့ တိုက်ရိုက် သွတ်သွင်းခြင်း 🎯
      output.assets.code.add(
        CodeAsset(
          package: config.packageName,
          name: '${packageName}_bindings_generated.dart',
          linkMode: DynamicLoadingBundled(),
          file: libPath,
        ),
      );
    }
  });
}
