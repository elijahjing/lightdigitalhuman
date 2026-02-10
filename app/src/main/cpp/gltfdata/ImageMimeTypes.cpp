//
// Created by vincentsyan on 2025/8/18.
//

#include "ImageMimeTypes.h"
#include <algorithm>
#include <cctype>

namespace digitalhumans {

// MIME类型字符串常量定义
const std::string ImageMimeTypes::JPEG = "image/jpeg";
const std::string ImageMimeTypes::PNG = "image/png";
const std::string ImageMimeTypes::WEBP = "image/webp";
const std::string ImageMimeTypes::HDR = "image/vnd.radiance";
const std::string ImageMimeTypes::KTX2 = "image/ktx2";
const std::string ImageMimeTypes::GLTEXTURE = "image/texture";

// 字符串到枚举的映射表
const std::unordered_map<std::string, ImageMimeType>
    ImageMimeTypes::stringToEnum = {
    {JPEG, ImageMimeType::JPEG},
    {PNG, ImageMimeType::PNG},
    {WEBP, ImageMimeType::WEBP},
    {HDR, ImageMimeType::HDR},
    {KTX2, ImageMimeType::KTX2},
    {GLTEXTURE, ImageMimeType::GLTEXTURE}
};

// 枚举到字符串的映射表
const std::unordered_map<ImageMimeType, std::string>
    ImageMimeTypes::enumToString = {
    {ImageMimeType::JPEG, JPEG},
    {ImageMimeType::PNG, PNG},
    {ImageMimeType::WEBP, WEBP},
    {ImageMimeType::HDR, HDR},
    {ImageMimeType::KTX2, KTX2},
    {ImageMimeType::GLTEXTURE, GLTEXTURE}
};

// 文件扩展名到枚举的映射表
const std::unordered_map<std::string, ImageMimeType>
    ImageMimeTypes::extensionToEnum = {
    {".jpg", ImageMimeType::JPEG},
    {".jpeg", ImageMimeType::JPEG},
    {".png", ImageMimeType::PNG},
    {".webp", ImageMimeType::WEBP},
    {".hdr", ImageMimeType::HDR},
    {".ktx2", ImageMimeType::KTX2},
    {".texture", ImageMimeType::GLTEXTURE}
};

// 枚举到文件扩展名的映射表
const std::unordered_map<ImageMimeType, std::string>
    ImageMimeTypes::enumToExtension = {
    {ImageMimeType::JPEG, ".jpg"},
    {ImageMimeType::PNG, ".png"},
    {ImageMimeType::WEBP, ".webp"},
    {ImageMimeType::HDR, ".hdr"},
    {ImageMimeType::KTX2, ".ktx2"},
    {ImageMimeType::GLTEXTURE, ".texture"}
};

std::string ImageMimeTypes::toString(ImageMimeType type) {
  auto it = enumToString.find(type);
  if (it != enumToString.end()) {
    return it->second;
  }
  return "";
}

ImageMimeType ImageMimeTypes::fromString(const std::string &mimeType) {
  auto it = stringToEnum.find(mimeType);
  if (it != stringToEnum.end()) {
    return it->second;
  }
  return ImageMimeType::UNKNOWN;
}

bool ImageMimeTypes::isValidImageMimeType(const std::string &mimeType) {
  return stringToEnum.find(mimeType) != stringToEnum.end();
}

ImageMimeType ImageMimeTypes::fromFileExtension(const std::string &extension) {
  // 转换为小写并确保以点号开头
  std::string normalizedExt = extension;
  std::transform(normalizedExt.begin(),
                 normalizedExt.end(),
                 normalizedExt.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if (!normalizedExt.empty() && normalizedExt[0] != '.') {
    normalizedExt = "." + normalizedExt;
  }

  auto it = extensionToEnum.find(normalizedExt);
  if (it != extensionToEnum.end()) {
    return it->second;
  }
  return ImageMimeType::UNKNOWN;
}

std::string ImageMimeTypes::getFileExtension(ImageMimeType type) {
  auto it = enumToExtension.find(type);
  if (it != enumToExtension.end()) {
    return it->second;
  }
  return "";
}

bool ImageMimeTypes::supportsTransparency(ImageMimeType type) {
  switch (type) {
    case ImageMimeType::PNG:
    case ImageMimeType::WEBP:
    case ImageMimeType::GLTEXTURE:
      return true;
    case ImageMimeType::JPEG:
    case ImageMimeType::HDR:
    case ImageMimeType::KTX2:
    default:
      return false;
  }
}

bool ImageMimeTypes::isCompressedFormat(ImageMimeType type) {
  switch (type) {
    case ImageMimeType::KTX2:
    case ImageMimeType::GLTEXTURE:
      return true;
    case ImageMimeType::JPEG:
    case ImageMimeType::PNG:
    case ImageMimeType::WEBP:
    case ImageMimeType::HDR:
    default:
      return false;
  }
}

std::vector<std::string> ImageMimeTypes::getAllMimeTypes() {
  std::vector<std::string> mimeTypes;
  for (const auto &pair: enumToString) {
    if (pair.first != ImageMimeType::UNKNOWN) {
      mimeTypes.push_back(pair.second);
    }
  }
  return mimeTypes;
}

} // namespace digitalhumans