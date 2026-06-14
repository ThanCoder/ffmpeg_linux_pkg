extern "C" {
#include <libavformat/avformat.h>
}

#pragma once

typedef void (*ErrorCallback)(const char *error_message);

// Linux/Android Compiler (GCC/Clang) အတွက် Function ကို ပျောက်မသွားအောင် ထိန်းပေးပြီး
// ပြင်ပ (Flutter FFI/JNI) ကနေ တိုက်ရိုက် မြင်နိုင်၊ ခေါ်နိုင်အောင် သတ်မှတ်ခြင်း
#if defined(_WIN32)
#define LibExport __declspec(dllexport) // (နောင်တချိန် Windows အတွက် လိုခဲ့ရင်)
#else
#define LibExport                                                              \
  __attribute__((visibility("default"))) // Linux/Android အတွက် အဓိက
#endif

#ifdef __cplusplus
extern "C" {
#endif

// need write

LibExport int extract_video_thumbnail(const char *video_path,
                                      const char *out_path,
                                      AVFormatContext *format_ctx = nullptr);

#ifdef __cplusplus
}
#endif