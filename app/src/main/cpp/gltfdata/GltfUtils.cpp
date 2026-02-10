//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfUtils.h"

#include "gtc/matrix_transform.hpp"
#include "gtx/matrix_decompose.hpp"
#include <cmath>
#include <limits>
#include <algorithm>
#include "Gltf.h"
#include "../utils/LogUtils.h"
#include "GltfPrimitive.h"
#include "GltfScene.h"
#include "GltfMesh.h"

namespace digitalhumans {

void GltfUtils::getSceneExtents(std::shared_ptr<Gltf> gltf,
                                int sceneIndex,
                                glm::vec3 &outMin,
                                glm::vec3 &outMax) {
  if (!gltf) {
    LOGE("Invalid gltf object");
    initInvalidBoundingBox(outMin, outMax);
    return;
  }

  const auto &scenes = gltf->getScenes();
  if (sceneIndex < 0 || sceneIndex >= static_cast<int>(scenes.size())) {
    LOGE("Invalid scene index: %d", sceneIndex);
    initInvalidBoundingBox(outMin, outMax);
    return;
  }

  // 初始化为无效包围盒
  initInvalidBoundingBox(outMin, outMax);

  auto scene = scenes[sceneIndex];
  if (!scene) {
    LOGE("Scene is null");
    return;
  }

  const auto &nodes = gltf->getNodes();
  const auto &meshes = gltf->getMeshes();
  const auto &accessors = gltf->getAccessors();

  // 获取场景根节点
  std::vector<int> nodeIndices = scene->getNodes();

  // 深度优先遍历所有节点
  while (!nodeIndices.empty()) {
    int nodeIndex = nodeIndices.back();
    nodeIndices.pop_back();

    if (nodeIndex < 0 || nodeIndex >= static_cast<int>(nodes.size())) {
      continue;
    }

    auto node = nodes[nodeIndex];
    if (!node) {
      continue;
    }

    // 添加子节点到待处理列表
    const auto &children = node->getChildren();
    nodeIndices.insert(nodeIndices.end(), children.begin(), children.end());

    // 检查节点是否有网格
    if (!(node->getMesh().has_value() && node->getMesh().value() >= 0)) {
      continue;
    }

    std::optional<int> meshIndex = node->getMesh();
    if (meshIndex < 0 || meshIndex >= static_cast<int>(meshes.size())) {
      continue;
    }

    auto mesh = meshes[meshIndex.value()];
    if (!mesh) {
      continue;
    }

    const auto &primitives = mesh->getPrimitives();
    if (primitives.empty()) {
      continue;
    }

    // 获取节点的世界变换矩阵
    glm::mat4 worldTransform = node->getWorldTransform();

    // 处理网格的每个图元
    for (auto primitive: primitives) {
      if (!primitive) {
        continue;
      }

      // 查找POSITION属性
      const auto &attributes = primitive->getAttributes();
      auto positionIt = attributes.find("POSITION");
      if (positionIt == attributes.end()) {
        continue;
      }

      int accessorIndex = positionIt->second;
      if (accessorIndex < 0
          || accessorIndex >= static_cast<int>(accessors.size())) {
        continue;
      }

      auto accessor = accessors[accessorIndex];
      if (!accessor) {
        continue;
      }

      // 计算这个访问器的包围盒
      glm::vec3 assetMin, assetMax;
      getExtentsFromAccessor(accessor, worldTransform, assetMin, assetMax);

      // 更新全局包围盒
      if (isValidBoundingBox(assetMin, assetMax)) {
        if (!isValidBoundingBox(outMin, outMax)) {
          // 第一个有效包围盒
          outMin = assetMin;
          outMax = assetMax;
        } else {
          // 合并包围盒
          unionBoundingBoxes(outMin,
                             outMax,
                             assetMin,
                             assetMax,
                             outMin,
                             outMax);
        }
      }
    }
  }
}

void GltfUtils::getExtentsFromAccessor(std::shared_ptr<GltfAccessor> accessor,
                                       const glm::mat4 &worldTransform,
                                       glm::vec3 &outMin,
                                       glm::vec3 &outMax) {
  if (!accessor) {
    LOGE("Invalid accessor");
    initInvalidBoundingBox(outMin, outMax);
    return;
  }

  // 获取访问器的min和max值
  const auto &accessorMin = accessor->getMin();
  const auto &accessorMax = accessor->getMax();

  if (accessorMin.size() < 3 || accessorMax.size() < 3) {
    LOGE("Accessor min/max vectors have insufficient components");
    initInvalidBoundingBox(outMin, outMax);
    return;
  }

  glm::vec3 min(accessorMin[0], accessorMin[1], accessorMin[2]);
  glm::vec3 max(accessorMax[0], accessorMax[1], accessorMax[2]);

  // 如果访问器是标准化的，需要反量化
  if (accessor->isNormalized()) {
    LOGE("accessor is normalized");

    // 假设 accessor 有 dequantize 方法返回 glm::vec3
    /*   min = accessor->dequantize(accessor->getDataSize(),min,accessor->getComponentType());
       max = accessor->dequantize(max);*/

  }

  // 构造包围盒的8个顶点
  std::array<glm::vec3, 8> boxVertices = {{
                                              glm::vec3(min.x, min.y, min.z),
                                              glm::vec3(min.x, min.y, max.z),
                                              glm::vec3(min.x, max.y, min.z),
                                              glm::vec3(min.x, max.y, max.z),
                                              glm::vec3(max.x, min.y, min.z),
                                              glm::vec3(max.x, min.y, max.z),
                                              glm::vec3(max.x, max.y, min.z),
                                              glm::vec3(max.x, max.y, max.z)
                                          }};

  // 变换所有包围盒顶点
  for (auto &vertex: boxVertices) {
    vertex = transformPoint(vertex, worldTransform);
  }

  // 从变换后的顶点计算新的轴对齐包围盒
  computeAABBFromVertices(boxVertices, outMin, outMax);
}

void GltfUtils::getNodeExtents(std::shared_ptr<Gltf> gltf,
                               std::shared_ptr<GltfNode> node,
                               glm::vec3 &outMin,
                               glm::vec3 &outMax) {
  if (!gltf || !node) {
    initInvalidBoundingBox(outMin, outMax);
    return;
  }

  initInvalidBoundingBox(outMin, outMax);

  // 处理当前节点的网格
  if (node->getMesh().has_value() && node->getMesh().value() >= 0) {
    glm::vec3 meshMin, meshMax;
    getMeshExtents(gltf,
                   node->getMesh().value(),
                   node->getWorldTransform(),
                   meshMin,
                   meshMax);

    if (isValidBoundingBox(meshMin, meshMax)) {
      if (!isValidBoundingBox(outMin, outMax)) {
        outMin = meshMin;
        outMax = meshMax;
      } else {
        unionBoundingBoxes(outMin, outMax, meshMin, meshMax, outMin, outMax);
      }
    }
  }

  // 递归处理子节点
  const auto &nodes = gltf->getNodes();
  const auto &children = node->getChildren();

  for (int childIndex: children) {
    if (childIndex >= 0 && childIndex < static_cast<int>(nodes.size())) {
      auto childNode = nodes[childIndex];
      if (childNode) {
        glm::vec3 childMin, childMax;
        getNodeExtents(gltf, childNode, childMin, childMax);

        if (isValidBoundingBox(childMin, childMax)) {
          if (!isValidBoundingBox(outMin, outMax)) {
            outMin = childMin;
            outMax = childMax;
          } else {
            unionBoundingBoxes(outMin,
                               outMax,
                               childMin,
                               childMax,
                               outMin,
                               outMax);
          }
        }
      }
    }
  }
}

void GltfUtils::getMeshExtents(std::shared_ptr<Gltf> gltf,
                               int meshIndex,
                               const glm::mat4 &worldTransform,
                               glm::vec3 &outMin,
                               glm::vec3 &outMax) {
  if (!gltf) {
    initInvalidBoundingBox(outMin, outMax);
    return;
  }

  const auto &meshes = gltf->getMeshes();
  if (meshIndex < 0 || meshIndex >= static_cast<int>(meshes.size())) {
    initInvalidBoundingBox(outMin, outMax);
    return;
  }

  auto mesh = meshes[meshIndex];
  if (!mesh) {
    initInvalidBoundingBox(outMin, outMax);
    return;
  }

  initInvalidBoundingBox(outMin, outMax);

  const auto &primitives = mesh->getPrimitives();
  const auto &accessors = gltf->getAccessors();

  for (auto primitive: primitives) {
    if (!primitive) {
      continue;
    }

    const auto &attributes = primitive->getAttributes();
    auto positionIt = attributes.find("POSITION");
    if (positionIt == attributes.end()) {
      continue;
    }

    int accessorIndex = positionIt->second;
    if (accessorIndex < 0
        || accessorIndex >= static_cast<int>(accessors.size())) {
      continue;
    }

    auto accessor = accessors[accessorIndex];
    if (!accessor) {
      continue;
    }

    glm::vec3 primitiveMin, primitiveMax;
    getExtentsFromAccessor(accessor,
                           worldTransform,
                           primitiveMin,
                           primitiveMax);

    if (isValidBoundingBox(primitiveMin, primitiveMax)) {
      if (!isValidBoundingBox(outMin, outMax)) {
        outMin = primitiveMin;
        outMax = primitiveMax;
      } else {
        unionBoundingBoxes(outMin,
                           outMax,
                           primitiveMin,
                           primitiveMax,
                           outMin,
                           outMax);
      }
    }
  }
}

void GltfUtils::getSceneBoundingSphere(std::shared_ptr<Gltf> gltf,
                                       int sceneIndex,
                                       glm::vec3 &outCenter,
                                       float &outRadius) {
  glm::vec3 min, max;
  getSceneExtents(gltf, sceneIndex, min, max);

  if (!isValidBoundingBox(min, max)) {
    outCenter = glm::vec3(0.0f);
    outRadius = 0.0f;
    return;
  }

  outCenter = getBoundingBoxCenter(min, max);
  outRadius = calculateBoundingSphereRadius(outCenter, min, max);
}

glm::vec3 GltfUtils::transformVec3(const glm::vec3 &vector,
                                   const glm::mat4 &transform) {
  glm::vec4 result = transform * glm::vec4(vector, 0.0f); // w=0 for vectors
  return glm::vec3(result);
}

glm::vec3 GltfUtils::transformPoint(const glm::vec3 &point,
                                    const glm::mat4 &transform) {
  glm::vec4 result = transform * glm::vec4(point, 1.0f); // w=1 for points

  // 透视除法（如果需要）
  if (std::abs(result.w) > EPSILON && std::abs(result.w - 1.0f) > EPSILON) {
    return glm::vec3(result) / result.w;
  }

  return glm::vec3(result);
}

glm::vec3 GltfUtils::transformDirection(const glm::vec3 &direction,
                                        const glm::mat4 &transform) {
  // 使用转置逆矩阵的左上角3x3部分变换方向向量
  glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
  return normalMatrix * direction;
}

void GltfUtils::unionBoundingBoxes(const glm::vec3 &min1,
                                   const glm::vec3 &max1,
                                   const glm::vec3 &min2,
                                   const glm::vec3 &max2,
                                   glm::vec3 &outMin,
                                   glm::vec3 &outMax) {
  outMin = glm::min(min1, min2);
  outMax = glm::max(max1, max2);
}

bool GltfUtils::isValidBoundingBox(const glm::vec3 &min,
                                   const glm::vec3 &max) {
  return (min.x <= max.x && min.y <= max.y && min.z <= max.z) &&
      (!std::isinf(min.x) && !std::isinf(min.y) && !std::isinf(min.z)) &&
      (!std::isinf(max.x) && !std::isinf(max.y) && !std::isinf(max.z));
}

glm::vec3 GltfUtils::getBoundingBoxCenter(const glm::vec3 &min,
                                          const glm::vec3 &max) {
  return (min + max) * 0.5f;
}

glm::vec3 GltfUtils::getBoundingBoxSize(const glm::vec3 &min,
                                        const glm::vec3 &max) {
  return max - min;
}

float GltfUtils::getBoundingBoxDiagonal(const glm::vec3 &min,
                                        const glm::vec3 &max) {
  return glm::length(max - min);
}

void GltfUtils::expandBoundingBox(glm::vec3 &min,
                                  glm::vec3 &max,
                                  const glm::vec3 &point) {
  min = glm::min(min, point);
  max = glm::max(max, point);
}

bool GltfUtils::isPointInsideBoundingBox(const glm::vec3 &point,
                                         const glm::vec3 &min,
                                         const glm::vec3 &max) {
  return (point.x >= min.x && point.x <= max.x) &&
      (point.y >= min.y && point.y <= max.y) &&
      (point.z >= min.z && point.z <= max.z);
}

bool GltfUtils::boundingBoxesIntersect(const glm::vec3 &min1,
                                       const glm::vec3 &max1,
                                       const glm::vec3 &min2,
                                       const glm::vec3 &max2) {
  return (min1.x <= max2.x && max1.x >= min2.x) &&
      (min1.y <= max2.y && max1.y >= min2.y) &&
      (min1.z <= max2.z && max1.z >= min2.z);
}

float GltfUtils::calculateBoundingSphereRadius(const glm::vec3 &center,
                                               const glm::vec3 &min,
                                               const glm::vec3 &max) {
  // 计算包围盒8个顶点到球心的最大距离
  auto vertices = getBoundingBoxVertices(min, max);
  float maxDistance = 0.0f;

  for (const auto &vertex: vertices) {
    float distance = glm::length(vertex - center);
    maxDistance = std::max(maxDistance, distance);
  }

  return maxDistance;
}

std::array<glm::vec3, 8> GltfUtils::getBoundingBoxVertices(const glm::vec3 &min,
                                                           const glm::vec3 &max) {
  return {{
              glm::vec3(min.x, min.y, min.z),
              glm::vec3(min.x, min.y, max.z),
              glm::vec3(min.x, max.y, min.z),
              glm::vec3(min.x, max.y, max.z),
              glm::vec3(max.x, min.y, min.z),
              glm::vec3(max.x, min.y, max.z),
              glm::vec3(max.x, max.y, min.z),
              glm::vec3(max.x, max.y, max.z)
          }};
}

bool GltfUtils::decomposeMatrix(const glm::mat4 &matrix,
                                glm::vec3 &outTranslation,
                                glm::quat &outRotation,
                                glm::vec3 &outScale) {
  glm::vec3 skew;
  glm::vec4 perspective;
  return glm::decompose(matrix,
                        outScale,
                        outRotation,
                        outTranslation,
                        skew,
                        perspective);
}

glm::mat4 GltfUtils::composeMatrix(const glm::vec3 &translation,
                                   const glm::quat &rotation,
                                   const glm::vec3 &scale) {
  glm::mat4 T = glm::translate(glm::mat4(1.0f), translation);
  glm::mat4 R = glm::mat4_cast(rotation);
  glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

  return T * R * S;
}

// === 私有方法实现 ===

void GltfUtils::initInvalidBoundingBox(glm::vec3 &outMin,
                                       glm::vec3 &outMax) {
  outMin = glm::vec3(FLOAT_MAX);
  outMax = glm::vec3(FLOAT_MIN);
}

void
GltfUtils::computeAABBFromVertices(const std::array<glm::vec3, 8> &boxVertices,
                                   glm::vec3 &outMin,
                                   glm::vec3 &outMax) {
  if (boxVertices.empty()) {
    initInvalidBoundingBox(outMin, outMax);
    return;
  }

  // 初始化为第一个顶点
  outMin = boxVertices[0];
  outMax = boxVertices[0];

  // 遍历所有顶点找到最小和最大值
  for (const auto &vertex: boxVertices) {
    outMin = glm::min(outMin, vertex);
    outMax = glm::max(outMax, vertex);
  }
}

} // namespace digitalhumans