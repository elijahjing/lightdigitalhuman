//
// Created by vincentsyan on 2025/7/15.
//

#ifndef LIGHTDIGITALHUMAN_LOGUTILS_H
#define LIGHTDIGITALHUMAN_LOGUTILS_H


// app/src/main/cpp/log_utils.h

#include <android/log.h>

#define LOG_TAG "DigitalHuman"

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, __VA_ARGS__)

// 条件编译优化
#ifdef NDEBUG
#undef LOGV
    #undef LOGD
    #define LOGV(...) ((void)0)
    #define LOGD(...) ((void)0)
#endif

#endif //LIGHTDIGITALHUMAN_LOGUTILS_H
