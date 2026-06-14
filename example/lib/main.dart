// ignore_for_file: avoid_print

import 'package:ffmpeg_linux_pkg/ffmpeg_linux_pkg.dart';
import 'package:flutter/material.dart';

void main() {
  runApp(MaterialApp(theme: ThemeData.dark(), home: const MyApp()));
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
            print('avformat_configuration: ${ff.avformat_configuration}');
            print('avutil_configuration: ${ff.avutil_configuration}');
            print('avcodec_configuration: ${ff.avcodec_configuration}');
            print('swresample_configuration: ${ff.swresample_configuration}');
          } catch (e) {
            print(e);
          }
        },
      ),
    );
  }
}
