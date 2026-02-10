//
// Created by vincentsyan on 2025/8/19.
//

#include "GltfConverter.h"
#include "../utils/LogUtils.h"
#include "converter/MaterialConverter.h"
#include "Gltf.h"
#include "GltfBuffer.h"
#include "GltfAsset.h"
#include "GltfBufferView.h"
#include "GltfAccessor.h"
#include "GltfImage.h"
#include "GltfSampler.h"
#include "GltfTexture.h"
#include "GltfMaterial.h"
#include "GltfMesh.h"
#include "GltfNode.h"
#include "GltfScene.h"
#include "GltfSkin.h"
#include "GltfLight.h"
#include "GltfCamera.h"
#include "GltfAnimation.h"
#include "GltfObject.h"
#include "GltfPrimitive.h"
#include "GltfMaterial.h"
#include "../../engine/Engine.h"
#include "UserCamera.h"


namespace digitalhumans {

std::shared_ptr<Gltf> GltfConverter::convert(const tinygltf::Model &model,
                                             Engine &gltfView,
                                             const std::string &filePath) {
  auto gltf = std::make_shared<Gltf>(filePath);
  if (!gltf) {
    LOGE("创建Gltf对象失败");
    return nullptr;
  }
  try {
    // 转换 Asset
    if (!model.asset.version.empty()) {
      gltf->setAsset(convertAsset(model.asset));
    }

    // 转换 Buffers
    for (const auto &buffer: model.buffers) {
      gltf->buffers.push_back(convertBuffer(buffer));
    }

    // 转换 BufferViews
    for (const auto &bufferView: model.bufferViews) {
      gltf->bufferViews.push_back(convertBufferView(bufferView));
    }

    // 转换 Accessors
    for (const auto &accessor: model.accessors) {
      gltf->accessors.push_back(convertAccessor(accessor));
    }

    // 转换 Images
    for (const auto &image: model.images) {
      gltf->images.push_back(convertImage(image));
    }

    // 转换 Samplers
    for (const auto &sampler: model.samplers) {
      gltf->samplers.push_back(convertSampler(sampler));
    }

    // 转换 Textures
    for (const auto &texture: model.textures) {
      gltf->textures.push_back(convertTexture(texture));
    }

    // 转换 Materials
    for (const auto &material: model.materials) {
      auto newMaterial = MaterialConverter::convertMaterial(material);
      newMaterial->initObjects();
      gltf->materials.push_back(newMaterial);
    }

    // 转换 Meshes
    for (const auto &mesh: model.meshes) {
      gltf->meshes.push_back(convertMesh(mesh, gltf, gltfView));
    }

    // 转换 Nodes
    for (const auto &node: model.nodes) {
      auto gltfNode = convertNode(node);
      gltf->addNode(gltfNode);
    }

    // 转换 Scenes
    for (const auto &scene: model.scenes) {
      gltf->addScene(convertScene(scene));
    }

    // 转换 Skins
    for (const auto &skin: model.skins) {
      auto gltfSkin = convertSkin(skin);
      gltfSkin->initGl(gltf, gltfView.context);
      gltf->addSkin(gltfSkin);
    }

    // 转换 Cameras
    for (const auto &camera: model.cameras) {
      gltf->addCamera(convertCamera(camera));
    }

    // 转换 Animations
    for (const auto &animation: model.animations) {
      auto gltfAnimation = convertAnimation(animation);
      gltfAnimation->initGl(gltf, gltfView.context);
      gltf->animations.push_back(gltfAnimation);
    }

    auto lights = convertLights(model);
    for (auto &light: lights) {
      gltf->lights.push_back(light);
    }
    // 设置默认场景
    gltf->setScene(model.defaultScene);
    return gltf;

  } catch (const std::exception &e) {
    LOGE("Exception during glTF conversion: %s", e.what());
    return nullptr;
  }
}

std::vector<std::shared_ptr<GltfLight>>
GltfConverter::convertLights(const tinygltf::Model &model) {
  std::vector<std::shared_ptr<GltfLight>> lights;

  try {
    auto extensionsIt = model.extensions.find("KHR_lights_punctual");
    if (extensionsIt != model.extensions.end()) {
      const tinygltf::Value &extension = extensionsIt->second;
      if (extension.Has("lights") && extension.Get("lights").IsArray()) {
        const tinygltf::Value &lightsArray = extension.Get("lights");

        for (size_t i = 0; i < lightsArray.ArrayLen(); ++i) {
          auto light =
              convertSingleLight(lightsArray.Get(i), static_cast<int>(i));
          if (light) {
            lights.push_back(light);
          }
        }
      }
    }

  } catch (const std::exception &e) {
    LOGE("Error converting lights: %s", e.what());
  }

  return lights;
}

std::shared_ptr<GltfLight>
GltfConverter::convertSingleLight(const tinygltf::Value &lightData, int index) {
  if (!lightData.IsObject()) {
    LOGE("Light data at index %d is not an object", index);
    return nullptr;
  }

  auto light = std::make_shared<GltfLight>();

  try {
    // 设置名称
    if (lightData.Has("name") && lightData.Get("name").IsString()) {
      light->setName(lightData.Get("name").Get<std::string>());
    } else {
      light->setName("Light_" + std::to_string(index));
    }

    // 设置类型
    if (lightData.Has("type") && lightData.Get("type").IsString()) {
      std::string typeStr = lightData.Get("type").Get<std::string>();
      if (typeStr == "directional") {
        light->setType(LightType::DIRECTIONAL);
      } else if (typeStr == "point") {
        light->setType(LightType::POINT);
      } else if (typeStr == "spot") {
        light->setType(LightType::SPOT);
      } else {
        LOGW("Unknown light type '%s' for light %d, defaulting to directional",
             typeStr.c_str(), index);
        light->setType(LightType::DIRECTIONAL);
      }
    } else {
      LOGE("Light %d missing required 'type' field", index);
      return nullptr;
    }

    // 设置颜色
    if (lightData.Has("color") && lightData.Get("color").IsArray()) {
      const auto &colorArray = lightData.Get("color");
      if (colorArray.ArrayLen() >= 3) {
        glm::vec3 color(
            static_cast<float>(colorArray.Get(0).GetNumberAsDouble()),
            static_cast<float>(colorArray.Get(1).GetNumberAsDouble()),
            static_cast<float>(colorArray.Get(2).GetNumberAsDouble())
        );
        light->setColor(color);
      }
    }

    // 设置强度
    if (lightData.Has("intensity") && lightData.Get("intensity").IsNumber()) {
      light->setIntensity(static_cast<float>(lightData.Get("intensity").GetNumberAsDouble()));
    }

    // 设置范围
    if (lightData.Has("range") && lightData.Get("range").IsNumber()) {
      light->setRange(static_cast<float>(lightData.Get("range").GetNumberAsDouble()));
    }

    // 设置聚光灯参数
    if (light->getType() == LightType::SPOT && lightData.Has("spot")) {
      const auto &spotData = lightData.Get("spot");
      if (spotData.IsObject()) {
        auto spotParams = std::make_shared<GltfLightSpot>();

        if (spotData.Has("innerConeAngle")
            && spotData.Get("innerConeAngle").IsNumber()) {
          spotParams->setInnerConeAngle(
              static_cast<float>(spotData.Get("innerConeAngle").GetNumberAsDouble())
          );
        }

        if (spotData.Has("outerConeAngle")
            && spotData.Get("outerConeAngle").IsNumber()) {
          spotParams->setOuterConeAngle(
              static_cast<float>(spotData.Get("outerConeAngle").GetNumberAsDouble())
          );
        }

        light->setSpot(spotParams);
      }
    }

    LOGD("Successfully converted light '%s' (type: %d)",
         light->getName().c_str(), static_cast<int>(light->getType()));

    return light;

  } catch (const std::exception &e) {
    LOGE("Error converting light %d: %s", index, e.what());
    return nullptr;
  }
}
// GltfConverter.cpp


std::shared_ptr<GltfCamera>
GltfConverter::convertCamera(const tinygltf::Camera &camera) {
  try {
    auto gltfCamera = std::make_shared<UserCamera>();

    // 设置摄像机名称
    gltfCamera->setName(camera.name);

    // 转换摄像机类型
    if (camera.type == "perspective") {
      gltfCamera->setType(GltfCamera::Type::PERSPECTIVE);

      // 转换透视摄像机参数
      if (!convertPerspectiveCamera(camera, gltfCamera.get())) {
        LOGE("Failed to convert perspective camera parameters for '%s'",
             camera.name.c_str());
        return nullptr;
      }
    } else if (camera.type == "orthographic") {
      gltfCamera->setType(GltfCamera::Type::ORTHOGRAPHIC);

      // 转换正交摄像机参数
      if (!convertOrthographicCamera(camera, gltfCamera.get())) {
        LOGE("Failed to convert orthographic camera parameters for '%s'",
             camera.name.c_str());
        return nullptr;
      }
    } else {
      LOGE("Unknown camera type '%s' for camera '%s'",
           camera.type.c_str(),
           camera.name.c_str());
      return nullptr;
    }

//            // 转换扩展数据
//            if (!camera.extensions.empty()) {
//                convertCameraExtensions(camera.extensions, gltfCamera.get());
//            }
//
//            // 转换额外数据
//            if (!camera.extras.empty()) {
//                convertExtras(camera.extras, gltfCamera.get());
//            }

    // 验证转换结果
    if (!gltfCamera->validate()) {
      LOGW("Converted camera '%s' failed validation", camera.name.c_str());
    }

    LOGI("Successfully converted %s camera: '%s'",
         camera.type.c_str(),
         camera.name.empty() ? "unnamed" : camera.name.c_str());

    return gltfCamera;

  } catch (const std::exception &e) {
    LOGE("Failed to convert camera '%s': %s",
         camera.name.empty() ? "unnamed" : camera.name.c_str(), e.what());
    return nullptr;
  }
}

bool GltfConverter::convertOrthographicCamera(const tinygltf::Camera &camera,
                                              GltfCamera *gltfCamera) {
  try {
    auto &orthographic = gltfCamera->getOrthographic();

    // tinygltf::Camera 有 orthographic 成员变量
    const auto &orthographicData = camera.orthographic;

    // 验证并设置水平放大倍数（必需）
    if (orthographicData.xmag <= 0.0f) {
      LOGE("Invalid xmag value: %f (must be positive)", orthographicData.xmag);
      return false;
    }
    orthographic.setXmag(orthographicData.xmag);

    // 验证并设置垂直放大倍数（必需）
    if (orthographicData.ymag <= 0.0f) {
      LOGE("Invalid ymag value: %f (must be positive)", orthographicData.ymag);
      return false;
    }
    orthographic.setYmag(orthographicData.ymag);

    // 验证并设置近平面（必需）
    if (orthographicData.znear < 0.0f) {
      LOGE("Invalid znear value: %f (must be non-negative)",
           orthographicData.znear);
      return false;
    }
    orthographic.setZnear(orthographicData.znear);

    // 验证并设置远平面（必需）
    if (orthographicData.zfar <= orthographicData.znear) {
      LOGE("Invalid zfar value: %f (must be greater than znear: %f)",
           orthographicData.zfar, orthographicData.znear);
      return false;
    }
    orthographic.setZfar(orthographicData.zfar);

    LOGI("Converted orthographic camera: xmag=%f, ymag=%f, znear=%f, zfar=%f",
         orthographic.getXmag(), orthographic.getYmag(),
         orthographic.getZnear(), orthographic.getZfar());

    return true;

  } catch (const std::exception &e) {
    LOGE("Failed to convert orthographic camera parameters: %s", e.what());
    return false;
  }
}

bool GltfConverter::convertPerspectiveCamera(const tinygltf::Camera &camera,
                                             GltfCamera *gltfCamera) {
  try {
    auto &perspective = gltfCamera->getPerspective();

    // tinygltf::Camera 有 perspective 成员变量
    const auto &perspectiveData = camera.perspective;

    // 设置垂直视场角（必需）
    perspective.setYfov(perspectiveData.yfov);

    // 验证视场角范围
    if (perspectiveData.yfov <= 0.0f || perspectiveData.yfov >= M_PI) {
      LOGE("Invalid yfov value: %f (must be between 0 and π)",
           perspectiveData.yfov);
      return false;
    }

    // 设置宽高比（可选）
    if (perspectiveData.aspectRatio > 0.0f) {
      perspective.setAspectRatio(perspectiveData.aspectRatio);
    } else {
      perspective.setAspectRatio(std::nullopt);
    }

    // 设置近平面（必需）
    if (perspectiveData.znear <= 0.0f) {
      LOGE("Invalid znear value: %f (must be positive)", perspectiveData.znear);
      return false;
    }
    perspective.setZnear(perspectiveData.znear);

    // 设置远平面（可选）
    if (perspectiveData.zfar > 0.0f) {
      if (perspectiveData.zfar <= perspectiveData.znear) {
        LOGE("Invalid zfar value: %f (must be greater than znear: %f)",
             perspectiveData.zfar, perspectiveData.znear);
        return false;
      }
      perspective.setZfar(perspectiveData.zfar);
    } else {
      // 如果 zfar <= 0，使用无限远平面
      perspective.setZfar(1000.0f);
    }

    LOGI(
        "Converted perspective camera: yfov=%f, aspectRatio=%s, znear=%f, zfar=%f",
        perspective.getYfov(),
        perspective.getAspectRatio().has_value() ?
        std::to_string(perspective.getAspectRatio().value()).c_str() : "auto",
        perspective.getZnear(),
        perspective.getZfar());

    return true;

  } catch (const std::exception &e) {
    LOGE("Failed to convert perspective camera parameters: %s", e.what());
    return false;
  }
}

// GltfConverter.cpp

std::shared_ptr<GltfSkin>
GltfConverter::convertSkin(const tinygltf::Skin &skin) {
  try {
    auto gltfSkin = std::make_shared<GltfSkin>();

    // 设置蒙皮名称
    gltfSkin->setName(skin.name);

    // 设置逆绑定矩阵访问器索引
    if (skin.inverseBindMatrices >= 0) {
      gltfSkin->setInverseBindMatrices(skin.inverseBindMatrices);
    } else {
      // 没有逆绑定矩阵，使用单位矩阵
      gltfSkin->setInverseBindMatrices(std::nullopt);
      LOGI("Skin '%s' has no inverse bind matrices, will use identity matrices",
           skin.name.c_str());
    }

    // 设置关节索引数组
    if (!skin.joints.empty()) {
      std::vector<int> validJoints;
      validJoints.reserve(skin.joints.size());

      for (size_t i = 0; i < skin.joints.size(); ++i) {
        int jointIndex = skin.joints[i];

        // 验证关节索引
        if (jointIndex >= 0) {
          validJoints.push_back(jointIndex);
        } else {
          LOGW("Invalid joint index %d at position %zu in skin '%s', skipping",
               jointIndex, i, skin.name.c_str());
        }
      }

      if (validJoints.empty()) {
        LOGE("Skin '%s' has no valid joints", skin.name.c_str());
        return nullptr;
      }

      gltfSkin->setJoints(validJoints);
      LOGI("Skin '%s' has %zu valid joints (from %zu total)",
           skin.name.c_str(), validJoints.size(), skin.joints.size());
    } else {
      LOGE("Skin '%s' has no joints defined", skin.name.c_str());
      return nullptr;
    }

    // 设置骨架根节点索引
    if (skin.skeleton >= 0) {
      gltfSkin->setSkeleton(skin.skeleton);
    } else {
      gltfSkin->setSkeleton(std::nullopt);
      LOGI("Skin '%s' has no skeleton root specified", skin.name.c_str());
    }

//            // 转换扩展数据
//            if (!skin.extensions.empty()) {
//                convertSkinExtensions(skin.extensions, gltfSkin.get());
//            }
//
//            // 转换额外数据
//            if (!skin.extras.empty()) {
//                convertExtras(skin.extras, gltfSkin.get());
//            }

    // 验证转换结果
    if (!gltfSkin->validate()) {
      LOGW("Converted skin '%s' failed validation", skin.name.c_str());
    }

    LOGI("Successfully converted skin: '%s' with %zu joints",
         skin.name.empty() ? "unnamed" : skin.name.c_str(),
         gltfSkin->getJointCount());

    return gltfSkin;

  } catch (const std::exception &e) {
    LOGE("Failed to convert skin '%s': %s",
         skin.name.empty() ? "unnamed" : skin.name.c_str(), e.what());
    return nullptr;
  }
}


std::shared_ptr<GltfScene>
GltfConverter::convertScene(const tinygltf::Scene &scene) {
  try {
    // 转换节点索引列表
    std::vector<int> nodeIndices;
    nodeIndices.reserve(scene.nodes.size());

    // 复制并验证节点索引
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
      int nodeIndex = scene.nodes[i];

      // 基本验证（更详细的验证需要模型上下文）
      if (nodeIndex >= 0) {
        nodeIndices.push_back(nodeIndex);
      } else {
        LOGW("Invalid node index %d in scene %s, skipping",
             nodeIndex, scene.name.c_str());
      }
    }

    // 创建场景对象
    auto gltfScene = std::make_shared<GltfScene>(nodeIndices, scene.name);

    // 转换扩展数据
    if (!scene.extensions.empty()) {
      //   convertSceneExtensions(scene.extensions, gltfScene.get());
    }

    // 转换额外数据
//            if (!scene.extras.empty()) {
//                convertExtras(scene.extras, gltfScene.get());
//            }

    LOGI("Converted scene: '%s' with %zu root nodes",
         scene.name.empty() ? "unnamed" : scene.name.c_str(),
         nodeIndices.size());

    return gltfScene;

  } catch (const std::exception &e) {
    LOGE("Failed to convert scene '%s': %s",
         scene.name.empty() ? "unnamed" : scene.name.c_str(), e.what());
    return nullptr;
  }
}


std::shared_ptr<GltfTexture>
GltfConverter::convertTexture(const tinygltf::Texture &texture) {
  try {

    // 处理索引
    std::optional<int> samplerIndex = (texture.sampler >= 0) ?
                                      std::make_optional(texture.sampler)
                                                             : std::nullopt;
    std::optional<int> sourceIndex = (texture.source >= 0) ?
                                     std::make_optional(texture.source)
                                                           : std::nullopt;

    // 验证必要参数
    if (!sourceIndex.has_value()) {
      LOGW("Texture has no source image, this may cause issues");
    }

    // 确定纹理类型（可能从扩展中获取）
    GLenum textureType = GL_TEXTURE_2D;  // 默认类型

    // 检查是否有立方体贴图扩展
    if (texture.extensions.find("EXT_texture_cube_map")
        != texture.extensions.end()) {
      textureType = GL_TEXTURE_CUBE_MAP;
    }

    // 创建 GltfTexture 对象
    auto gltfTexture =
        std::make_shared<GltfTexture>(samplerIndex, sourceIndex, textureType);

    // 处理线性空间设置（可能从扩展或约定中推断）
    bool isLinear = inferLinearSpace(texture, sourceIndex);
    gltfTexture->setLinear(isLinear);
    gltfTexture->setType(textureType);
    // 转换扩展数据
    convertTextureExtensions(texture.extensions, gltfTexture.get());

    // 转换额外数据
    convertExtras(texture.extras, gltfTexture.get());

    LOGI("Converted texture: sampler=%d, source=%d, type=0x%04X, name='%s'",
         samplerIndex.value_or(-1), sourceIndex.value_or(-1),
         textureType, texture.name.c_str());

    return gltfTexture;

  } catch (const std::exception &e) {
    LOGE("Exception converting texture: %s", e.what());
    return nullptr;
  }
}




//    void GltfConverter::convertMaterialExtensions(
//            const tinygltf::ExtensionMap& extensions,
//            GltfMaterial* gltfMaterial)
//    {
//        for (const auto& [extName, extValue] : extensions) {
//            if (extName == "KHR_materials_pbrSpecularGlossiness") {
//                convertKHRMaterialsPbrSpecularGlossiness(extValue, gltfMaterial);
//            } else if (extName == "KHR_materials_clearcoat") {
//                convertKHRMaterialsClearcoat(extValue, gltfMaterial);
//            } else if (extName == "KHR_materials_sheen") {
//                convertKHRMaterialsSheen(extValue, gltfMaterial);
//            } else if (extName == "KHR_materials_transmission") {
//                convertKHRMaterialsTransmission(extValue, gltfMaterial);
//            } else if (extName == "KHR_materials_specular") {
//                convertKHRMaterialsSpecular(extValue, gltfMaterial);
//            } else if (extName == "KHR_materials_volume") {
//                convertKHRMaterialsVolume(extValue, gltfMaterial);
//            } else if (extName == "KHR_materials_iridescence") {
//                convertKHRMaterialsIridescence(extValue, gltfMaterial);
//            } else if (extName == "KHR_materials_anisotropy") {
//                convertKHRMaterialsAnisotropy(extValue, gltfMaterial);
//            } else if (extName == "KHR_materials_emissive_strength") {
//                convertKHRMaterialsEmissiveStrength(extValue, gltfMaterial);
//            } else if (extName == "KHR_materials_ior") {
//                convertKHRMaterialsIor(extValue, gltfMaterial);
//            } else if (extName == "KHR_materials_dispersion") {
//                convertKHRMaterialsDispersion(extValue, gltfMaterial);
//            } else if (extName == "KHR_materials_diffuse_transmission") {
//                convertKHRMaterialsDiffuseTransmission(extValue, gltfMaterial);
//            } else if (extName == "KHR_materials_unlit") {
//                // Unlit 扩展通常没有额外参数
//                LOGI("Material uses unlit shading model");
//            } else {
//                LOGW("Unhandled material extension: %s", extName.c_str());
//            }
//        }
//    }


// GltfConverter.cpp - 辅助方法

void GltfConverter::convertTextureExtensions(
    const tinygltf::ExtensionMap &extensions,
    GltfTexture *gltfTexture) {
  for (const auto &[extName, extValue]: extensions) {
    if (extName == "KHR_texture_transform") {
      // 处理纹理变换扩展
      convertKHRTextureTransform(extValue, gltfTexture);
    } else if (extName == "EXT_texture_cube_map") {
      // 处理立方体贴图扩展
      gltfTexture->setType(GL_TEXTURE_CUBE_MAP);
    } else if (extName == "KHR_texture_basisu") {
      // 处理 Basis Universal 纹理扩展
      //   convertKHRTextureBasisU(extValue, gltfTexture);
    } else {
      // 其他扩展
      LOGW("Unhandled texture extension: %s", extName.c_str());
    }
  }
}

bool GltfConverter::inferLinearSpace(const tinygltf::Texture &texture,
                                     std::optional<int> sourceIndex) {
  // 根据用途推断颜色空间
  // 这需要上下文信息，这里提供一个简单的实现

  // 如果纹理名称包含某些关键词，可能是线性空间
  std::string name = texture.name;
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);

  if (name.find("normal") != std::string::npos ||
      name.find("roughness") != std::string::npos ||
      name.find("metallic") != std::string::npos ||
      name.find("occlusion") != std::string::npos ||
      name.find("ao") != std::string::npos) {
    return true;  // 通常这些是线性空间
  }

  if (name.find("diffuse") != std::string::npos ||
      name.find("albedo") != std::string::npos ||
      name.find("basecolor") != std::string::npos ||
      name.find("emissive") != std::string::npos) {
    return false; // 通常这些是sRGB空间
  }

  // 默认假设是sRGB
  return false;
}

void GltfConverter::convertKHRTextureTransform(
    const tinygltf::Value &extValue,
    GltfTexture *gltfTexture) {
  // 这个方法需要根据您的 KHRTextureTransform 类来实现
  // 由于这个扩展通常在 GltfTextureInfo 中使用，这里可能需要调整
  LOGI("Processing KHR_texture_transform extension");

  // 示例实现（需要根据实际结构调整）
  if (extValue.IsObject()) {
    const auto &obj = extValue.Get<tinygltf::Value::Object>();

    // 处理 offset
    if (obj.find("offset") != obj.end()) {
      // 处理偏移量
    }

    // 处理 scale
    if (obj.find("scale") != obj.end()) {
      // 处理缩放
    }

    // 处理 rotation
    if (obj.find("rotation") != obj.end()) {
      // 处理旋转
    }
  }
}

std::shared_ptr<GltfAsset>
GltfConverter::convertAsset(const tinygltf::Asset &asset) {
  auto gltfAsset = std::make_shared<GltfAsset>();
  gltfAsset->setVersion(asset.version);
  gltfAsset->setGenerator(asset.generator);
  gltfAsset->setCopyright(asset.copyright);
  gltfAsset->setMinVersion(asset.minVersion);
  return gltfAsset;
}

std::shared_ptr<GltfAccessor>
GltfConverter::convertAccessor(const tinygltf::Accessor &accessor) {
  auto gltfAccessor = std::make_shared<GltfAccessor>();

  gltfAccessor->setBufferView(accessor.bufferView);
  gltfAccessor->setByteOffset(accessor.byteOffset);
  gltfAccessor->setComponentType(accessor.componentType);
  gltfAccessor->setCount(accessor.count);
  std::optional<std::string> type;    ///< 访问器类型

  switch (accessor.type) {
    case TINYGLTF_TYPE_SCALAR:
      type = "SCALAR";
      break;
    case TINYGLTF_TYPE_VEC2:
      type = "VEC2";
      break;
    case TINYGLTF_TYPE_VEC3:
      type = "VEC3";
      break;
    case TINYGLTF_TYPE_VEC4:
      type = "VEC4";
      break;
    case TINYGLTF_TYPE_MAT2:
      type = "MAT2";
      break;
    case TINYGLTF_TYPE_MAT3:
      type = "MAT3";
      break;
    case TINYGLTF_TYPE_MAT4:
      type = "MAT4";
      break;
  }
  gltfAccessor->setType(type);
  gltfAccessor->setNormalized(accessor.normalized);
  gltfAccessor->setName(accessor.name);

  // 转换 min/max 值
  if (!accessor.minValues.empty()) {
    gltfAccessor->setMin(accessor.minValues);
  }
  if (!accessor.maxValues.empty()) {
    gltfAccessor->setMax(accessor.maxValues);
  }

  // 转换 sparse 数据
  if (accessor.sparse.isSparse) {
    GltfSparse sparse;
    sparse.count = accessor.sparse.count;

    sparse.indices.bufferView = accessor.sparse.indices.bufferView;
    sparse.indices.byteOffset = accessor.sparse.indices.byteOffset;
    sparse.indices.componentType = accessor.sparse.indices.componentType;

    sparse.values.bufferView = accessor.sparse.values.bufferView;
    sparse.values.byteOffset = accessor.sparse.values.byteOffset;
    gltfAccessor->setSparse(sparse);
  }

  return gltfAccessor;
}

std::shared_ptr<GltfBuffer>
GltfConverter::convertBuffer(const tinygltf::Buffer &buffer) {
  auto gltfBuffer = std::make_shared<GltfBuffer>();

  gltfBuffer->setBuffer(buffer.data);
  gltfBuffer->setByteLength(buffer.data.size());
  gltfBuffer->setUri(buffer.uri);
  gltfBuffer->setName(buffer.name);
  return gltfBuffer;
}

std::shared_ptr<GltfBufferView>
GltfConverter::convertBufferView(const tinygltf::BufferView &bufferView) {
  auto gltfBufferView = std::make_shared<GltfBufferView>();

  gltfBufferView->buffer = bufferView.buffer;
  gltfBufferView->byteOffset = bufferView.byteOffset;
  gltfBufferView->byteLength = bufferView.byteLength;
  gltfBufferView->byteStride = bufferView.byteStride;
  gltfBufferView->target = bufferView.target;
  gltfBufferView->name = bufferView.name;
  return gltfBufferView;
}

std::shared_ptr<GltfImage>
GltfConverter::convertImage(const tinygltf::Image &image) {
  auto gltfImage = std::make_shared<GltfImage>();

  gltfImage->setUri(image.uri);
  // 设置 MIME 类型
  ImageMimeType mimeType = convertMimeType(image.mimeType);
  gltfImage->setMimeType(mimeType);
  gltfImage->setBufferView(image.bufferView);
  gltfImage->setName(image.name);
  gltfImage->setType(GL_TEXTURE_2D);
  // 如果 tinygltf 已经解码了图像数据
  if (!image.image.empty() && image.width > 0 && image.height > 0) {
    auto imageData = createImageDataFromPixels(image);
    if (imageData) {
      gltfImage->setImageData(imageData);
      LOGI("Converted image with decoded pixels: %dx%d, %d channels",
           image.width, image.height, image.component);
    }
  } else {
    LOGI("Image has no decoded pixel data, will need to load: %s",
         image.uri.empty() ? "from bufferView" : image.uri.c_str());
  }

  return gltfImage;
}

std::shared_ptr<ImageData>
GltfConverter::createImageDataFromPixels(const tinygltf::Image &image) {
  if (image.image.empty() || image.width <= 0 || image.height <= 0) {
    LOGE("Invalid image data: empty or invalid dimensions");
    return nullptr;
  }

  // 创建像素数据副本
  std::vector<uint8_t> pixelData;

  // tinygltf 的图像数据格式转换
  int expectedSize = image.width * image.height * image.component;
  if (static_cast<int>(image.image.size()) < expectedSize) {
    LOGE("Image data size mismatch: expected %d, got %zu",
         expectedSize, image.image.size());
    return nullptr;
  }

  // 根据像素类型处理数据
  if (image.bits == 8) {
    // 8位数据，直接复制
    pixelData.assign(image.image.begin(),
                     image.image.begin() + expectedSize);
  } else if (image.bits == 16) {
    // 16位数据，需要转换为8位
    pixelData.reserve(expectedSize);
    const uint16_t
        *src16 = reinterpret_cast<const uint16_t *>(image.image.data());
    for (int i = 0; i < expectedSize; ++i) {
      // 简单的16位到8位转换（丢失精度）
      pixelData.push_back(static_cast<uint8_t>(src16[i] >> 8));
    }
    LOGW("Converting 16-bit image to 8-bit, precision loss may occur");
  } else if (image.bits == 32) {
    // 32位浮点数据，转换为8位
    pixelData.reserve(expectedSize);
    const float *srcFloat = reinterpret_cast<const float *>(image.image.data());
    for (int i = 0; i < expectedSize; ++i) {
      // 浮点到8位转换，假设浮点值在[0,1]范围
      float value = std::clamp(srcFloat[i], 0.0f, 1.0f);
      pixelData.push_back(static_cast<uint8_t>(value * 255.0f));
    }
    LOGW("Converting 32-bit float image to 8-bit");
  } else {
    LOGE("Unsupported image bit depth: %d", image.bits);
    return nullptr;
  }

  // 处理通道数转换（如果需要）
  int channels = image.component;
  if (channels < 1 || channels > 4) {
    LOGE("Unsupported channel count: %d", channels);
    return nullptr;
  }

  // 如果是单通道或双通道，可能需要转换为RGB/RGBA
  if (channels == 1) {
    // 灰度转换为RGB
    std::vector<uint8_t> rgbData;
    rgbData.reserve(image.width * image.height * 3);

    for (size_t i = 0; i < pixelData.size(); ++i) {
      uint8_t gray = pixelData[i];
      rgbData.push_back(gray);  // R
      rgbData.push_back(gray);  // G
      rgbData.push_back(gray);  // B
    }

    pixelData = std::move(rgbData);
    channels = 3;
  } else if (channels == 2) {
    // 灰度+Alpha 转换为RGBA
    std::vector<uint8_t> rgbaData;
    rgbaData.reserve(image.width * image.height * 4);

    for (size_t i = 0; i < pixelData.size(); i += 2) {
      uint8_t gray = pixelData[i];
      uint8_t alpha = pixelData[i + 1];
      rgbaData.push_back(gray);   // R
      rgbaData.push_back(gray);   // G
      rgbaData.push_back(gray);   // B
      rgbaData.push_back(alpha);  // A
    }

    pixelData = std::move(rgbaData);
    channels = 4;
  }

  // 创建 BasicImageData 对象
  return std::make_shared<BasicImageData>(image.width,
                                          image.height,
                                          channels,
                                          std::move(pixelData));
}

ImageMimeType GltfConverter::convertMimeType(const std::string &mimeType) {
  if (mimeType.empty()) {
    return ImageMimeType::UNKNOWN;
  }

  // 转换为小写进行比较
  std::string lowerMimeType = mimeType;
  std::transform(lowerMimeType.begin(), lowerMimeType.end(),
                 lowerMimeType.begin(), ::tolower);

  // 映射常见的MIME类型
  static const std::unordered_map<std::string, ImageMimeType> mimeTypeMap = {
      {"image/jpeg", ImageMimeType::JPEG},
      {"image/jpg", ImageMimeType::JPEG},
      {"image/png", ImageMimeType::PNG},
      {"image/webp", ImageMimeType::WEBP},
      {"image/bmp", ImageMimeType::UNKNOWN},      // BMP不在您的枚举中
      {"image/gif", ImageMimeType::UNKNOWN},      // GIF不在您的枚举中
      {"image/vnd.radiance", ImageMimeType::HDR},
      {"image/hdr", ImageMimeType::HDR},
      {"image/ktx2", ImageMimeType::KTX2},
      {"image/gltexture", ImageMimeType::GLTEXTURE}
  };

  auto it = mimeTypeMap.find(lowerMimeType);
  if (it != mimeTypeMap.end()) {
    return it->second;
  }

  LOGW("Unknown MIME type: %s", mimeType.c_str());
  return ImageMimeType::UNKNOWN;
}

std::shared_ptr<GltfSampler>
GltfConverter::convertSampler(const tinygltf::Sampler &sampler) {
  try {
    // 处理默认值（tinygltf 可能使用 -1 表示未设置）
    int magFilter = (sampler.magFilter != -1) ? sampler.magFilter : GL_LINEAR;
    int minFilter =
        (sampler.minFilter != -1) ? sampler.minFilter : GL_LINEAR_MIPMAP_LINEAR;
    int wrapS = (sampler.wrapS != -1) ? sampler.wrapS : GL_REPEAT;
    int wrapT = (sampler.wrapT != -1) ? sampler.wrapT : GL_REPEAT;

    // 验证参数有效性
    if (!GltfSampler::isValidMagFilter(magFilter)) {
      LOGW("Invalid magFilter %d, using default GL_LINEAR", magFilter);
      magFilter = GL_LINEAR;
    }

    if (!GltfSampler::isValidMinFilter(minFilter)) {
      LOGW("Invalid minFilter %d, using default GL_LINEAR_MIPMAP_LINEAR",
           minFilter);
      minFilter = GL_LINEAR_MIPMAP_LINEAR;
    }

    if (!GltfSampler::isValidWrapMode(wrapS)) {
      LOGW("Invalid wrapS %d, using default GL_REPEAT", wrapS);
      wrapS = GL_REPEAT;
    }

    if (!GltfSampler::isValidWrapMode(wrapT)) {
      LOGW("Invalid wrapT %d, using default GL_REPEAT", wrapT);
      wrapT = GL_REPEAT;
    }

    // 创建 GltfSampler 对象
    auto gltfSampler =
        std::make_shared<GltfSampler>(magFilter, minFilter, wrapS, wrapT);

    // 设置名称（如果存在）
    if (!sampler.name.empty()) {
      gltfSampler->setName(sampler.name);
    }

    // 最终验证
    if (!gltfSampler->validate()) {
      LOGE("Converted sampler failed validation");
      return nullptr;
    }

    LOGI("Converted sampler: mag=%d, min=%d, wrapS=%d, wrapT=%d, name='%s'",
         magFilter, minFilter, wrapS, wrapT, sampler.name.c_str());

    return gltfSampler;

  } catch (const std::exception &e) {
    LOGE("Exception converting sampler: %s", e.what());
    return nullptr;
  }
}

std::shared_ptr<GltfMesh> GltfConverter::convertMesh(const tinygltf::Mesh &mesh,
                                                     std::shared_ptr<Gltf> gltf,
                                                     Engine &gltfView) {
  auto gltfMesh = std::make_shared<GltfMesh>();

  gltfMesh->setName(mesh.name);
  for (const auto &primitive: mesh.primitives) {
    auto gltfPrimitive = std::make_shared<GltfPrimitive>();

    gltfPrimitive->setAttributes(primitive.attributes);

    gltfPrimitive->setIndices(primitive.indices);
    gltfPrimitive->setMaterial(primitive.material);
    gltfPrimitive->setMode(primitive.mode);
    // gltfPrimitive->setMaterial(mesh.)
    gltfPrimitive->setTargets(primitive.targets);
    //            convertExtensions(primitive.extensions, &gltfPrimitive);
//            convertExtras(primitive.extras, &gltfPrimitive);
    gltfPrimitive->initGl(gltf, gltfView.context);

    gltfMesh->addPrimitive(gltfPrimitive);
  }

  // 转换 weights
  gltfMesh->setWeights(mesh.weights);

  convertExtensions(mesh.extensions, gltfMesh.get());
  convertExtras(mesh.extras, gltfMesh.get());

  return gltfMesh;
}

std::shared_ptr<GltfNode>
GltfConverter::convertNode(const tinygltf::Node &node) {
  auto gltfNode = std::make_shared<GltfNode>();
  gltfNode->setName(node.name);
  gltfNode->setMesh(node.mesh);
  gltfNode->setCamera(node.camera);
  gltfNode->setSkin(node.skin);
  gltfNode->setChildren(node.children);
  gltfNode->setWeights(node.weights);
  gltfNode->setLight(node.light);
  // 转换变换矩阵 gltf 2.0 规范matrix 优先
  if (node.matrix.size() == 16) {
    glm::mat4 matrix = glm::make_mat4(node.matrix.data());
    // 设置到 optional
    gltfNode->setMatrix(matrix);
  } else {
    // 转换 TRS
    if (node.translation.size() == 3) {
      gltfNode->setTranslation(glm::vec3(node.translation[0],
                                         node.translation[1],
                                         node.translation[2]));
      gltfNode->setInitialTranslation(gltfNode->getTranslation());
      //  gltfNode->hasWorldTransform() = true;
    }

    if (node.rotation.size() == 4) {
      gltfNode->setRotation(
          glm::quat(node.rotation[3],
                    node.rotation[0],
                    node.rotation[1],
                    node.rotation[2]));
      gltfNode->setInitialRotation(gltfNode->getRotation());
      //gltfNode->hasRotation = true;
    }

    if (node.scale.size() == 3) {
      gltfNode->setScale(glm::vec3(node.scale[0],
                                   node.scale[1],
                                   node.scale[2]));
      gltfNode->setInitialScale(gltfNode->getScale());
      //gltfNode->hasScale = true;
    }
  }
  convertExtensions(node.extensions, gltfNode.get());
  convertExtras(node.extras, gltfNode.get());
  return gltfNode;
}

InterpolationMode
GltfConverter::convertInterpolationMode(const std::string &interpolation) {
  static const std::unordered_map<std::string, InterpolationMode>
      interpolationMap = {
      {"LINEAR", InterpolationMode::LINEAR},
      {"STEP", InterpolationMode::STEP},
      {"CUBICSPLINE", InterpolationMode::CUBICSPLINE},
      // 处理小写版本
      {"linear", InterpolationMode::LINEAR},
      {"step", InterpolationMode::STEP},
      {"cubicspline", InterpolationMode::CUBICSPLINE}
  };

  auto it = interpolationMap.find(interpolation);
  if (it != interpolationMap.end()) {
    return it->second;
  } else {
    LOGW("Unknown interpolation mode: %s, using LINEAR", interpolation.c_str());
    return InterpolationMode::LINEAR;
  }
}

std::shared_ptr<GltfAnimation>
GltfConverter::convertAnimation(const tinygltf::Animation &animation) {
  auto gltfAnimation = std::make_shared<GltfAnimation>();

  gltfAnimation->setName(animation.name);

  // 转换 samplers
  for (const auto &sampler: animation.samplers) {
    auto gltfSampler = std::make_shared<GltfAnimationSampler>();;
    gltfSampler->setInput(sampler.input);
    gltfSampler->setOutput(sampler.output);
    gltfSampler->setInterpolation(convertInterpolationMode(sampler.interpolation));

    convertExtensions(sampler.extensions, gltfSampler.get());
    convertExtras(sampler.extras, gltfSampler.get());
    gltfAnimation->addSampler(gltfSampler);
  }

  // 转换 channels
  for (const auto &channel: animation.channels) {
    auto gltfChannel = std::make_shared<GltfAnimationChannel>();
    gltfChannel->setSampler(channel.sampler);
    auto gltfSampler = std::make_shared<GltfAnimationSampler>();;
    auto target = std::make_shared<GltfAnimationTarget>();

    target->setNode(channel.target_node);
    target->setPath(convertTargetPath(channel.target_path));
    gltfChannel->setTarget(target);

//            convertExtensions(channel.extensions, &gltfChannel);
//            convertExtras(channel.extras, &gltfChannel);

    gltfAnimation->addChannel(gltfChannel);
  }

  convertExtensions(animation.extensions, gltfAnimation.get());
  convertExtras(animation.extras, gltfAnimation.get());

  return gltfAnimation;
}

InterpolationPath
GltfConverter::convertTargetPath(const std::string &targetPath) {
  static const std::unordered_map<std::string, InterpolationPath> pathMap = {
      // 标准路径
      {"translation", InterpolationPath::TRANSLATION},
      {"rotation", InterpolationPath::ROTATION},
      {"scale", InterpolationPath::SCALE},
      {"weights", InterpolationPath::WEIGHTS},
      {"pointer", InterpolationPath::POINTER},

      // 处理大写版本
      {"TRANSLATION", InterpolationPath::TRANSLATION},
      {"ROTATION", InterpolationPath::ROTATION},
      {"SCALE", InterpolationPath::SCALE},
      {"WEIGHTS", InterpolationPath::WEIGHTS},
      {"POINTER", InterpolationPath::POINTER}
  };

  auto it = pathMap.find(targetPath);
  if (it != pathMap.end()) {
    return it->second;
  } else {
    LOGW("Unknown target path: %s, using UNKNOWN", targetPath.c_str());
    return InterpolationPath::UNKNOWN;
  }
}

// 工具函数实现
void GltfConverter::convertExtensions(const tinygltf::ExtensionMap &extensions,
                                      GltfObject *object) {
  if (!object) return;

  for (const auto &ext: extensions) {
    // 这里需要根据您的 GltfObject 类实现来设置扩展数据
    // object->setExtension(ext.first, ext.second);
  }
}

void GltfConverter::convertExtras(const tinygltf::Value &extras,
                                  GltfObject *object) {
  if (!object) return;

  // 这里需要根据您的 GltfObject 类实现来设置额外数据
  // object->setExtras(extras);
}

} // namespace digitalhumans