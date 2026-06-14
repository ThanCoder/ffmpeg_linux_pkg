#pragma once

#include <functional>
#include <string>
extern "C" {

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/log.h>
#include <libavutil/opt.h>
}

// Progress ကို App ဘက်ကနေ လှမ်းသိဖို့ Callback Function Type သတ်မှတ်ခြင်း
using ProgressCallback = std::function<void(double percentage)>;
using VideoAudioConverterErrorCallback = std::function<void(std::string error)>;

enum class AudioFormat { AAC, MP3, WAV };

class VideoAudioConverter {
private:
  std::string input_filename;
  std::string output_filename;
  AudioFormat target_format = AudioFormat::AAC;

  // FFmpeg Contexts
  AVFormatContext *ifmt_ctx = nullptr;
  AVFormatContext *ofmt_ctx = nullptr;
  AVCodecContext *decoder_ctx = nullptr;
  AVCodecContext *encoder_ctx = nullptr;
  int audio_stream_index = -1;

  // အနာဂတ်အတွက် ထည့်သွင်းထားသော Parameter များ
  double start_time_sec = 0.0; // Trim လုပ်ရန် စက္ကန့်
  double end_time_sec = -1.0;  // Trim လုပ်ရန် စက္ကန့် (-1 ဆိုလျှင် အဆုံးထိ)

  // Private Helper Functions (Logic များကို ခွဲထုတ်ထားခြင်း)
  bool open_input();
  bool open_output();
  bool init_decoder();
  bool init_encoder();
  bool transcode_loop(ProgressCallback progress_cb);
  void rescale_timestamps(AVPacket *packet, AVStream *in_stream,
                          AVStream *out_stream);
  bool is_within_trim_range(AVPacket *packet, AVStream *in_stream);
  void update_progress(int64_t current_dts, AVStream *in_stream,
                       ProgressCallback callback);
  bool open_output_for_stream_copy();
  bool stream_copy_loop(ProgressCallback progress_cb);
  // RAII အတိုင်း Resource တွေကို သေချာပြန်ပိတ်ဖို့ Cleanup Function
  void cleanup();

public:
  // Constructor
  VideoAudioConverter(const std::string &input, const std::string &output);

  // Destructor
  ~VideoAudioConverter();
  void set_audio_format(AudioFormat format) { target_format = format; }

  // Trim သတ်မှတ်ရန် Setter Functions
  // -1 -> to end
  void set_trim(double start_sec, double end_sec) {
    start_time_sec = start_sec;
    end_time_sec = end_sec;
  }

  // အဓိက အလုပ်လုပ်မည့် Function (အပြင်ကနေ Progress ပြချင်ရင် Callback ထည့်ပေးရုံပဲ)
  // အဓိက အလုပ်လုပ်မည့် function
  bool convert(ProgressCallback progress_cb = nullptr);
};