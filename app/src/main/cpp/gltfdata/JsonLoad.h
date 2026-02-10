//
// Created by vincentsyan on 2025/8/19.
//

#ifndef LIGHTDIGITALHUMAN_JSONLOAD_H
#define LIGHTDIGITALHUMAN_JSONLOAD_H

#include <fstream>
#include <iostream>
#include <vector>
#include <optional>
#include "glm.hpp"
#include "examples/validator/src/json.hpp"
#include "tiny_gltf.h"
namespace digitalhumans {

class JsonLoad {

  bool loadFromFile(const std::string &filepath) {
    try {
      std::ifstream file(filepath);
      if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
      }

      nlohmann::json j;
      file >> j;

      return parseGltf(j);

    } catch (const std::exception &e) {
      std::cerr << "Error loading glTF: " << e.what() << std::endl;
      return false;
    }
  }

  bool parseGltf(const nlohmann::json &j) {
    // 解析 asset
    // 解析 asset
    if (j.find("asset") != j.end()) {
      auto asset = j["asset"];
      // parseAsset(j["asset"], gltfData.asset);
    }
  }
};

}
#endif //LIGHTDIGITALHUMAN_JSONLOAD_H
