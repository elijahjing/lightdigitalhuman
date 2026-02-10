//
// Created by vincentsyan on 2025/8/11.
//

#ifndef LIGHTDIGITALHUMAN_GLTFLOADER_H
#define LIGHTDIGITALHUMAN_GLTFLOADER_H

#include <android/asset_manager.h>
#include <iostream>
#include "../../../engine/Engine.h"
#include "tiny_gltf.h"

namespace digitalhumans {

class GltfLoader {

 public:
  bool loadGltfFromAssets(AAssetManager *assetManager,
                          const std::string &filename,
                          Engine
                          &outAssetData);

  bool loadFromFile(const std::string &filePath, Engine &outAssetData);


  void clearModelCache();

 private:

  bool validateFile(const std::string &filePath);

  bool isGltfFile(const std::string &filePath);

  bool isGlbFile(const std::string &filePath);

  std::string getFileExtension(const std::string &filePath);

  std::unordered_map<std::string, std::shared_ptr<tinygltf::Model>>
      loadedModels;
  std::mutex modelsMutex;

};

} // digtalhuman

#endif //LIGHTDIGITALHUMAN_GLTFLOADER_H
