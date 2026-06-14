#include "video_to_audio.hpp"
#include <iostream>

// Constructor
VideoAudioConverter::VideoAudioConverter(const std::string &input,
                                         const std::string &output)
    : input_filename(input), output_filename(output) {}

// Destructor
VideoAudioConverter::~VideoAudioConverter() { cleanup(); }
bool VideoAudioConverter::open_input() {
  if (avformat_open_input(&ifmt_ctx, input_filename.c_str(), nullptr, nullptr) <
      0)
    return false;
  if (avformat_find_stream_info(ifmt_ctx, nullptr) < 0)
    return false;

  for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
    if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
      audio_stream_index = i;
      break;
    }
  }
  return (audio_stream_index != -1);
}

bool VideoAudioConverter::init_decoder() {
  AVCodecParameters *codecpar = ifmt_ctx->streams[audio_stream_index]->codecpar;
  const AVCodec *decoder = avcodec_find_decoder(codecpar->codec_id);
  if (!decoder)
    return false;

  decoder_ctx = avcodec_alloc_context3(decoder);
  if (!decoder_ctx)
    return false;

  if (avcodec_parameters_to_context(decoder_ctx, codecpar) < 0)
    return false;
  if (avcodec_open2(decoder_ctx, decoder, nullptr) < 0)
    return false;

  return true;
}

bool VideoAudioConverter::init_encoder() {
  const AVCodec *encoder = nullptr;

  // User ရွေးချယ်လိုက်သော Format အလိုက် Encoder ကို ရှာဖွေခြင်း
  switch (target_format) {
  case AudioFormat::MP3:
    encoder = avcodec_find_encoder(AV_CODEC_ID_MP3);
    break;
  case AudioFormat::AAC:
    encoder = avcodec_find_encoder(AV_CODEC_ID_AAC);
    break;
  case AudioFormat::WAV:
    encoder = avcodec_find_encoder(AV_CODEC_ID_PCM_S16LE);
    break;
  }
  if (!encoder)
    return false;

  encoder_ctx = avcodec_alloc_context3(encoder);
  if (!encoder_ctx)
    return false;

  // Output Audio Settings (Sample Rate, Channels, Bitrate များကို သတ်မှတ်ခြင်း)
  encoder_ctx->sample_rate = decoder_ctx->sample_rate;
  av_channel_layout_copy(&encoder_ctx->ch_layout, &decoder_ctx->ch_layout);
  encoder_ctx->sample_fmt =
      encoder->sample_fmts ? encoder->sample_fmts[0] : decoder_ctx->sample_fmt;
  encoder_ctx->bit_rate = 192000; // 192 kbps

  if (avcodec_open2(encoder_ctx, encoder, nullptr) < 0)
    return false;
  return true;
}

bool VideoAudioConverter::open_output() {
  avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr,
                                 output_filename.c_str());
  if (!ofmt_ctx)
    return false;

  AVStream *out_stream = avformat_new_stream(ofmt_ctx, nullptr);
  if (!out_stream)
    return false;

  if (avcodec_parameters_from_context(out_stream->codecpar, encoder_ctx) < 0)
    return false;

  if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
    if (avio_open(&ofmt_ctx->pb, output_filename.c_str(), AVIO_FLAG_WRITE) < 0)
      return false;
  }

  if (avformat_write_header(ofmt_ctx, nullptr) < 0)
    return false;
  return true;
}

bool VideoAudioConverter::convert(ProgressCallback progress_cb) {
  if (!open_input()) {
    std::cerr << "Open Input Failed\n";
    return false;
  }
  if (target_format == AudioFormat::AAC) {
    std::cout << "[Log] Target is AAC. Using fast Stream Copy (No Encoder).\n";
    if (!open_output_for_stream_copy())
      return false;
    return stream_copy_loop(progress_cb); // အင်မတန်မြန်ပြီး FIFO မလိုတဲ့ loop
  } else {
    if (!init_decoder()) {
      std::cerr << "Init Decoder Failed\n";
      return false;
    }
    if (!init_encoder()) {
      std::cerr << "Init Encoder Failed\n";
      return false;
    }
    if (!open_output()) {
      std::cerr << "Open Output Failed\n";
      return false;
    }
    return transcode_loop(progress_cb);
  }
}

// Decoding နှင့် Encoding ကို ပေါင်းစပ်ပတ်မည့် Loop ကြီး
bool VideoAudioConverter::transcode_loop(ProgressCallback progress_cb) {
  AVPacket *in_packet = av_packet_alloc();
  AVFrame *frame = av_frame_alloc();
  AVPacket *out_packet = av_packet_alloc();
  AVStream *in_stream = ifmt_ctx->streams[audio_stream_index];

  if (!in_packet || !frame || !out_packet) {
    std::cerr << "Could not allocate packets or frames\n";
    return false;
  }

  std::cout << "[Log] Processing Transcoding & Trimming...\n";
  int ret = 0;

  while (av_read_frame(ifmt_ctx, in_packet) >= 0) {
    if (in_packet->stream_index == audio_stream_index) {

      // Trim / Time Check Logic
      double current_time = in_packet->pts * av_q2d(in_stream->time_base);
      if (current_time >= start_time_sec &&
          (end_time_sec < 0 || current_time <= end_time_sec)) {

        // 1. Send Packet to Decoder
        ret = avcodec_send_packet(decoder_ctx, in_packet);
        if (ret < 0) {
          std::cerr << "Error submitting a packet for decoding\n";
          break;
        }

        // 2. Receive Frames from Decoder (Loop ပတ်ပြီး တတ်နိုင်သမျှ frame အကုန်ထုတ်ယူမယ်)
        while (ret >= 0) {
          ret = avcodec_receive_frame(decoder_ctx, frame);
          if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break; // ဒီအချိန်မှာ packet အသစ် ထပ်ဖတ်ဖို့ လိုပြီမို့ out_loop ကို သွားမယ်
          } else if (ret < 0) {
            std::cerr << "Error during decoding\n";
            goto end; // တကယ့် error မို့ အလုပ်ရပ်မယ်
          }

          // 3. Send Decoded Frame to Encoder
          int enc_ret = avcodec_send_frame(encoder_ctx, frame);
          if (enc_ret < 0) {
            std::cerr << "Error sending a frame for encoding\n";
            goto end;
          }

          // 4. Receive Packets from Encoder
          while (enc_ret >= 0) {
            enc_ret = avcodec_receive_packet(encoder_ctx, out_packet);
            if (enc_ret == AVERROR(EAGAIN) || enc_ret == AVERROR_EOF) {
              break;
            } else if (enc_ret < 0) {
              std::cerr << "Error during encoding\n";
              goto end;
            }

            // Write encoded packet to output file
            out_packet->stream_index = 0;

            // (မဖြစ်မနေလိုအပ်ချက်) Encoder ကထွက်လာတဲ့ timestamp ကို output stream
            // timescale ထဲ ပြောင်းပေးရပါမယ်
            av_packet_rescale_ts(out_packet, encoder_ctx->time_base,
                                 ofmt_ctx->streams[0]->time_base);

            av_interleaved_write_frame(ofmt_ctx, out_packet);
            av_packet_unref(out_packet);
          }

          av_frame_unref(frame); // နောက်တစ်ကြိမ် loop အတွက် frame ကို clear လုပ်ပေးတာ
        }
      }

      // Progress Callback
      if (progress_cb && in_stream->duration > 0) {
        double percent =
            ((double)in_packet->dts / (double)in_stream->duration) * 100.0;
        progress_cb(percent > 100.0 ? 100.0 : percent);
      }
    }
    av_packet_unref(in_packet);
  }

  // Flush Encoder (Buffer ထဲ ကျန်နေတာတွေ အကုန် ဆွဲထုတ်ပြီး သိမ်းခြင်း)
  avcodec_send_frame(encoder_ctx, nullptr);
  while (avcodec_receive_packet(encoder_ctx, out_packet) >= 0) {
    out_packet->stream_index = 0;
    av_packet_rescale_ts(out_packet, encoder_ctx->time_base,
                         ofmt_ctx->streams[0]->time_base);
    av_interleaved_write_frame(ofmt_ctx, out_packet);
    av_packet_unref(out_packet);
  }

  av_write_trailer(ofmt_ctx);

end:
  av_packet_free(&in_packet);
  av_frame_free(&frame);
  av_packet_free(&out_packet);
  return (ret >= 0 || ret == AVERROR_EOF || ret == AVERROR(EAGAIN));
}

bool VideoAudioConverter::open_output_for_stream_copy() {
  avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr,
                                 output_filename.c_str());
  if (!ofmt_ctx)
    return false;

  AVStream *in_stream = ifmt_ctx->streams[audio_stream_index];
  AVStream *out_stream = avformat_new_stream(ofmt_ctx, nullptr);
  if (!out_stream)
    return false;

  // Encoder မသုံးဘဲ ပါရာမီတာတွေကို တန်းကူးချလိုက်တာ
  if (avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar) < 0)
    return false;
  out_stream->codecpar->codec_tag = 0;

  if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
    if (avio_open(&ofmt_ctx->pb, output_filename.c_str(), AVIO_FLAG_WRITE) < 0)
      return false;
  }
  return avformat_write_header(ofmt_ctx, nullptr) >= 0;
}

bool VideoAudioConverter::stream_copy_loop(ProgressCallback progress_cb) {
  AVPacket *packet = av_packet_alloc();
  AVStream *in_stream = ifmt_ctx->streams[audio_stream_index];
  AVStream *out_stream = ofmt_ctx->streams[0];

  while (av_read_frame(ifmt_ctx, packet) >= 0) {
    if (packet->stream_index == audio_stream_index) {
      double current_time = packet->pts * av_q2d(in_stream->time_base);

      // Trim Logic ကိုပါ တစ်ခါတည်း သုံးလို့ရပါတယ်
      if (current_time >= start_time_sec &&
          (end_time_sec < 0 || current_time <= end_time_sec)) {
        // Timebase ပြန်ညှိခြင်း
        packet->pts = av_rescale_q_rnd(
            packet->pts, in_stream->time_base, out_stream->time_base,
            (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        packet->dts = av_rescale_q_rnd(
            packet->dts, in_stream->time_base, out_stream->time_base,
            (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        packet->duration = av_rescale_q(packet->duration, in_stream->time_base,
                                        out_stream->time_base);
        packet->pos = -1;
        packet->stream_index = 0;

        av_interleaved_write_frame(ofmt_ctx, packet);
      }

      if (progress_cb && in_stream->duration > 0) {
        int64_t current_dts = packet->dts < 0 ? 0 : packet->dts;
        progress_cb(((double)current_dts / (double)in_stream->duration) *
                    100.0);
      }
    }
    av_packet_unref(packet);
  }
  av_write_trailer(ofmt_ctx);
  av_packet_free(&packet);
  return true;
}

void VideoAudioConverter::cleanup() {
  if (decoder_ctx)
    avcodec_free_context(&decoder_ctx);
  if (encoder_ctx)
    avcodec_free_context(&encoder_ctx);
  if (ifmt_ctx)
    avformat_close_input(&ifmt_ctx);
  if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    avio_closep(&ofmt_ctx->pb);
  if (ofmt_ctx)
    avformat_free_context(ofmt_ctx);
}