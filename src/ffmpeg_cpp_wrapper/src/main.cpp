#include "video_to_audio.hpp"
#include <iostream>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/log.h>
}

int main() {
  // အသုံးပြုပုံ ဥပမာ
  std::string input_video = "/home/thancoder/Videos/adehhh sunset.mp4";
  std::string output_audio =
      "../extracted_audio.aac"; // မူရင်း video format ထဲက audio က aac ဖြစ်ရင် .aac နဲ့
                                // သိမ်းရပါမယ်

  try {
    VideoAudioConverter converter(input_video, output_audio);
    // converter.set_audio_format();
    // converter.set_trim(double start_sec, double end_sec);
    auto callback = [](double pro) {
      std::cout << "progress: " << pro << "\n";
    };
    if (converter.convert(callback)) {
      std::cout << "Process finished without errors.\n";
    } else {
      std::cerr << "Process failed.\n";
    }
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
// int main() {
// std::cout << "========================================" << std::endl;
// std::cout << "   FFmpeg C++ Static Link Test Successful!  " << std::endl;
// std::cout << "========================================" << std::endl;

// // FFmpeg Configuration နှင့် Version အချက်အလက်များကို ထုတ်ပြခြင်း
// std::cout << "FFmpeg Version    : " << av_version_info() << std::endl;
// std::cout << "AVCodec Version   : " << avcodec_version() << std::endl;
// std::cout << "AVFormat Version  : " << avformat_version() << std::endl;
// std::cout << "AVUtil Version    : " << avutil_version() << std::endl;
// std::cout << "========================================" << std::endl;
//   return 0;
// }