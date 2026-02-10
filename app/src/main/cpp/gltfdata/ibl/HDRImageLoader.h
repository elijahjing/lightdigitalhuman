//
// Created by vincentsyan on 2025/9/16.
//

#ifndef LIGHTDIGITALHUMAN_HDRIMAGELOADER_H
#define LIGHTDIGITALHUMAN_HDRIMAGELOADER_H
#include <android/asset_manager.h>
#include <string>
#include <vector>
namespace digitalhumans {
class HDRImage;

class HDRImageLoader {
 public:
  static void setAssetManager(AAssetManager *assetManager);
  static HDRImage loadFromAssets(const std::string &assetPath);
  static HDRImage loadFromFile(const std::string &filePath);

 private:
  static AAssetManager *assetManager_;
  static std::vector<uint8_t> readAssetFile(const std::string &assetPath);
};
} // digitalhumans

#endif //LIGHTDIGITALHUMAN_HDRIMAGELOADER_H
