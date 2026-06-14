// ignore_for_file: non_constant_identifier_names

import 'dart:ffi' show DynamicLibrary;
import 'dart:io';

import 'package:ffi/ffi.dart';

import 'ffmpeg_linux_pkg_bindings_generated.dart' as _bindings;

FFmpegInstance getFfmpeg({String? customLibPath}) {
  if (!Platform.isLinux) {
    throw UnsupportedError('This package only supports Linux platform.');
  }
  if (customLibPath != null) {
    DynamicLibrary.open(customLibPath);
    // _bindings.av_version_info()
  }
  return FFmpegInstance();
}

class FFmpegInstance {
  const FFmpegInstance();

  /// ### AVCodec Version
  int get avcodec_version => _bindings.avcodec_version();

  /// ### AVUtil Version
  int get avutil_version => _bindings.avutil_version();

  /// ### SWScale Version
  int get swscale_version => _bindings.swscale_version();

  /// ### AVFormat Version
  int get avformat_version => _bindings.avformat_version();

  /// ### SWresample Version
  int get swresample_version => _bindings.swresample_version();

  /// ### FFmpeg Version
  String get av_version_info {
    final strPtr = _bindings.av_version_info();
    return strPtr.cast<Utf8>().toDartString();
    // return "";
  }
}
