//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfPrimitive.h"
#include <android/log.h>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <set>
#include "../utils/LogUtils.h"
#include "Gltf.h"
#include "../utils/utils.h"
#include "GltfOpenGLContext.h"
#include "GltfSampler.h"
#include "GltfBufferView.h"
#include "GltfBuffer.h"
#include "ImageMimeTypes.h"
#include "GltfImage.h"


#define TINYGLTF_COMPONENT_TYPE_BYTE (5120)
#define TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE (5121)
#define TINYGLTF_COMPONENT_TYPE_SHORT (5122)
#define TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT (5123)
#define TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT (5125)
#define TINYGLTF_COMPONENT_TYPE_FLOAT (5126)

namespace digitalhumans {

GltfPrimitive::GltfPrimitive()
    : GltfObject(), attributes(), targets(), indices(std::nullopt),
      material(std::nullopt), mode(GL_TRIANGLES), glAttributes(),
      morphTargetTextureInfo(nullptr), defines(), skip(false),
      m_hasWeights(false), m_hasJoints(false), m_hasNormals(false),
      m_hasTangents(false), m_hasTexcoord(false), m_hasColor(false),
      centroid({0.0f, 0.0f, 0.0f}), mappings() {
}


void GltfPrimitive::initGl(std::shared_ptr<Gltf> gltf,
                           std::shared_ptr<GltfOpenGLContext> openGlContext) {
  if (!gltf || !openGlContext) {
    return;
  }

  // 使用默认glTF材质
  if (!material.has_value()) {
    const auto &materials = gltf->getMaterials();
    if (!materials.empty()) {
      material = static_cast<int>(materials.size() - 1);
    }
  }

  GLint maxAttributes = 0;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttributes);

  // 处理扩展（如Draco压缩）
  handleDracoCompression(gltf);

  // 生成切线（如果需要）
//        if (attributes.find("TANGENT") == attributes.end() &&
//            attributes.find("NORMAL") != attributes.end() &&
//            attributes.find("TEXCOORD_0") != attributes.end() &&
//            mode > 3) {
//
//            LOGI("Generating tangents using the MikkTSpace algorithm.");
//            unweld(gltf);
//            generateTangents(gltf);
//        }

  // 处理顶点属性
  glAttributes.clear();
  defines.clear();

  for (const auto &[attributeName, accessorIndex]: attributes) {
    if (static_cast<int>(glAttributes.size()) >= maxAttributes) {
      LOGE("Too many vertex attributes for this primitive, skipping %s",
           attributeName.c_str());
      break;
    }

    bool knownAttribute = true;

    // 检查属性类型并设置标志
    if (attributeName == "POSITION") {
      skip = false;
    } else if (attributeName == "NORMAL") {
      m_hasNormals = true;
    } else if (attributeName == "TANGENT") {
      m_hasTangents = true;
    } else if (attributeName == "TEXCOORD_0" || attributeName == "TEXCOORD_1") {
      m_hasTexcoord = true;
    } else if (attributeName == "COLOR_0") {
      m_hasColor = true;
    } else if (attributeName == "JOINTS_0" || attributeName == "JOINTS_1") {
      m_hasJoints = true;
    } else if (attributeName == "WEIGHTS_0" || attributeName == "WEIGHTS_1") {
      m_hasWeights = true;
    } else {
      knownAttribute = false;
      LOGI("Unknown attribute: %s", attributeName.c_str());
    }

    if (knownAttribute) {
      // 创建OpenGL属性名称
      std::string glAttributeName = "a_" + attributeName;
      std::transform(glAttributeName.begin(), glAttributeName.end(),
                     glAttributeName.begin(), ::tolower);

      GLAttribute glAttr;
      glAttr.attribute = attributeName;
      glAttr.name = glAttributeName;
      glAttr.accessor = accessorIndex;
      glAttributes.push_back(glAttr);

      // 添加宏定义
      if (accessorIndex >= 0
          && accessorIndex < static_cast<int>(gltf->getAccessors().size())) {
        auto accessor = gltf->getAccessors()[accessorIndex];
        if (accessor) {
          std::string define =
              "HAS_" + attributeName + "_" + accessor->getType().value() + " 1";
          defines.push_back(define);
        }
      }
    }
  }

  // 处理变形目标
  processMorphTargets(gltf, openGlContext);

  // 计算重心
  computeCentroid(gltf);
}

void GltfPrimitive::handleDracoCompression(std::shared_ptr<Gltf> gltf) {
  // TODO: 实现Draco压缩处理
  // 这需要集成Draco解码库，在Android平台上可能需要特殊处理
  LOGW("Draco compression handling not implemented yet");
}

void GltfPrimitive::processMorphTargets(std::shared_ptr<Gltf> gltf,
                                        std::shared_ptr<GltfOpenGLContext> webGlContext) {
  if (targets.empty()) {
    return;
  }

  GLint maxTextureSize = 0;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
  const int max2DTextureSize = maxTextureSize * maxTextureSize;

  GLint maxArrayTextureLayers = 0;
  glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxArrayTextureLayers);

  std::set<std::string> attributeSet;
  for (const auto &target: targets) {
    for (const auto &[attrName, _]: target) {
      attributeSet.insert(attrName);
    }
  }

  std::vector<std::string>
      morphAttributes(attributeSet.begin(), attributeSet.end());

  if (morphAttributes.empty()) {
    return;
  }

  // 获取顶点数量
  int vertexCount = 0;
  if (!morphAttributes.empty()) {
    auto firstAttrIt = attributes.find(morphAttributes[0]);
    if (firstAttrIt != attributes.end()) {
      const int accessorIndex = firstAttrIt->second;
      if (accessorIndex >= 0
          && accessorIndex < static_cast<int>(gltf->getAccessors().size())) {
        auto accessor = gltf->getAccessors()[accessorIndex];
        if (accessor) {
          vertexCount = accessor->getCount().value();
        }
      }
    }
  }

  if (vertexCount == 0) {
    return;
  }

  defines.push_back("NUM_VERTICES " + std::to_string(vertexCount));

  // 检查纹理大小限制
  int targetCount = static_cast<int>(targets.size());
  if (targetCount * static_cast<int>(morphAttributes.size())
      > maxArrayTextureLayers) {
    targetCount =
        maxArrayTextureLayers / static_cast<int>(morphAttributes.size());
    LOGW("Morph targets exceed texture size limit. Only %d of %zu are used.",
         targetCount, targets.size());
  }

  // 设置属性偏移
  std::unordered_map<std::string, int> attributeOffsets;
  int attributeOffset = 0;
  for (const std::string &attribute: morphAttributes) {
    defines.push_back("HAS_MORPH_TARGET_" + attribute + " 1");
    defines.push_back("MORPH_TARGET_" + attribute + "_OFFSET "
                          + std::to_string(attributeOffset));
    attributeOffsets[attribute] = attributeOffset;
    attributeOffset += targetCount;
  }
  defines.push_back("HAS_MORPH_TARGETS 1");

  if (vertexCount <= max2DTextureSize) {
    // 创建变形目标纹理
    const int width = static_cast<int>(std::ceil(std::sqrt(vertexCount)));
    const int singleTextureSize = width * width * 4;
    const int totalTextureSize = singleTextureSize * targetCount
        * static_cast<int>(morphAttributes.size());

    std::vector<float> morphTargetTextureArray(totalTextureSize, 0.0f);

    // 组装纹理数据
    for (int i = 0; i < targetCount; ++i) {
      const auto &target = targets[i];

      for (const auto &[attributeName, offsetRef]: attributeOffsets) {
        auto targetAttrIt = target.find(attributeName);
        if (targetAttrIt != target.end()) {
          const int accessorIndex = targetAttrIt->second;
          if (accessorIndex >= 0 && accessorIndex
              < static_cast<int>(gltf->getAccessors().size())) {
            auto accessor = gltf->getAccessors()[accessorIndex];
            if (accessor && accessor->getComponentType() == GL_FLOAT) {
              auto data = accessor->getNormalizedDeinterlacedView(*gltf);
              if (data.size() > 0) {
                const int offset = offsetRef * singleTextureSize;
                const int componentCount = accessor->getComponentCount();

                // 将数据复制到纹理数组中，添加必要的填充
                int paddingOffset = 0;
                int accessorOffset = 0;
                for (int j = 0; j < accessor->getCount(); ++j) {
                  const int copyCount = std::min(componentCount, 4);
                  std::copy(data.begin() + accessorOffset,
                            data.begin() + accessorOffset + copyCount,
                            morphTargetTextureArray.begin() + offset
                                + paddingOffset);
                  paddingOffset += 4;
                  accessorOffset += componentCount;
                }
              }
            }
          }
        }
      }
    }

    // 创建WebGL纹理
    GLenum texture = webGlContext->createTexture();
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

    // 上传纹理数据
    glTexImage3D(
        GL_TEXTURE_2D_ARRAY,
        0,                                              // level
        GL_RGBA32F,                                     // internal format
        width,                                          // width
        width,                                          // height
        targetCount * static_cast<int>(morphAttributes.size()),  // depth
        0,                                              // border
        GL_RGBA,                                        // format
        GL_FLOAT,                                       // type
        morphTargetTextureArray.data()                  // data
    );

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // 创建gltf纹理资源
    auto morphTargetImage = std::make_shared<GltfImage>(
        "",                    // uri
        GL_TEXTURE_2D_ARRAY,            // type
        0,                              // mip level
        std::nullopt,                   // buffer view
        "",                   // name
        ImageMimeType::GLTEXTURE,     // mime type
        texture                         // webgl texture
    );
    //TODO
    gltf->addImage(morphTargetImage);

    auto sampler = std::make_shared<GltfSampler>(
        GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE
    );
    gltf->addSampler(sampler);

    auto morphTargetTexture = std::make_shared<GltfTexture>(
        static_cast<int>(gltf->getSamplers().size() - 1),
        static_cast<int>(gltf->getImages().size() - 1),
        GL_TEXTURE_2D_ARRAY
    );
    morphTargetTexture->setInitialized(true);
    gltf->addTexture(morphTargetTexture);

    morphTargetTextureInfo = std::make_shared<GltfTextureInfo>(
        static_cast<int>(gltf->getTextures().size() - 1), 0, true
    );
    morphTargetTextureInfo->setSamplerName("u_MorphTargetsSampler");
    morphTargetTextureInfo->setGenerateMips(false);
  } else {
    LOGW("Mesh of Morph targets too big. Cannot apply morphing.");
  }
}

void GltfPrimitive::computeCentroid(std::shared_ptr<Gltf> gltf) {
  // 基础空指针检查
  if (!gltf) {
    LOGE(" Gltf对象为空");
    return;
  }

  // 查找POSITION属性
  auto positionIt = attributes.find("POSITION");
  if (positionIt == attributes.end()) {
    LOGW("未找到POSITION属性");
    return;
  }

  const int positionAccessorIndex = positionIt->second;
  if (positionAccessorIndex < 0 || positionAccessorIndex
      >= static_cast<int>(gltf->getAccessors().size())) {
    LOGE("POSITION访问器索引无效: %d", positionAccessorIndex);
    return;
  }

  auto positionsAccessor = gltf->getAccessors()[positionAccessorIndex];
  if (!positionsAccessor) {
    LOGE("POSITION访问器为空");
    return;
  }

  // 获取位置数据
  auto positions = positionsAccessor->getNormalizedTypedView(*gltf);
  if (positions.empty()) {
    LOGW("位置数据为空");
    return;
  }

  // 验证位置数据格式
  auto positionType = positionsAccessor->getType();
  if (!positionType.has_value() || positionType.value() != "VEC3") {
    LOGE("POSITION不是VEC3类型: %s",
         positionType.has_value() ? positionType.value().c_str() : "unknown");
    return;
  }

  const int vertexCount = positionsAccessor->getCount().value_or(0);
  if (vertexCount <= 0) {
    LOGW("顶点数量无效: %d", vertexCount);
    return;
  }

  // 验证位置数据大小
  const size_t expectedPositionSize = vertexCount * 3;
  if (positions.size() < expectedPositionSize) {
    LOGE("位置数据大小不匹配，期望: %zu，实际: %zu",
         expectedPositionSize,
         positions.size());
    return;
  }

  std::array<float, 3> acc = {0.0f, 0.0f, 0.0f};
  int count = 0;

  if (indices.has_value()) {
    // 图元有索引的情况
    const int indicesAccessorIndex = indices.value();

    if (indicesAccessorIndex < 0 || indicesAccessorIndex
        >= static_cast<int>(gltf->getAccessors().size())) {
      LOGE("索引访问器索引无效: %d", indicesAccessorIndex);
      return;
    }

    auto indicesAccessor = gltf->getAccessors()[indicesAccessorIndex];
    if (!indicesAccessor) {
      LOGE("索引访问器为空");
      return;
    }

    auto indicesData = indicesAccessor->getTypedView(*gltf);
    if (indicesData.second <= 0 || indicesData.first == nullptr) {
      LOGE("索引数据无效");
      return;
    }

    const int indexCount = indicesAccessor->getCount().value_or(0);
    if (indexCount <= 0) {
      LOGE("索引数量无效: %d", indexCount);
      return;
    }

    auto componentType = indicesAccessor->getComponentType();
    if (!componentType.has_value()) {
      LOGE("索引组件类型未定义");
      return;
    }


    // 处理索引数据
    for (int i = 0; i < indexCount; ++i) {
      uint32_t index = 0;
      bool validIndex = false;

      // 根据不同的组件类型读取索引
      switch (componentType.value()) {
        case GL_UNSIGNED_BYTE: {
          if (i < static_cast<int>(indicesData.second / sizeof(uint8_t))) {
            const uint8_t
                *byteIndices = static_cast<const uint8_t *>(indicesData.first);
            index = byteIndices[i];
            validIndex = true;
          }
          break;
        }
        case GL_UNSIGNED_SHORT: {
          if (i < static_cast<int>(indicesData.second / sizeof(uint16_t))) {
            const uint16_t *shortIndices =
                static_cast<const uint16_t *>(indicesData.first);
            index = shortIndices[i];
            validIndex = true;
          }
          break;
        }
        case GL_UNSIGNED_INT: {
          if (i < static_cast<int>(indicesData.second / sizeof(uint32_t))) {
            const uint32_t
                *intIndices = static_cast<const uint32_t *>(indicesData.first);
            index = intIndices[i];
            validIndex = true;
          }
          break;
        }
        default:
          LOGE("不支持的索引组件类型: %d", componentType.value());
          continue;
      }

      if (!validIndex) {
        LOGE("无法读取索引 %d", i);
        continue;
      }

      // 验证索引范围
      if (index >= static_cast<uint32_t>(vertexCount)) {
        LOGE("索引越界: %u >= %d", index, vertexCount);
        continue;
      }

      // 计算位置数据偏移
      const size_t offset = 3 * index;

      // 双重检查边界
      if (offset + 2 >= positions.size()) {
        LOGE("位置数据偏移越界: %zu + 2 >= %zu", offset, positions.size());
        continue;
      }

      // 累加位置数据
      acc[0] += positions[offset];
      acc[1] += positions[offset + 1];
      acc[2] += positions[offset + 2];
      count++;
    }

  } else {
    for (int i = 0; i < vertexCount; ++i) {
      const size_t offset = 3 * i;

      // 检查边界
      if (offset + 2 >= positions.size()) {
        LOGE("无索引模式位置数据偏移越界: %zu + 2 >= %zu",
             offset,
             positions.size());
        break;
      }

      acc[0] += positions[offset];
      acc[1] += positions[offset + 1];
      acc[2] += positions[offset + 2];
      count++;
    }
  }

  // 计算重心
  if (count > 0) {
    centroid[0] = acc[0] / count;
    centroid[1] = acc[1] / count;
    centroid[2] = acc[2] / count;

  } else {
    centroid = {0.0f, 0.0f, 0.0f};
  }
}

void GltfPrimitive::generateTangents(std::shared_ptr<Gltf> gltf) {
  auto normalIt = attributes.find("NORMAL");
  auto texCoordIt = attributes.find("TEXCOORD_0");
  auto positionIt = attributes.find("POSITION");

  if (normalIt == attributes.end() || texCoordIt == attributes.end()
      || positionIt == attributes.end()) {
    return;
  }

  // 获取访问器
  auto positionsAccessor = gltf->getAccessors()[positionIt->second];
  auto normalsAccessor = gltf->getAccessors()[normalIt->second];
  auto texCoordsAccessor = gltf->getAccessors()[texCoordIt->second];

  if (!positionsAccessor || !normalsAccessor || !texCoordsAccessor) {
    return;
  }

  // 获取数据视图
  auto positions = positionsAccessor->getTypedView(*gltf);
  auto normals = normalsAccessor->getTypedView(*gltf);
  auto texCoords = texCoordsAccessor->getTypedView(*gltf);

  // 修改检查逻辑
  if (positions.first == nullptr || positions.second == 0 ||
      normals.first == nullptr || normals.second == 0 ||
      texCoords.first == nullptr || texCoords.second == 0) {
    return;
  }

  const int vertexCount = positionsAccessor->getCount().value();

  // 这里应该调用MikkTSpace算法生成切线
  // 为简化，我们创建一个简单的切线数据
  std::vector<float> tangents(vertexCount * 4, 0.0f);

  // TODO: 实现真正的MikkTSpace切线生成算法
  // 这里只是一个占位符实现
  for (int i = 0; i < vertexCount; ++i) {
    tangents[i * 4 + 0] = 1.0f;  // x
    tangents[i * 4 + 1] = 0.0f;  // y
    tangents[i * 4 + 2] = 0.0f;  // z
    tangents[i * 4 + 3] = 1.0f;  // w (handedness)
  }

  // 创建切线的访问器
  const int tangentAccessorIndex = createBufferAndAccessor(
      gltf,
      tangents.data(),
      tangents.size() * sizeof(float),
      GL_ARRAY_BUFFER,
      vertexCount,
      "VEC4",
      GL_FLOAT
  );

  // 更新图元属性
  attributes["TANGENT"] = tangentAccessorIndex;
  m_hasTangents = true;
}

void GltfPrimitive::unweld(std::shared_ptr<Gltf> gltf) {
  // 展开是幂等操作
  if (!indices.has_value()) {
    return;
  }

  const int indicesAccessorIndex = indices.value();
  if (indicesAccessorIndex < 0 || indicesAccessorIndex
      >= static_cast<int>(gltf->getAccessors().size())) {
    return;
  }

  auto indicesAccessor = gltf->getAccessors()[indicesAccessorIndex];
  if (!indicesAccessor) {
    return;
  }

  auto indicesData = indicesAccessor->getTypedView(*gltf);
  if (indicesData.first == nullptr || indicesData.second == 0) {
    return;
  }
  auto indices = getIndicesAsUint32(indicesAccessor, *gltf);

  for (auto &[attributeName, accessorIndex]: attributes) {
    if (accessorIndex >= 0
        && accessorIndex < static_cast<int>(gltf->getAccessors().size())) {
      auto accessor = gltf->getAccessors()[accessorIndex];
      if (accessor) {
        attributes[attributeName] = unweldAccessor(gltf, accessor, indices);
      }
    }
  }

  // 展开变形目标
  for (auto &target: targets) {
    for (auto &[attributeName, accessorIndex]: target) {
      if (accessorIndex >= 0
          && accessorIndex < static_cast<int>(gltf->getAccessors().size())) {
        auto accessor = gltf->getAccessors()[accessorIndex];
        if (accessor) {
          target[attributeName] = unweldAccessor(gltf, accessor, indices);
        }
      }
    }
  }

}

// 在适当的地方定义这个辅助函数
std::vector<uint32_t>
GltfPrimitive::getIndicesAsUint32(std::shared_ptr<GltfAccessor> accessor,
                                  const Gltf &gltf) {
  auto rawData = accessor->getTypedView(gltf);
  if (rawData.first == nullptr || rawData.second == 0) {
    return {};
  }

  auto componentType = accessor->getComponentType();
  std::vector<uint32_t> result;

  switch (componentType.value()) {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
      const uint8_t *indices = static_cast<const uint8_t *>(rawData.first);
      size_t count = rawData.second / sizeof(uint8_t);
      result.reserve(count);
      for (size_t i = 0; i < count; ++i) {
        result.push_back(static_cast<uint32_t>(indices[i]));
      }
      break;
    }
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
      const uint16_t *indices = static_cast<const uint16_t *>(rawData.first);
      size_t count = rawData.second / sizeof(uint16_t);
      result.reserve(count);
      for (size_t i = 0; i < count; ++i) {
        result.push_back(static_cast<uint32_t>(indices[i]));
      }
      break;
    }
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
      const uint32_t *indices = static_cast<const uint32_t *>(rawData.first);
      size_t count = rawData.second / sizeof(uint32_t);
      result.assign(indices, indices + count);
      break;
    }
    default:
      break;
  }

  return result;
}

int GltfPrimitive::unweldAccessor(std::shared_ptr<Gltf> gltf,
                                  std::shared_ptr<GltfAccessor> accessor,
                                  const std::vector<uint32_t> &indicesData) {
  if (!accessor) {
    return -1;
  }

  const int componentCount = accessor->getComponentCount();
  auto weldedData = accessor->getDeinterlacedView(*gltf);
  if (weldedData.second <= 0) {
    return -1;
  }

  // 创建展开后的数据
  std::vector<float> unweldedData(indicesData.size() * componentCount);

// 获取weldedData并转换为float指针
  const float *weldedFloatData = static_cast<const float *>(weldedData.first);
  size_t weldedFloatCount = weldedData.second / sizeof(float); // 元素数量而不是字节数量

// 应用索引映射
  for (size_t i = 0; i < indicesData.size(); ++i) {
    const uint32_t index = indicesData[i];
    const size_t srcOffset = index * componentCount;
    const size_t dstOffset = i * componentCount;

    // 检查边界（现在是以float元素为单位）
    if (srcOffset + componentCount <= weldedFloatCount) {
      // 直接拷贝float数据
      std::copy(weldedFloatData + srcOffset,
                weldedFloatData + srcOffset + componentCount,
                unweldedData.begin() + dstOffset);
    }
  }

  // 创建新的访问器
  return createBufferAndAccessor(
      gltf,
      unweldedData.data(),
      unweldedData.size() * sizeof(float),
      GL_ARRAY_BUFFER,
      static_cast<int>(indicesData.size()),
      accessor->getType().value(),
      accessor->getComponentType().value()
  );
}

int GltfPrimitive::createBufferAndAccessor(std::shared_ptr<Gltf> gltf,
                                           const void *data,
                                           size_t byteLength,
                                           GLenum target,
                                           int count,
                                           const std::string &type,
                                           GLenum componentType) {
  if (!gltf || !data || byteLength == 0) {
    return -1;
  }

  // 创建缓冲区
  auto buffer = std::make_shared<GltfBuffer>();
  buffer->setByteLength(byteLength);

  // 复制数据到缓冲区
  std::vector<uint8_t> bufferData(byteLength);
  std::memcpy(bufferData.data(), data, byteLength);
  buffer->setBuffer(std::move(bufferData));

  gltf->addBuffer(buffer);

  // 创建缓冲区视图
  auto bufferView = std::make_shared<GltfBufferView>();
  bufferView->setBuffer(static_cast<int>(gltf->getBuffers().size() - 1));
  bufferView->setByteLength(byteLength);
  bufferView->setTarget(target);
  gltf->addBufferView(bufferView);

  // 创建访问器
  auto accessor = std::make_shared<GltfAccessor>();
  accessor->setBufferView(static_cast<int>(gltf->getBufferViews().size() - 1));
  accessor->setByteOffset(0);
  accessor->setCount(count);
  accessor->setType(type);
  accessor->setComponentType(componentType);
  gltf->addAccessor(accessor);

  return static_cast<int>(gltf->getAccessors().size() - 1);
}

} // namespace digitalhumans