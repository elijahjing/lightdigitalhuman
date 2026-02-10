//
// Created by vincentsyan on 2025/8/11.
//

#include "GltfLoader.h"
#include "tiny_gltf.h"


#include <algorithm>
#include <cstddef>
#include <android/asset_manager.h>
#include <__filesystem/operations.h>
#include "../../../engine/Engine.h"
#include "../GltfConverter.h"
#include "../GltfState.h"
#include "../../utils/LogUtils.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace digitalhumans {
bool GltfLoader::loadGltfFromAssets(AAssetManager *assetManager,
                                    const std::string &filename,
                                    Engine &outAssetData) {

  std::lock_guard<std::mutex> lock(modelsMutex);
  auto it = loadedModels.find(filename);
  if (it != loadedModels.end()) {
    auto gltf = GltfConverter::convert(*it->second, outAssetData, "");
    outAssetData.state->setGltf(gltf);
    return true;
  }
  AAsset *asset =
      AAssetManager_open(assetManager, filename.c_str(), AASSET_MODE_STREAMING);
  if (!asset) {
    return false;
  }

  size_t length = AAsset_getLength(asset);
  if (length == 0) {
    AAsset_close(asset);
    return false;
  }

  std::vector<uint8_t> buffer(length);
  int bytesRead = AAsset_read(asset, buffer.data(), length);
  AAsset_close(asset);

  if (bytesRead != static_cast<int>(length)) {
    return false;
  }

  auto model = std::make_shared<tinygltf::Model>();
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;
  bool success = loader.LoadBinaryFromMemory(model.get(), &err, &warn,
                                             buffer.data(), buffer.size());

  if (!warn.empty()) {
    LOGW("GLTF warn: %s", warn.c_str());
  }
  if (!err.empty()) {
    LOGE("GLTF error: %s", err.c_str());
  }
  if (!success) {
    return false;
  }

  try {
    auto gltf = GltfConverter::convert(*model, outAssetData, "");
    if (!gltf) {
      return false;
    }
    loadedModels[filename] = model;
    outAssetData.state->setGltf(gltf);
    return true;
  } catch (const std::exception &e) {
    return false;
  } catch (...) {
    return false;
  }
}

/**
 * @brief 从完整文件路径加载GLTF模型//dum-heli-01  skybox
 * @param filePath 完整的GLTF文件路径，如 "/data/data/com.app/files/models/character/model.gltf"
 * @param model 输出的模型对象
 * @return 是否加载成功
 */
bool
GltfLoader::loadFromFile(const std::string &filePath, Engine &outAssetData) {
  if (!validateFile(filePath)) {
    return false;
  }
  auto model = std::make_shared<tinygltf::Model>();
  tinygltf::TinyGLTF loader;
  std::string error, warning;
  bool success = false;

  if (isGlbFile(filePath)) {
    success =
        loader.LoadBinaryFromFile(model.get(), &error, &warning, filePath);
    if (!success) {
      LOGE("GLTF加载失败: %s", error.c_str());
      return false;
    }
  } else if (isGltfFile(filePath)) {
    success = loader.LoadASCIIFromFile(model.get(), &error, &warning, filePath);
  } else {
    LOGE("不支持的文件格式: %s", filePath.c_str());
    return false;
  }
  if (!warning.empty()) {
    LOGW("GLTF警告: %s", warning.c_str());
  }
  try {
    auto gltf = GltfConverter::convert(*model, outAssetData, "");
    if (!gltf) {
      return false;
    }
    loadedModels[filePath] = model;
    outAssetData.state->setGltf(gltf);
    return true;

  } catch (const std::exception &e) {
    return false;
  } catch (...) {
    return false;
  }
  return true;
}

/**
 * @brief 检查是否是GLTF文件
 */
bool GltfLoader::isGltfFile(const std::string &filePath) {
  std::string extension = getFileExtension(filePath);
  return extension == ".gltf";
}

/**
 * @brief 检查是否是GLB文件
 */
bool GltfLoader::isGlbFile(const std::string &filePath) {
  std::string extension = getFileExtension(filePath);
  return extension == ".glb";
}

/**
* @brief 验证文件存在性和可读性
*/
bool GltfLoader::validateFile(const std::string &filePath) {
  if (!std::filesystem::exists(filePath)) {
    LOGE("文件不存在: %s", filePath.c_str());
    return false;
  }

  if (!std::filesystem::is_regular_file(filePath)) {
    LOGE("不是常规文件: %s", filePath.c_str());
    return false;
  }
  return true;
}

/**
 * @brief 获取文件扩展名
 */
std::string GltfLoader::getFileExtension(const std::string &filePath) {
  size_t dotPos = filePath.find_last_of('.');
  if (dotPos != std::string::npos) {
    std::string ext = filePath.substr(dotPos);
    // 转换为小写
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
  }
  return "";
}


/**
 * @brief 清理已加载的模型缓存
 */
void GltfLoader::clearModelCache() {
  std::lock_guard<std::mutex> lock(modelsMutex);
  loadedModels.clear();
}

} // digitahuman