// ignore_for_file: avoid_print

import 'package:ffmpeg_linux_pkg/ffmpeg_linux_pkg.dart';
import 'package:flutter/material.dart';

void main() {
  runApp(MaterialApp(home: const MyApp()));
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Placeholder(),
      floatingActionButton: FloatingActionButton(
        onPressed: () async {
          try {
            final ff = getFfmpeg();

            print('av_version_info: ${ff.av_version_info}');
            print('avcodec_version: ${ff.avcodec_version}');
            print('avformat_version: ${ff.avformat_version}');
            print('avutil_version: ${ff.avutil_version}');
            print('swresample_version: ${ff.swresample_version}');
            print('swscale_version: ${ff.swscale_version}');
          } catch (e) {
            print(e);
          }
        },
      ),
    );
  }
}
