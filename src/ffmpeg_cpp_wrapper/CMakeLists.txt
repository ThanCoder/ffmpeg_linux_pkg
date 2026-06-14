cmake_minimum_required(VERSION 3.10)
project(ffmpeg LANGUAGES C CXX)


set(CMAKE_CXX_STANDARD 17)

# -----------------------------------------------------------------
# ၁။ libdrm ကို ရှာဖွေရန် PkgConfig သုံးခြင်း
# -----------------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(DRM REQUIRED libdrm)

# add_library(myffmpeg SHARED src/main.cpp)
add_executable(myffmpeg
    src/main.cpp
    src/dart_ffi_wrapper.h
    src/dart_ffi_wrapper.cpp
    src/video_to_audio.hpp
    src/video_to_audio.cpp

)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

# -----------------------------------------------------------------
# ၂။ Libraries များကို စနစ်တကျ ချိတ်ဆက်ခြင်း
# -----------------------------------------------------------------
target_link_libraries(myffmpeg
    PRIVATE
    # FFmpeg static libraries များကို အကုန်လုံးအလုပ်လုပ်စေရန် whole-archive ဖြင့်အုပ်ပါသည်
    -Wl,--whole-archive
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavformat.a
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavcodec.a
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavutil.a
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libswresample.a
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libswscale.a
    -Wl,--no-whole-archive

    # အောက်ခြေမှ မှီခိုနေသော System Libraries များကို Group ဖွဲ့ပြီး link လုပ်ပါသည်
    -Wl,--start-group
    lzma
    bz2
    ${DRM_LIBRARIES} # ပထမအဆင့်က ရှာတွေ့ထားတဲ့ variable
    pthread
    m
    z
    dl # g++ မှာပါတဲ့ -ldl ကျန်ခဲ့လို့ ထပ်ဖြည့်ပေးထားပါတယ်
    -Wl,--end-group
)
