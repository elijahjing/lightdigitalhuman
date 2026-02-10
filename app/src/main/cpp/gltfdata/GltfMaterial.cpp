//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfMaterial.h"
#include "../utils/utils.h"
#include "GltfState.h"
#include <android/log.h>
#include <cmath>
#include <algorithm>

#include "../utils/LogUtils.h"

#include "GltfShader.h"

namespace digitalhumans {

// ===== KHRMaterialsPbrSpecularGlossiness实现 =====

KHRMaterialsPbrSpecularGlossiness::KHRMaterialsPbrSpecularGlossiness()
    : GltfObject(), diffuseFactor({1.0f, 1.0f, 1.0f, 1.0f}),
      diffuseTexture(nullptr),
      specularFactor({1.0f, 1.0f, 1.0f}), specularGlossinessTexture(nullptr),
      glossinessFactor(1.0f) {
}

//    void KHRMaterialsPbrSpecularGlossiness::fromJson(const JsonObject& json)
//    {
//        GltfObject::fromJson(json);
//
//        // 处理diffuseFactor
//        auto diffuseFactorIt = json.find("diffuseFactor");
//        if (diffuseFactorIt != json.end()) {
//            try {
//                const auto& array = std::any_cast<std::vector<std::any>>(diffuseFactorIt->second);
//                if (array.size() >= 4) {
//                    for (size_t i = 0; i < 4; ++i) {
//                        diffuseFactor[i] = std::any_cast<float>(array[i]);
//                    }
//                }
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//
//        // 处理specularFactor
//        auto specularFactorIt = json.find("specularFactor");
//        if (specularFactorIt != json.end()) {
//            try {
//                const auto& array = std::any_cast<std::vector<std::any>>(specularFactorIt->second);
//                if (array.size() >= 3) {
//                    for (size_t i = 0; i < 3; ++i) {
//                        specularFactor[i] = std::any_cast<float>(array[i]);
//                    }
//                }
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//
//        // 处理glossinessFactor
//        auto glossinessFactorIt = json.find("glossinessFactor");
//        if (glossinessFactorIt != json.end()) {
//            try {
//                glossinessFactor = std::any_cast<float>(glossinessFactorIt->second);
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//
//        // 处理diffuseTexture
//        auto diffuseTextureIt = json.find("diffuseTexture");
//        if (diffuseTextureIt != json.end()) {
//            try {
//                const auto& textureObj = std::any_cast<JsonObject>(diffuseTextureIt->second);
//                diffuseTexture = std::make_shared<GltfTextureInfo>();
//                diffuseTexture->fromJson(textureObj);
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//
//        // 处理specularGlossinessTexture
//        auto specularGlossinessTextureIt = json.find("specularGlossinessTexture");
//        if (specularGlossinessTextureIt != json.end()) {
//            try {
//                const auto& textureObj = std::any_cast<JsonObject>(specularGlossinessTextureIt->second);
//                specularGlossinessTexture = std::make_shared<GltfTextureInfo>();
//                specularGlossinessTexture->fromJson(textureObj);
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//    }



// ===== KHRMaterialsClearcoat实现 =====

KHRMaterialsClearcoat::KHRMaterialsClearcoat()
    : GltfObject(), clearcoatFactor(0.0f), clearcoatTexture(nullptr),
      clearcoatRoughnessFactor(0.0f),
      clearcoatRoughnessTexture(nullptr), clearcoatNormalTexture(nullptr) {
}

//    void KHRMaterialsClearcoat::fromJson(const JsonObject& json)
//    {
//        GltfObject::fromJson(json);
//
//        // 处理clearcoatFactor
//        auto clearcoatFactorIt = json.find("clearcoatFactor");
//        if (clearcoatFactorIt != json.end()) {
//            try {
//                clearcoatFactor = std::any_cast<float>(clearcoatFactorIt->second);
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//
//        // 处理clearcoatRoughnessFactor
//        auto clearcoatRoughnessFactorIt = json.find("clearcoatRoughnessFactor");
//        if (clearcoatRoughnessFactorIt != json.end()) {
//            try {
//                clearcoatRoughnessFactor = std::any_cast<float>(clearcoatRoughnessFactorIt->second);
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//
//        // 处理纹理
//        auto clearcoatTextureIt = json.find("clearcoatTexture");
//        if (clearcoatTextureIt != json.end()) {
//            try {
//                const auto& textureObj = std::any_cast<JsonObject>(clearcoatTextureIt->second);
//                clearcoatTexture = std::make_shared<GltfTextureInfo>();
//                clearcoatTexture->fromJson(textureObj);
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//
//        auto clearcoatRoughnessTextureIt = json.find("clearcoatRoughnessTexture");
//        if (clearcoatRoughnessTextureIt != json.end()) {
//            try {
//                const auto& textureObj = std::any_cast<JsonObject>(clearcoatRoughnessTextureIt->second);
//                clearcoatRoughnessTexture = std::make_shared<GltfTextureInfo>();
//                clearcoatRoughnessTexture->fromJson(textureObj);
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//
//        auto clearcoatNormalTextureIt = json.find("clearcoatNormalTexture");
//        if (clearcoatNormalTextureIt != json.end()) {
//            try {
//                const auto& textureObj = std::any_cast<JsonObject>(clearcoatNormalTextureIt->second);
//                clearcoatNormalTexture = std::make_shared<GltfTextureInfo>();
//                clearcoatNormalTexture->fromJson(textureObj);
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//    }

// ===== PbrMetallicRoughness实现 =====

PbrMetallicRoughness::PbrMetallicRoughness()
    : GltfObject(), baseColorFactor({1.0f, 1.0f, 1.0f, 1.0f}),
      baseColorTexture(nullptr), metallicFactor(1.0f),
      roughnessFactor(1.0f), metallicRoughnessTexture(nullptr) {
}

//    void PbrMetallicRoughness::fromJson(const JsonObject& json)
//    {
//        GltfObject::fromJson(json);
//
//        // 处理baseColorFactor
//        auto baseColorFactorIt = json.find("baseColorFactor");
//        if (baseColorFactorIt != json.end()) {
//            try {
//                const auto& array = std::any_cast<std::vector<std::any>>(baseColorFactorIt->second);
//                if (array.size() >= 4) {
//                    for (size_t i = 0; i < 4; ++i) {
//                        baseColorFactor[i] = std::any_cast<float>(array[i]);
//                    }
//                }
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//
//        // 处理metallicFactor
//        auto metallicFactorIt = json.find("metallicFactor");
//        if (metallicFactorIt != json.end()) {
//            try {
//                metallicFactor = std::any_cast<float>(metallicFactorIt->second);
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//
//        // 处理roughnessFactor
//        auto roughnessFactorIt = json.find("roughnessFactor");
//        if (roughnessFactorIt != json.end()) {
//            try {
//                roughnessFactor = std::any_cast<float>(roughnessFactorIt->second);
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//
//        // 处理baseColorTexture
//        auto baseColorTextureIt = json.find("baseColorTexture");
//        if (baseColorTextureIt != json.end()) {
//            try {
//                const auto& textureObj = std::any_cast<JsonObject>(baseColorTextureIt->second);
//                baseColorTexture = std::make_shared<GltfTextureInfo>();
//                baseColorTexture->fromJson(textureObj);
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//
//        // 处理metallicRoughnessTexture
//        auto metallicRoughnessTextureIt = json.find("metallicRoughnessTexture");
//        if (metallicRoughnessTextureIt != json.end()) {
//            try {
//                const auto& textureObj = std::any_cast<JsonObject>(metallicRoughnessTextureIt->second);
//                metallicRoughnessTexture = std::make_shared<GltfTextureInfo>();
//                metallicRoughnessTexture->fromJson(textureObj);
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//    }


// ===== GltfMaterial实现 =====

GltfMaterial::GltfMaterial()
    : GltfObject(), name(),
      pbrMetallicRoughness(std::make_shared<PbrMetallicRoughness>()),
      normalTexture(nullptr), occlusionTexture(nullptr),
      emissiveTexture(nullptr),
      emissiveFactor({0.0f, 0.0f, 0.0f}), alphaMode(AlphaMode::OPAQUE),
      alphaCutoff(0.5f), doubleSided(false),
      hasClearcoat(false), hasSheen(false), hasTransmission(false),
      hasDiffuseTransmission(false),
      hasIOR(false), hasEmissiveStrength(false), hasVolume(false),
      hasIridescence(false), hasAnisotropy(false),
      hasDispersion(false), hasSpecular(false), type(MaterialType::UNLIT),
      textures(), textureTransforms(),
      defines(), extensions() {
}

std::shared_ptr<GltfMaterial> GltfMaterial::createDefault() {
  auto defaultMaterial = std::make_shared<GltfMaterial>();
  defaultMaterial->type = MaterialType::MR;
  defaultMaterial->name = "Default Material";
  defaultMaterial->defines.push_back("MATERIAL_METALLICROUGHNESS 1");
  return defaultMaterial;
}

//
//    void GltfMaterial::fromJsonMaterialExtensions(const JsonObject& jsonExtensions)
//    {
//        // 处理KHR_materials_pbrSpecularGlossiness
//        auto pbrSpecularGlossinessIt = jsonExtensions.find("KHR_materials_pbrSpecularGlossiness");
//        if (pbrSpecularGlossinessIt != jsonExtensions.end()) {
//            try {
//                const auto& extensionObj = std::any_cast<JsonObject>(pbrSpecularGlossinessIt->second);
//                type = MaterialType::SG;
//                auto extension = std::make_shared<KHRMaterialsPbrSpecularGlossiness>();
//                extension->fromJson(extensionObj);
//                extensions["KHR_materials_pbrSpecularGlossiness"] = extension;
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//
//        // 处理KHR_materials_unlit
//        auto unlitIt = jsonExtensions.find("KHR_materials_unlit");
//        if (unlitIt != jsonExtensions.end()) {
//            type = MaterialType::UNLIT;
//        }
//
//        // 处理KHR_materials_clearcoat
//        auto clearcoatIt = jsonExtensions.find("KHR_materials_clearcoat");
//        if (clearcoatIt != jsonExtensions.end()) {
//            try {
//                const auto& extensionObj = std::any_cast<JsonObject>(clearcoatIt->second);
//                auto extension = std::make_shared<KHRMaterialsClearcoat>();
//                extension->fromJson(extensionObj);
//                extensions["KHR_materials_clearcoat"] = extension;
//            } catch (const std::bad_any_cast&) {
//                // 处理类型转换错误
//            }
//        }
//
//        // 处理其他扩展...
//        // 为简化，这里省略其他扩展的详细处理
//        // 实际实现中需要为每个扩展添加相应的处理逻辑
//    }

std::vector<std::string>
GltfMaterial::getDefines(const RenderingParameters &renderingParameters) const {
  std::vector<std::string> allDefines = defines;

  if (hasClearcoat
      && renderingParameters.enabledExtensions.KHR_materials_clearcoat) {
    allDefines.emplace_back("MATERIAL_CLEARCOAT 1");
  }
  if (hasSheen && renderingParameters.enabledExtensions.KHR_materials_sheen) {
    allDefines.emplace_back("MATERIAL_SHEEN 1");
  }
  if (hasTransmission
      && renderingParameters.enabledExtensions.KHR_materials_transmission) {
    allDefines.emplace_back("MATERIAL_TRANSMISSION 1");
  }
  if (hasDiffuseTransmission
      && renderingParameters.enabledExtensions.KHR_materials_diffuse_transmission) {
    allDefines.emplace_back("MATERIAL_DIFFUSE_TRANSMISSION 1");
  }
  if (hasVolume && renderingParameters.enabledExtensions.KHR_materials_volume) {
    allDefines.emplace_back("MATERIAL_VOLUME 1");
  }
  if (hasIOR && renderingParameters.enabledExtensions.KHR_materials_ior) {
    allDefines.emplace_back("MATERIAL_IOR 1");
  }
  if (hasSpecular
      && renderingParameters.enabledExtensions.KHR_materials_specular) {
    allDefines.emplace_back("MATERIAL_SPECULAR 1");
  }
  if (hasIridescence
      && renderingParameters.enabledExtensions.KHR_materials_iridescence) {
    allDefines.emplace_back("MATERIAL_IRIDESCENCE 1");
  }
  if (hasEmissiveStrength
      && renderingParameters.enabledExtensions.KHR_materials_emissive_strength) {
    allDefines.emplace_back("MATERIAL_EMISSIVE_STRENGTH 1");
  }
  if (hasAnisotropy
      && renderingParameters.enabledExtensions.KHR_materials_anisotropy) {
    allDefines.emplace_back("MATERIAL_ANISOTROPY 1");
  }
  if (hasDispersion
      && renderingParameters.enabledExtensions.KHR_materials_dispersion) {
    allDefines.emplace_back("MATERIAL_DISPERSION 1");
  }

  return allDefines;
}

void GltfMaterial::updateTextureTransforms(std::shared_ptr<GltfShader> shader) {
  if (!shader) {
    return;
  }

  for (const auto &transform: textureTransforms) {

    auto uvMatrix = glm::mat3(1.0f);  // 单位矩阵、
    // 1. 缩放变换 (手动构建)
    if (transform.scale.has_value()) {
      const auto &scale = transform.scale.value();

      glm::mat3 scaleMat = glm::mat3(
          scale[0], 0.0f, 0.0f,    // 第一列
          0.0f, scale[1], 0.0f,    // 第二列
          0.0f, 0.0f, 1.0f     // 第三列
      );

      uvMatrix = uvMatrix * scaleMat;
    }

    // 2. 旋转变换 (手动构建)
    if (transform.rotation.has_value()) {
      const float angle = transform.rotation.value();
      const float c = std::cos(angle);
      const float s = std::sin(angle);

      glm::mat3 rotMat = glm::mat3(
          c, s, 0.0f,    // 第一列
          -s, c, 0.0f,    // 第二列
          0.0f, 0.0f, 1.0f     // 第三列
      );

      uvMatrix = uvMatrix * rotMat;

      // 3. 平移变换 (手动构建)
      if (transform.offset.has_value()) {
        const auto &offset = transform.offset.value();

        glm::mat3 transMat = glm::mat3(
            1.0f, 0.0f, 0.0f,        // 第一列
            0.0f, 1.0f, 0.0f,        // 第二列
            offset[0], offset[1], 1.0f  // 第三列 (注意：平移在第三列)
        );

        uvMatrix = uvMatrix * transMat;
      }

      // 更新着色器
      std::string uniformName = "u_" + transform.key + "UVTransform";
      shader->updateUniform(uniformName, uvMatrix);

      if (transform.key == "Normal") {
        shader->updateUniform("u_vertNormalUVTransform", uvMatrix);
      }
    }
  }
}

void
GltfMaterial::parseTextureInfoExtensions(std::shared_ptr<GltfTextureInfo> textureInfo,
                                         const std::string &textureKey) {
//        if (!textureInfo || !textureInfo->hasExtensions()) {
//            return;
//        }
//
//        const auto& extensions = textureInfo->getExtensions();
//
//        // 查找KHR_texture_transform扩展
//        auto transformIt = extensions.find("KHR_texture_transform");
//        if (transformIt == extensions.end()) {
//            return;
//        }
//
//        try {
//            const auto& transformObj = std::any_cast<JsonObject>(transformIt->second);
//
//            TextureTransform transform;
//            transform.key = textureKey;
//
//            // 解析旋转
//            auto rotationIt = transformObj.find("rotation");
//            if (rotationIt != transformObj.end()) {
//                try {
//                    transform.rotation = std::any_cast<float>(rotationIt->second);
//                } catch (const std::bad_any_cast&) {
//                    // 处理类型转换错误
//                }
//            }
//
//            // 解析缩放
//            auto scaleIt = transformObj.find("scale");
//            if (scaleIt != transformObj.end()) {
//                try {
//                    const auto& scaleArray = std::any_cast<std::vector<std::any>>(scaleIt->second);
//                    if (scaleArray.size() >= 2) {
//                        std::array<float, 2> scaleVec;
//                        scaleVec[0] = std::any_cast<float>(scaleArray[0]);
//                        scaleVec[1] = std::any_cast<float>(scaleArray[1]);
//                        transform.scale = scaleVec;
//                    }
//                } catch (const std::bad_any_cast&) {
//                    // 处理类型转换错误
//                }
//            }
//
//            // 解析偏移
//            auto offsetIt = transformObj.find("offset");
//            if (offsetIt != transformObj.end()) {
//                try {
//                    const auto& offsetArray = std::any_cast<std::vector<std::any>>(offsetIt->second);
//                    if (offsetArray.size() >= 2) {
//                        std::array<float, 2> offsetVec;
//                        offsetVec[0] = std::any_cast<float>(offsetArray[0]);
//                        offsetVec[1] = std::any_cast<float>(offsetArray[1]);
//                        transform.offset = offsetVec;
//                    }
//                } catch (const std::bad_any_cast&) {
//                    // 处理类型转换错误
//                }
//            }
//
//            // 解析纹理坐标索引
//            auto texCoordIt = transformObj.find("texCoord");
//            if (texCoordIt != transformObj.end()) {
//                try {
//                    int texCoordIndex = std::any_cast<int>(texCoordIt->second);
//                    transform.texCoord = texCoordIndex;
//                    textureInfo->setTexCoord(texCoordIndex);
//                } catch (const std::bad_any_cast&) {
//                    // 处理类型转换错误
//                }
//            }
//
//            // 添加纹理变换到列表
//            textureTransforms.push_back(transform);
//
//            // 添加着色器宏定义
//            std::string defineStr = "HAS_" + textureKey;
//            std::transform(defineStr.begin(), defineStr.end(), defineStr.begin(), ::toupper);
//            defineStr += "_UV_TRANSFORM 1";
//            defines.push_back(defineStr);
//
//        } catch (const std::bad_any_cast&) {
//            LOGW("Failed to parse KHR_texture_transform extension for texture: %s", textureKey.c_str());
//        }
}

void GltfMaterial::initObjects() {
  // 初始化法线纹理
  if (normalTexture) {
    normalTexture->setSamplerName("u_NormalSampler");
    parseTextureInfoExtensions(normalTexture, "Normal");
    textures.push_back(normalTexture);
    defines.push_back("HAS_NORMAL_MAP 1");
  }

  // 初始化遮挡纹理
  if (occlusionTexture) {
    occlusionTexture->setSamplerName("u_OcclusionSampler");
    parseTextureInfoExtensions(occlusionTexture, "Occlusion");
    textures.push_back(occlusionTexture);
    defines.push_back("HAS_OCCLUSION_MAP 1");
  }

  // 初始化发光纹理
  if (emissiveTexture) {
    emissiveTexture->setSamplerName("u_EmissiveSampler");
    parseTextureInfoExtensions(emissiveTexture, "Emissive");
    textures.push_back(emissiveTexture);
    defines.push_back("HAS_EMISSIVE_MAP 1");
  }

  // 初始化PBR纹理
  if (pbrMetallicRoughness) {
    if (pbrMetallicRoughness->getBaseColorTexture()) {
      auto baseColorTexture = pbrMetallicRoughness->getBaseColorTexture();
      baseColorTexture->setSamplerName("u_BaseColorSampler");
      parseTextureInfoExtensions(baseColorTexture, "BaseColor");
      textures.push_back(baseColorTexture);
      defines.push_back("HAS_BASE_COLOR_MAP 1");
    }

    if (pbrMetallicRoughness->getMetallicRoughnessTexture()) {
      auto metallicRoughnessTexture =
          pbrMetallicRoughness->getMetallicRoughnessTexture();
      metallicRoughnessTexture->setSamplerName("u_MetallicRoughnessSampler");
      parseTextureInfoExtensions(metallicRoughnessTexture, "MetallicRoughness");
      textures.push_back(metallicRoughnessTexture);
      defines.push_back("HAS_METALLIC_ROUGHNESS_MAP 1");
    }
  }

  // 处理扩展纹理
  //   processExtensionTextures();

  // 设置Alpha模式宏定义
  defines.push_back("ALPHAMODE_OPAQUE 0");
  defines.push_back("ALPHAMODE_MASK 1");
  defines.push_back("ALPHAMODE_BLEND 2");

  switch (alphaMode) {
    case AlphaMode::MASK:
      defines.push_back("ALPHAMODE ALPHAMODE_MASK");
      break;
    case AlphaMode::OPAQUE:
      defines.push_back("ALPHAMODE ALPHAMODE_OPAQUE");
      break;
    case AlphaMode::BLEND:
      defines.push_back("ALPHAMODE ALPHAMODE_BLEND");
      break;
  }

  // 设置材质类型宏定义
  if (type != MaterialType::SG) {
    defines.push_back("MATERIAL_METALLICROUGHNESS 1");
  }

  // 处理材质扩展
  processExtensions();

}

//    void GltfMaterial::processExtensionTextures()
//    {
//        // 处理镜面光泽度扩展
//        auto pbrSGExtension = getExtension<KHRMaterialsPbrSpecularGlossiness>("KHR_materials_pbrSpecularGlossiness");
//        if (pbrSGExtension) {
//            if (pbrSGExtension->getDiffuseTexture()) {
//                auto diffuseTexture = pbrSGExtension->getDiffuseTexture();
//                diffuseTexture->setSamplerName("u_DiffuseSampler");
//                diffuseTexture->setLinear(false);
//                parseTextureInfoExtensions(diffuseTexture, "Diffuse");
//                textures.push_back(diffuseTexture);
//                defines.push_back("HAS_DIFFUSE_MAP 1");
//            }
//
//            if (pbrSGExtension->getSpecularGlossinessTexture()) {
//                auto specularGlossinessTexture = pbrSGExtension->getSpecularGlossinessTexture();
//                specularGlossinessTexture->setSamplerName("u_SpecularGlossinessSampler");
//                specularGlossinessTexture->setLinear(false);
//                parseTextureInfoExtensions(specularGlossinessTexture, "SpecularGlossiness");
//                textures.push_back(specularGlossinessTexture);
//                defines.push_back("HAS_SPECULAR_GLOSSINESS_MAP 1");
//            }
//        }
//
//        // 处理清漆扩展
//        auto clearcoatExtension = getExtension<KHRMaterialsClearcoat>("KHR_materials_clearcoat");
//        if (clearcoatExtension) {
//            hasClearcoat = true;
//
//            if (clearcoatExtension->getClearcoatTexture()) {
//                auto clearcoatTexture = clearcoatExtension->getClearcoatTexture();
//                clearcoatTexture->setSamplerName("u_ClearcoatSampler");
//                parseTextureInfoExtensions(clearcoatTexture, "Clearcoat");
//                textures.push_back(clearcoatTexture);
//                defines.push_back("HAS_CLEARCOAT_MAP 1");
//            }
//
//            if (clearcoatExtension->getClearcoatRoughnessTexture()) {
//                auto clearcoatRoughnessTexture = clearcoatExtension->getClearcoatRoughnessTexture();
//                clearcoatRoughnessTexture->setSamplerName("u_ClearcoatRoughnessSampler");
//                parseTextureInfoExtensions(clearcoatRoughnessTexture, "ClearcoatRoughness");
//                textures.push_back(clearcoatRoughnessTexture);
//                defines.push_back("HAS_CLEARCOAT_ROUGHNESS_MAP 1");
//            }
//
//            if (clearcoatExtension->getClearcoatNormalTexture()) {
//                auto clearcoatNormalTexture = clearcoatExtension->getClearcoatNormalTexture();
//                clearcoatNormalTexture->setSamplerName("u_ClearcoatNormalSampler");
//                parseTextureInfoExtensions(clearcoatNormalTexture, "ClearcoatNormal");
//                textures.push_back(clearcoatNormalTexture);
//                defines.push_back("HAS_CLEARCOAT_NORMAL_MAP 1");
//            }
//        }
//
//        // 可以继续添加其他扩展的纹理处理...
//    }
//
void GltfMaterial::processExtensions() {
  // 处理unlit扩展
  if (extensions.find("KHR_materials_unlit") != extensions.end()) {
    defines.push_back("MATERIAL_UNLIT 1");
  }

  // 处理镜面光泽度扩展
  if (extensions.find("KHR_materials_pbrSpecularGlossiness")
      != extensions.end()) {
    defines.push_back("MATERIAL_SPECULARGLOSSINESS 1");
  }

  // 其他扩展的处理可以继续添加...
}

template<typename T>
std::shared_ptr<T>
GltfMaterial::getExtension(const std::string &extensionName) const {
  auto it = extensions.find(extensionName);
  if (it != extensions.end()) {
    return std::dynamic_pointer_cast<T>(it->second);
  }
  return nullptr;
}


const std::vector<TextureTransform> &
GltfMaterial::getTextureTransforms() const {
  return textureTransforms;
}

void GltfMaterial::setHasClearcoat(bool hasClearcoat) {
  GltfMaterial::hasClearcoat = hasClearcoat;
}

void GltfMaterial::setHasSheen(bool hasSheen) {
  GltfMaterial::hasSheen = hasSheen;
}

void GltfMaterial::setHasTransmission(bool hasTransmission) {
  GltfMaterial::hasTransmission = hasTransmission;
}

void GltfMaterial::setHasDiffuseTransmission(bool hasDiffuseTransmission) {
  GltfMaterial::hasDiffuseTransmission = hasDiffuseTransmission;
}

void GltfMaterial::setHasIor(bool hasIor) {
  hasIOR = hasIor;
}

void GltfMaterial::setHasEmissiveStrength(bool hasEmissiveStrength) {
  GltfMaterial::hasEmissiveStrength = hasEmissiveStrength;
}

void GltfMaterial::setHasVolume(bool hasVolume) {
  GltfMaterial::hasVolume = hasVolume;
}

void GltfMaterial::setHasIridescence(bool hasIridescence) {
  GltfMaterial::hasIridescence = hasIridescence;
}

void GltfMaterial::setHasAnisotropy(bool hasAnisotropy) {
  GltfMaterial::hasAnisotropy = hasAnisotropy;
}

void GltfMaterial::setHasDispersion(bool hasDispersion) {
  GltfMaterial::hasDispersion = hasDispersion;
}

void GltfMaterial::setHasSpecular(bool hasSpecular) {
  GltfMaterial::hasSpecular = hasSpecular;
}

} // namespace digitalhumans