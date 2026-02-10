//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfNode.h"
#include "Gltf.h"
#include "../utils/utils.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "gtx/matrix_decompose.hpp"
#include "GltfMesh.h"

namespace digitalhumans {

GltfNode::GltfNode()
    : GltfObject(), camera(std::nullopt), children(), matrix(std::nullopt),
      rotation(1.0f, 0.0f, 0.0f, 0.0f)  // 单位四元数
    , scale(1.0f, 1.0f, 1.0f)            // 单位缩放
    , translation(0.0f, 0.0f, 0.0f)      // 零平移
    , name(""), mesh(std::nullopt), skin(std::nullopt), weights(),
      worldTransform(1.0f)               // 单位矩阵
    , worldQuaternion(1.0f, 0.0f, 0.0f, 0.0f), inverseWorldTransform(1.0f),
      normalMatrix(1.0f),
      light(std::nullopt), instanceMatrices(), instanceWorldTransforms() {
}


std::vector<double> GltfNode::getWeights(std::shared_ptr<Gltf> gltf) {
  if (!weights.empty()) {
    return weights;
  } else if (mesh.has_value()) {
    const auto &meshes = gltf->getMeshes();
    if (mesh.value() >= 0 && mesh.value() < static_cast<int>(meshes.size())) {
      return meshes[mesh.value()]->getWeights();
    }
  }
  return {};
}

void GltfNode::applyMatrix(const glm::mat4 &matrixData) {
  // 转换为GLM矩阵
  matrix = matrixData;  // 假设你有matrix成员变量
  // 分解矩阵
  glm::vec3 tempScale, tempTranslation, tempSkew;
  glm::quat tempRotation;
  glm::vec4 tempPerspective;

  if (glm::decompose(matrix.value(),
                     tempScale,
                     tempRotation,
                     tempTranslation,
                     tempSkew,
                     tempPerspective)) {
    scale = tempScale;
    rotation = tempRotation;
    translation = tempTranslation;

    // 确保四元数归一化
    rotation = glm::normalize(rotation);
  }
}

glm::mat4 GltfNode::getLocalTransform() {
  glm::mat4 result(1.0f);

  // 应用变换：T * R * S
  result = glm::translate(result, translation);
  result *= glm::mat4_cast(rotation);
  result = glm::scale(result, scale);

  return result;
}

void GltfNode::setInitialRotation(const glm::quat &initialRotation) {
  GltfNode::initialRotation = initialRotation;
}

void GltfNode::setInitialScale(const glm::vec3 &initialScale) {
  GltfNode::initialScale = initialScale;
}

void GltfNode::setInitialTranslation(const glm::vec3 &initialTranslation) {
  GltfNode::initialTranslation = initialTranslation;
}

void GltfNode::setInitialWeights(const std::vector<double> &initialWeights) {
  GltfNode::initialWeights = initialWeights;
}

} // namespace digitalhumans