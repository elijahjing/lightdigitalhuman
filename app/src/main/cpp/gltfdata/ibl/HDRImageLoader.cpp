//
// Created by vincentsyan on 2025/9/16.
//
#include "HDRImageLoader.h"
#include <android/log.h>
#include "stb_image.h"
#include "../ibl_sampler.h"

#define LOG_TAG "HDRImageLoader"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace digitalhumans {

AAssetManager *HDRImageLoader::assetManager_ = nullptr;

void HDRImageLoader::setAssetManager(AAssetManager *assetManager) {
  assetManager_ = assetManager;
  LOGI("AssetManager set successfully");
}

HDRImage HDRImageLoader::loadFromAssets(const std::string &assetPath) {
  HDRImage image;
  if (!assetManager_) {
    LOGE("AssetManager not set!");
    return image;
  }

  try {
    std::vector<uint8_t> fileData = readAssetFile(assetPath);
    if (fileData.empty()) {
      LOGE(" Failed to read asset file: %s", assetPath.c_str());
      return image;
    }
    int width, height, channels;
    float *hdrData = stbi_loadf_from_memory(
        fileData.data(),
        static_cast<int>(fileData.size()),
        &width, &height, &channels, 3  // 强制转换为3通道RGB
    );

    if (!hdrData) {
      LOGE("Failed to decode HDR data: %s", " stbi_failure_reason()");
      return image;
    }
    image.width = width;
    image.height = height;
    size_t pixelCount = width * height * 3; // RGB
    image.dataFloat.resize(pixelCount);
    std::memcpy(image.dataFloat.data(), hdrData, pixelCount * sizeof(float));
    stbi_image_free(hdrData);
    return image;

  } catch (const std::exception &e) {
    LOGE("Exception loading HDR: %s", e.what());
    return HDRImage{};
  }
}

HDRImage HDRImageLoader::loadFromFile(const std::string &filePath) {
  HDRImage image;
  int width, height, channels;
  float *hdrData = stbi_loadf(filePath.c_str(), &width, &height, &channels, 3);
  if (!hdrData) {
    LOGE("Failed to load HDR file: %s", "stbi_failure_reason()");
    return image;
  }

  image.width = width;
  image.height = height;
  size_t pixelCount = width * height * 3;
  image.dataFloat.resize(pixelCount);
  std::memcpy(image.dataFloat.data(), hdrData, pixelCount * sizeof(float));
  stbi_image_free(hdrData);
  return image;
}

std::vector<uint8_t>
HDRImageLoader::readAssetFile(const std::string &assetPath) {
  std::vector<uint8_t> data;
  AAsset *asset =
      AAssetManager_open(assetManager_, assetPath.c_str(), AASSET_MODE_BUFFER);
  if (!asset) {
    LOGE("Failed to open asset: %s", assetPath.c_str());
    return data;
  }
  off_t fileSize = AAsset_getLength(asset);
  if (fileSize <= 0) {
    LOGE("Invalid asset size: %ld", fileSize);
    AAsset_close(asset);
    return data;
  }
  data.resize(static_cast<size_t>(fileSize));
  int bytesRead = AAsset_read(asset, data.data(), fileSize);
  if (bytesRead != fileSize) {
    LOGE(" Failed to read complete asset file. Expected: %ld, Read: %d",
         fileSize, bytesRead);
    data.clear();
  }
  AAsset_close(asset);
  return data;
}

} // namespace digitalhumans