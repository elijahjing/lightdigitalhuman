//
// Created by vincentsyan on 2025/8/21.
//

#ifndef LIGHTDIGITALHUMAN_SHADERMANAGER_H
#define LIGHTDIGITALHUMAN_SHADERMANAGER_H

#include "../GltfRenderer.h"
#include <memory>

namespace digitalhumans {
struct GLSLStringFiles {
  std::string animation;
  std::string brdf;
  std::string cubemap;
  std::string cubemap_vert;

  std::string functions;
  std::string ibl;
  std::string iridescence;

  std::string material_info;
  std::string pbr;
  std::string primitive;

  std::string punctual;
  std::string specular_glossiness;

  std::string textures;
  std::string tonemapping;
  std::string fullscreen;
  std::string panorama_to_cubemap;
  std::string ibl_filtering;
  std::string debug;
};

class ShaderManager {
 public:
  static ShaderManager &getInstance() {
    static ShaderManager instance;
    return instance;
  }

  void setShaderFiles(const GLSLStringFiles &files) {
    shaderFiles_ = files;
  }

  [[nodiscard]] const GLSLStringFiles &getShaderFiles() const {
    return shaderFiles_;
  }

 private:
  ShaderManager() = default;
  ~ShaderManager() = default;
  ShaderManager(const ShaderManager &) = delete;
  ShaderManager &operator=(const ShaderManager &) = delete;
  GLSLStringFiles shaderFiles_;
};
}
#endif //LIGHTDIGITALHUMAN_SHADERMANAGER_H
