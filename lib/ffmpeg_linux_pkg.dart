// ignore_for_file: non_constant_identifier_names

import 'dart:ffi'
    show DynamicLibrary, Int, Pointer, PointerPointer, Void, nullptr, sizeOf;
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

  /// ### avcodec_get_supported_config
  int avcodec_get_supported_config(
    Pointer<_bindings.AVCodecContext> avctx,
    Pointer<_bindings.AVCodec> codec,
    _bindings.AVCodecConfig config,
    int flags,
    Pointer<Pointer<Void>> out_configs,
    Pointer<Int> out_num_configs,
  ) {
    return _bindings.avcodec_get_supported_config(
      avctx,
      codec,
      config,
      flags,
      out_configs,
      out_num_configs,
    );
  }

  // void listAllC() {
  //   final opaquePtr = malloc.allocate<Pointer<Void>>(sizeOf<Pointer<Void>>());
  //   opaquePtr.value = nullptr;

  //   try {
  //     while (true) {
  //       final codecPtr = _bindings.av_muxer_iterate(opaquePtr);
  //       if (opaquePtr.value == nullptr) break;
  //     }
  //   } catch (e) {
  //   } finally {
  //     malloc.free(opaquePtr);
  //   }
  // }

  /// ### swresample Configuration
  String get swresample_configuration {
    final result = _bindings.swresample_configuration();
    return result.cast<Utf8>().toDartString();
  }

  /// ### avformat Configuration
  String get avformat_configuration {
    final result = _bindings.avformat_configuration();
    return result.cast<Utf8>().toDartString();
  }

  /// ### swscale Configuration
  String get swscale_configuration {
    final result = _bindings.swscale_configuration();
    return result.cast<Utf8>().toDartString();
  }

  /// ### avcodec Configuration
  String get avcodec_configuration {
    final result = _bindings.avcodec_configuration();
    return result.cast<Utf8>().toDartString();
  }

  /// ### avutil Configuration
  String get avutil_configuration {
    final avutil_configuration = _bindings.avutil_configuration();
    return avutil_configuration.cast<Utf8>().toDartString();
  }

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
