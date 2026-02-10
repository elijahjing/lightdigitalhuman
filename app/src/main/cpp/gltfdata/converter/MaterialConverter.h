//
// Created by vincentsyan on 2025/8/19.
//

#ifndef LIGHTDIGITALHUMAN_MATERIALCONVERTER_H
#define LIGHTDIGITALHUMAN_MATERIALCONVERTER_H

#include "tiny_gltf.h"
#include <memory>
#include "../GltfMaterial.h"

namespace digitalhumans {

class MaterialConverter {
 public:
  static std::shared_ptr<GltfMaterial>
  convertMaterial(const tinygltf::Material &material);

 private:

  static std::shared_ptr<PbrMetallicRoughness> convertPbrMetallicRoughness(
      const tinygltf::PbrMetallicRoughness &pbr);

  static AlphaMode convertAlphaMode(const std::string &alphaModeStr);

  static MaterialType determineMaterialType(const tinygltf::Material &material);

  static std::shared_ptr<GltfTextureInfo> convertOcclusionTextureInfo(
      const tinygltf::OcclusionTextureInfo &occlusionInfo);

  static std::shared_ptr<GltfTextureInfo> convertNormalTextureInfo(
      const tinygltf::NormalTextureInfo &normalInfo);

  static std::shared_ptr<GltfTextureInfo> convertMetallicRoughnessTextureInfo(
      const tinygltf::TextureInfo &textureInfo);

  static std::shared_ptr<GltfTextureInfo> convertEmissiveTextureInfo(
      const tinygltf::TextureInfo &textureInfo);


  static std::shared_ptr<GltfTextureInfo> convertTextureInfo(
      const tinygltf::TextureInfo &textureInfo);

  static std::shared_ptr<GltfTextureInfo> convertTextureInfoFromValue(
      const tinygltf::Value &value);

  static void
  convertMaterialExtensions(const tinygltf::ExtensionMap &extensions,
                            std::shared_ptr<GltfMaterial>
                            &gltfMaterial);

};
}

#endif //LIGHTDIGITALHUMAN_MATERIALCONVERTER_H
