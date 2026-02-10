//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfScene.h"
#include "Gltf.h"

#include "gtc/matrix_transform.hpp"
#include "gtc/matrix_inverse.hpp"
#include <algorithm>
#include <stack>
#include <utility>

namespace digitalhumans {

GltfScene::GltfScene(const std::vector<int> &nodes, std::string name)
    : GltfObject(), nodes(nodes), name(std::move(name)),
      imageBasedLight(nullptr) {
}


void GltfScene::applyTransformHierarchy(std::shared_ptr<Gltf> gltf,
                                        const glm::mat4 &rootTransform) {
  // 第一部分：应用变换矩阵层次结构
  for (int nodeIndex: nodes) {
    if (nodeIndex >= 0 && nodeIndex < static_cast<int>(gltf->nodes.size())) {
      auto node = gltf->nodes[nodeIndex];
      applyTransform(gltf, node, rootTransform);
    }
  }

  // 第二部分：应用旋转四元数层次结构
  glm::quat identityQuat(1.0f, 0.0f, 0.0f, 0.0f);
  for (int nodeIndex: nodes) {
    if (nodeIndex >= 0 && nodeIndex < static_cast<int>(gltf->nodes.size())) {
      auto node = gltf->nodes[nodeIndex];
      applyWorldRotation(gltf, node, identityQuat);
    }
  }
}

std::vector<std::shared_ptr<GltfNode>>
GltfScene::gatherNodes(const std::shared_ptr<Gltf> &gltf) {
  std::vector<std::shared_ptr<GltfNode>> collectedNodes;

  for (int nodeIndex: nodes) {
    gatherNode(gltf, nodeIndex, collectedNodes);
  }

  return collectedNodes;
}

bool GltfScene::includesNode(std::shared_ptr<Gltf> gltf, int nodeIndex) {
  std::stack<int> children;

  // 初始化堆栈
  for (int rootNode: nodes) {
    children.push(rootNode);
  }

  while (!children.empty()) {
    int childIndex = children.top();
    children.pop();

    if (childIndex == nodeIndex) {
      return true;
    }

    // 添加子节点到堆栈
    if (childIndex >= 0 && childIndex < static_cast<int>(gltf->nodes.size())) {
      auto node = gltf->nodes[childIndex];
      const auto &nodeChildren = node->getChildren();
      for (int grandChild: nodeChildren) {
        children.push(grandChild);
      }
    }
  }

  return false;
}


void GltfScene::applyTransform(std::shared_ptr<Gltf> gltf,
                               std::shared_ptr<GltfNode> node,
                               const glm::mat4 &parentTransform) {
  // 计算世界变换矩阵
  glm::mat4 localTransform = node->getLocalTransform();
  glm::mat4 worldTransform = parentTransform * localTransform;
  node->setWorldTransform(worldTransform);

  // 计算逆世界变换
  glm::mat4 inverseWorldTransform = glm::inverse(worldTransform);
  node->setInverseWorldTransform(inverseWorldTransform);

  // 计算法线矩阵
  glm::mat3 normalMatrix = glm::transpose(glm::mat3(inverseWorldTransform));
  node->setNormalMatrix(normalMatrix);

  // 处理实例化矩阵
  const auto &instanceMatrices = node->getInstanceMatrices();
  if (!instanceMatrices.empty()) {
    std::vector<glm::mat4> instanceWorldTransforms;
    instanceWorldTransforms.reserve(instanceMatrices.size());

    for (const auto &instanceTransform: instanceMatrices) {
      glm::mat4 instanceWorldTransform = worldTransform * instanceTransform;
      instanceWorldTransforms.push_back(instanceWorldTransform);
    }
    node->setInstanceWorldTransforms(instanceWorldTransforms);
  }

  // 递归处理子节点
  const auto &children = node->getChildren();
  for (int childIndex: children) {
    if (childIndex >= 0 && childIndex < static_cast<int>(gltf->nodes.size())) {
      auto childNode = gltf->nodes[childIndex];
      applyTransform(gltf, childNode, worldTransform);
    }
  }
}

void GltfScene::applyWorldRotation(std::shared_ptr<Gltf> gltf,
                                   std::shared_ptr<GltfNode> node,
                                   const glm::quat &parentRotation) {
  // 计算世界四元数
  glm::quat nodeRotation = node->getRotation();
  glm::quat worldQuaternion = parentRotation * nodeRotation;
  node->setWorldQuaternion(worldQuaternion);

  // 递归处理子节点
  const auto &children = node->getChildren();
  for (int childIndex: children) {
    if (childIndex >= 0 && childIndex < static_cast<int>(gltf->nodes.size())) {
      auto childNode = gltf->nodes[childIndex];
      applyWorldRotation(gltf, childNode, worldQuaternion);
    }
  }
}

void GltfScene::gatherNode(std::shared_ptr<Gltf> gltf,
                           int nodeIndex,
                           std::vector<std::shared_ptr<GltfNode>> &collectedNodes) {
  if (nodeIndex >= 0 && nodeIndex < static_cast<int>(gltf->nodes.size())) {
    auto node = gltf->nodes[nodeIndex];
    collectedNodes.push_back(node);

    // 递归收集子节点
    const auto &children = node->getChildren();
    for (int childIndex: children) {
      gatherNode(gltf, childIndex, collectedNodes);
    }
  }
}

} // namespace digitalhumans