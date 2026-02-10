//
// Created by vincentsyan on 2025/8/19.
//

#include "MaterialConverter.h"
#include "../../utils/LogUtils.h"
#include "vec3.hpp"

namespace digitalhumans {

std::shared_ptr<GltfMaterial>
MaterialConverter::convertMaterial(const tinygltf::Material &material) {
  try {
    auto gltfMaterial = std::make_shared<GltfMaterial>();
    if (!material.name.empty()) {
      gltfMaterial->setName(material.name);
    }
    if (!material.pbrMetallicRoughness.baseColorFactor.empty() ||
        material.pbrMetallicRoughness.baseColorTexture.index >= 0 ||
        material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {

      auto pbr = convertPbrMetallicRoughness(material.pbrMetallicRoughness);
      gltfMaterial->setPbrMetallicRoughness(pbr);
    }
    if (material.normalTexture.index >= 0) {
      auto normalTexture = convertNormalTextureInfo(material.normalTexture);
      gltfMaterial->setNormalTexture(normalTexture);
    }
    if (material.occlusionTexture.index >= 0) {
      auto occlusionTexture =
          convertOcclusionTextureInfo(material.occlusionTexture);
      gltfMaterial->setOcclusionTexture(occlusionTexture);
    }
    if (material.emissiveTexture.index >= 0) {
      auto emissiveTexture = convertTextureInfo(material.emissiveTexture);
      gltfMaterial->setEmissiveTexture(emissiveTexture);
    }
    if (material.emissiveFactor.size() >= 3) {
      glm::vec3 emissiveFactor(
          material.emissiveFactor[0],
          material.emissiveFactor[1],
          material.emissiveFactor[2]
      );
      gltfMaterial->setEmissiveFactor(emissiveFactor);
    }
    AlphaMode alphaMode = convertAlphaMode(material.alphaMode);
    gltfMaterial->setAlphaMode(alphaMode);
    gltfMaterial->setAlphaCutoff(static_cast<float>(material.alphaCutoff));
    gltfMaterial->setDoubleSided(material.doubleSided);
    convertMaterialExtensions(material.extensions, gltfMaterial);
    MaterialType materialType = determineMaterialType(material);
    gltfMaterial->setType(materialType);
    return gltfMaterial;

  } catch (const std::exception &e) {
    LOGE("Failed to convert material: %s", e.what());
    return GltfMaterial::createDefault();
  }
}

void
MaterialConverter::convertMaterialExtensions(const tinygltf::ExtensionMap &extensions,
                                             std::shared_ptr
                                                 <GltfMaterial>
                                             &gltfMaterial) {
  for (const auto &[extensionName, extensionValue]: extensions) {
    if (extensionName == "KHR_materials_unlit") {
      gltfMaterial->setType(MaterialType::UNLIT);
    } else if (extensionName == "KHR_materials_pbrSpecularGlossiness") {
      gltfMaterial->setType(MaterialType::SG);
    } else if (extensionName == "KHR_materials_clearcoat") {
      gltfMaterial->setHasClearcoat(true);
    } else if (extensionName == "KHR_materials_transmission") {
      gltfMaterial->setHasTransmission(true);
    } else if (extensionName == "KHR_materials_volume") {
      gltfMaterial->setHasVolume(true);
    } else if (extensionName == "KHR_materials_ior") {
      gltfMaterial->setHasIor(true);
    } else if (extensionName == "KHR_materials_sheen") {
      gltfMaterial->setHasSheen(true);
    } else if (extensionName == "KHR_materials_anisotropy") {
      gltfMaterial->setHasAnisotropy(true);
    } else if (extensionName == "KHR_materials_iridescence") {
      gltfMaterial->setHasIridescence(true);
    } else if (extensionName == "KHR_materials_specular") {
      gltfMaterial->setHasSpecular(true);
    } else if (extensionName == "KHR_materials_emissive_strength") {
      gltfMaterial->setHasEmissiveStrength(true);
    } else if (extensionName == "KHR_materials_specular") {
      gltfMaterial->setHasSpecular(true);
    }

  }
}

std::shared_ptr<GltfTextureInfo> MaterialConverter::convertTextureInfo(
    const tinygltf::TextureInfo &textureInfo) {
  try {
    std::optional<int> index = std::nullopt;
    if (textureInfo.index >= 0) {
      index = textureInfo.index;
    } else {
      return nullptr;
    }

    auto gltfTextureInfo = std::make_shared<GltfTextureInfo>(
        index,
        textureInfo.texCoord,
        false,
        "",
        true
    );

//            // 转换扩展
//            if (!textureInfo.extensions.empty()) {
//                convertTextureInfoExtensions(textureInfo.extensions, gltfTextureInfo.get());
//            }
//
//            // 转换额外数据
//            if (!textureInfo.extras.empty()) {
//                convertExtras(textureInfo.extras, gltfTextureInfo.get());
//            }

    return gltfTextureInfo;

  } catch (const std::exception &e) {
    LOGE("Failed to convert texture info: %s", e.what());
    return nullptr;
  }
}

std::shared_ptr<GltfTextureInfo>
MaterialConverter::convertMetallicRoughnessTextureInfo(
    const tinygltf::TextureInfo &textureInfo) {
  auto gltfTextureInfo = convertTextureInfo(textureInfo);
  if (gltfTextureInfo) {
    gltfTextureInfo->setLinear(true);
    gltfTextureInfo->setSamplerName("metallicRoughness");
  }
  return gltfTextureInfo;
}

std::shared_ptr<GltfTextureInfo> MaterialConverter::convertEmissiveTextureInfo(
    const tinygltf::TextureInfo &textureInfo) {
  auto gltfTextureInfo = convertTextureInfo(textureInfo);
  if (gltfTextureInfo) {
    gltfTextureInfo->setLinear(false);
    gltfTextureInfo->setGenerateMips(false);
  }
  return gltfTextureInfo;
}

std::shared_ptr<GltfTextureInfo> MaterialConverter::convertNormalTextureInfo(
    const tinygltf::NormalTextureInfo &normalInfo) {
  try {
    std::optional<int> index = std::nullopt;
    if (normalInfo.index >= 0) {
      index = normalInfo.index;
    } else {
      LOGW("Invalid normal texture index: %d", normalInfo.index);
      return nullptr;
    }

    auto textureInfo = std::make_shared<GltfTextureInfo>(
        index,
        normalInfo.texCoord,
        true,
        "",
        true
    );
    textureInfo->setScale(static_cast<float>(normalInfo.scale));
    textureInfo->setLinear(true);

    // 转换扩展（特别是纹理变换）
//            if (!normalInfo.extensions.empty()) {
//                convertTextureInfoExtensions(normalInfo.extensions, textureInfo.get());
//            }

    // 转换额外数据
//            if (!normalInfo.extras.empty()) {
//                convertExtras(normalInfo.extras, textureInfo.get());
//            }

    return textureInfo;

  } catch (const std::exception &e) {
    LOGE("Failed to convert normal texture info: %s", e.what());
    return nullptr;
  }
}

std::shared_ptr<GltfTextureInfo> MaterialConverter::convertOcclusionTextureInfo(
    const tinygltf::OcclusionTextureInfo &occlusionInfo) {
  try {
    std::optional<int> index = std::nullopt;
    if (occlusionInfo.index >= 0) {
      index = occlusionInfo.index;
    } else {
      LOGW("Invalid occlusion texture index: %d", occlusionInfo.index);
      return nullptr;
    }

    auto textureInfo = std::make_shared<GltfTextureInfo>(
        index,
        occlusionInfo.texCoord,
        true,
        "",
        true
    );

    textureInfo->setStrength(static_cast<float>(occlusionInfo.strength));
    textureInfo->setLinear(true);

//            // 转换扩展
//            if (!occlusionInfo.extensions.empty()) {
//                convertTextureInfoExtensions(occlusionInfo.extensions, textureInfo.get());
//            }
//
//            // 转换额外数据
//            if (!occlusionInfo.extras.empty()) {
//                convertExtras(occlusionInfo.extras, textureInfo.get());
//            }

    LOGI("Converted occlusion texture: index=%d, texCoord=%d, strength=%f",
         index.value(), occlusionInfo.texCoord, occlusionInfo.strength);

    return textureInfo;

  } catch (const std::exception &e) {
    LOGE("Failed to convert occlusion texture info: %s", e.what());
    return nullptr;
  }
}

MaterialType
MaterialConverter::determineMaterialType(const tinygltf::Material &material) {
  if (material.extensions.find("KHR_materials_unlit")
      != material.extensions.end()) {
    return MaterialType::UNLIT;
  }
  if (material.extensions.find("KHR_materials_pbrSpecularGlossiness")
      != material.extensions.end()) {
    return MaterialType::SG;
  }
  return MaterialType::MR;
}

AlphaMode MaterialConverter::convertAlphaMode(const std::string &alphaModeStr) {
  if (alphaModeStr == "OPAQUE") {
    return AlphaMode::OPAQUE;
  } else if (alphaModeStr == "MASK") {
    return AlphaMode::MASK;
  } else if (alphaModeStr == "BLEND") {
    return AlphaMode::BLEND;
  } else {
    LOGW("Unknown alpha mode: %s, using OPAQUE", alphaModeStr.c_str());
    return AlphaMode::OPAQUE;
  }
}

std::shared_ptr<PbrMetallicRoughness>
MaterialConverter::convertPbrMetallicRoughness(
    const tinygltf::PbrMetallicRoughness &pbr) {
  auto gltfPbr = std::make_shared<PbrMetallicRoughness>();
  if (pbr.baseColorFactor.size() >= 4) {
    glm::vec4 baseColorFactor(
        pbr.baseColorFactor[0],
        pbr.baseColorFactor[1],
        pbr.baseColorFactor[2],
        pbr.baseColorFactor[3]
    );
    gltfPbr->setBaseColorFactor(baseColorFactor);
  } else {
    gltfPbr->setBaseColorFactor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
  }

  if (pbr.baseColorTexture.index >= 0) {
    auto baseColorTexture = convertTextureInfo(pbr.baseColorTexture);
    gltfPbr->setBaseColorTexture(baseColorTexture);
  }
  gltfPbr->setMetallicFactor(static_cast<float>(pbr.metallicFactor));
  gltfPbr->setRoughnessFactor(static_cast<float>(pbr.roughnessFactor));
  if (pbr.metallicRoughnessTexture.index >= 0) {
    auto metallicRoughnessTexture =
        convertTextureInfo(pbr.metallicRoughnessTexture);
    gltfPbr->setMetallicRoughnessTexture(metallicRoughnessTexture);
  }

  return gltfPbr;
}

//    std::shared_ptr<GltfTextureInfo> MaterialConverter::convertTextureInfoFromValue(
//            const tinygltf::Value& value)
//    {
//        if (!value.IsObject()) {
//            LOGW("Texture info value is not an object");
//            return nullptr;
//        }
//
//        const auto& obj = value.Get<tinygltf::Value::Object>();
//
//        // 解析 index 字段（必需）
//        int textureIndex = -1;
//        if (obj.find("index") != obj.end()) {
//            const auto& indexValue = obj.at("index");
//            if (indexValue.IsInt()) {
//                textureIndex = indexValue.GetInt();
//            } else {
//                LOGE("Texture info 'index' is not an integer");
//                return nullptr;
//            }
//        } else {
//            LOGE("Texture info missing required 'index' field");
//            return nullptr;
//        }
//
//        // 解析 texCoord 字段（可选，默认0）
//        int texCoord = 0;
//        if (obj.find("texCoord") != obj.end()) {
//            const auto& texCoordValue = obj.at("texCoord");
//            if (texCoordValue.IsInt()) {
//                texCoord = texCoordValue.GetInt();
//            }
//        }
//
//        // 创建基础纹理信息结构
//        tinygltf::TextureInfo basicInfo;
//        basicInfo.index = textureIndex;
//        basicInfo.texCoord = texCoord;
//
//        // 转换扩展
//        if (obj.find("extensions") != obj.end()) {
//            const auto& extensions = obj.at("extensions");
//            if (extensions.IsObject()) {
//                // 将扩展数据转换为 tinygltf 格式
//                // 这里需要根据具体扩展进行处理
//              //  convertValueToExtensionMap(extensions, basicInfo.extensions);
//            }
//        }
//
//        // 转换额外数据
//        if (obj.find("extras") != obj.end()) {
//            const auto& extras = obj.at("extras");
//            basicInfo.extras = extras;
//        }
//
//        // 使用基础转换方法
//        return convertTextureInfo(basicInfo);
//    }
//
}