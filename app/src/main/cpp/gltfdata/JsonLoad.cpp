//
// Created by vincentsyan on 2025/8/19.
//
#include "JsonLoad.h"
#include <algorithm>
#include <cstddef>
#include <android/asset_manager.h>
namespace digitalhumans {

// 从 assets 读取文件内容为字符串
std::string
readAssetFile(AAssetManager *assetManager, const std::string &filename) {
  if (!assetManager) {
    return "";
  }

  AAsset *asset =
      AAssetManager_open(assetManager, filename.c_str(), AASSET_MODE_BUFFER);
  if (!asset) {
    // 文件不存在或打开失败
    return "";
  }

  // 获取文件大小
  off_t fileSize = AAsset_getLength(asset);
  if (fileSize <= 0) {
    AAsset_close(asset);
    return "";
  }

  // 读取文件内容
  std::string content;
  content.resize(fileSize);

  int bytesRead = AAsset_read(asset, &content[0], fileSize);
  AAsset_close(asset);

  if (bytesRead != fileSize) {
    return "";
  }

  return content;
}

}