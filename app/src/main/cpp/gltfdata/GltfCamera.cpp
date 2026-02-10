//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfCamera.h"
#include "GltfRenderer.h"
#include "gtc/matrix_transform.hpp"
#include "gtx/quaternion.hpp"
#include "gtc/matrix_inverse.hpp"
#include <algorithm>
#include <stdexcept>
#include "GltfNode.h"
#include "GltfPrimitive.h"
#include "Gltf.h"

namespace digitalhumans {

// === PerspectiveCamera实现 ===
PerspectiveCamera::PerspectiveCamera()
    : GltfObject(), yfov(glm::radians(45.0f)), aspectRatio(std::nullopt),
      znear(0.1f), zfar(100.0f) {
}



// === OrthographicCamera实现 ===
OrthographicCamera::OrthographicCamera()
    : GltfObject(), xmag(1.0f), ymag(1.0f), znear(0.01f), zfar(100.0f) {
}


// === GltfCamera实现 ===
GltfCamera::GltfCamera()
    : GltfObject(), name(""), node(std::nullopt), type(Type::PERSPECTIVE),
      perspective(), orthographic() {
}


std::vector<Drawable>
GltfCamera::sortPrimitivesByDepth(std::shared_ptr<Gltf> gltf,
                                  std::vector<Drawable> drawables) {
  const glm::mat4 viewMatrix = getViewMatrix(gltf);
  // 预计算距离，避免在排序过程中重复计算
  for (auto &drawable: drawables) {
    // 计算模型视图矩阵
    glm::mat4 modelView = viewMatrix * drawable.node->getWorldTransform();

    // 变换图元质心以找到图元的深度
    glm::vec3 centroid = drawable.primitive->getCentroid();
    glm::vec4 pos = modelView * glm::vec4(centroid, 1.0f);

    // 存储深度值（Z坐标）
    drawable.depth = pos.z;
  }

  // 1. 移除相机后面的图元
  //    --> 它们永远不会可见，在这里丢弃它们成本很低
  auto it = std::remove_if(drawables.begin(), drawables.end(),
                           [](const Drawable &drawable) {
                             return drawable.depth
                                 > 0.0f; // 在观察空间中，相机后面的物体z > 0
                           });
  drawables.erase(it, drawables.end());

  // 2. 排序图元，使最远的节点首先渲染
  //    这是正确透明渲染所必需的
  std::sort(drawables.begin(), drawables.end(),
            [](const Drawable &a, const Drawable &b) {
              return a.depth < b.depth; // 从远到近排序（深度值越小越远）
            });

  return drawables;
}

glm::mat4 GltfCamera::getProjectionMatrix(float aspectRatio) const {
  glm::mat4 projection(1.0f);

  if (type == Type::PERSPECTIVE) {
    const float
        actualAspectRatio = perspective.getAspectRatio().value_or(aspectRatio);
    projection = glm::perspective(
        perspective.getYfov(),
        actualAspectRatio,
        perspective.getZnear(),
        perspective.getZfar()
    );
  } else if (type == Type::ORTHOGRAPHIC) {
    const float znear = orthographic.getZnear();
    const float zfar = orthographic.getZfar();

    // 手动构建正交投影矩阵
    projection = glm::mat4(1.0f);
    projection[0][0] = 1.0f / orthographic.getXmag();
    projection[1][1] = 1.0f / orthographic.getYmag();
    projection[2][2] = 2.0f / (znear - zfar);
    projection[3][2] = (zfar + znear) / (znear - zfar);
  }

  return projection;
}

glm::mat4 GltfCamera::getViewMatrix(std::shared_ptr<Gltf> gltf) const {
  glm::mat4 result(1.0f);
  const glm::mat4 transform = getTransformMatrix(gltf);
  result = glm::inverse(transform);
  return result;
}

glm::vec3 GltfCamera::getTarget(const std::shared_ptr<Gltf> &gltf) const {
  const glm::vec3 position = getPosition(gltf);
  const glm::vec3 lookDirection = getLookDirection(gltf);
  return position + lookDirection;
}

glm::vec3 GltfCamera::getPosition(std::shared_ptr<Gltf> gltf) const {
  try {
    const auto node = getNode(gltf);
    return glm::vec3(node->getWorldTransform()[3]);
  } catch (const std::exception &e) {
    return glm::vec3(0.0f, 1.0f, 5.0f);  // 默认摄像机位置（避免在原点）
  }
}

glm::vec3 GltfCamera::getLookDirection(std::shared_ptr<Gltf> gltf) const {
  const glm::quat rotation = getRotation(gltf);
  const glm::vec3 direction = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
  return direction;
}

//    glm::mat4 GltfCamera::getRotationMatrix(std::shared_ptr<Gltf> gltf) const
//    {
//        const auto node = getNode(gltf);
//        return node->getWorldRotation();
//    }

glm::quat GltfCamera::getRotation(std::shared_ptr<Gltf> gltf) const {
  const auto node = getNode(gltf);
  return node->getWorldQuaternion();
}

void GltfCamera::setNode(std::shared_ptr<Gltf> gltf, int nodeIndex) {
  if (!gltf || nodeIndex < 0
      || nodeIndex >= static_cast<int>(gltf->getNodes().size())) {
    throw std::invalid_argument("Invalid camera node index");
  }

  const auto &nodePtr = gltf->getNodes()[nodeIndex];
  if (!nodePtr || !nodePtr->getCamera().has_value()) {
    throw std::invalid_argument("Node does not have a camera");
  }

  node = nodeIndex;
}

std::shared_ptr<GltfNode>
GltfCamera::getNode(std::shared_ptr<Gltf> gltf) const {
  if (!node.has_value() || !gltf) {
    throw std::runtime_error("Camera node is not defined");
  }

  const int nodeIndex = node.value();
  if (nodeIndex < 0 || nodeIndex >= static_cast<int>(gltf->getNodes().size())) {
    throw std::runtime_error("Camera node index is out of range");
  }

  const auto &nodePtr = gltf->getNodes()[nodeIndex];
  if (!nodePtr || !nodePtr->getCamera().has_value()) {
    throw std::runtime_error("Node does not have a camera");
  }

  return nodePtr;
}

glm::mat4 GltfCamera::getTransformMatrix(std::shared_ptr<Gltf> gltf) const {
  try {
    const auto nodePtr = getNode(gltf);
    if (!nodePtr || !nodePtr->hasWorldTransform()) {
      return glm::mat4(1.0f);
    }

    // 从旋转和位置组合变换，忽略缩放
    const glm::quat rotation = getRotation(gltf);
    const glm::vec3 position = getPosition(gltf);
    const glm::vec3 scale(1.0f, 1.0f, 1.0f);

    glm::mat4 result(1.0f);
    result = glm::translate(result, position);
    result = result * glm::mat4_cast(rotation);
    result = glm::scale(result, scale);

    return result;
  } catch (const std::exception &) {
    return createFallbackViewMatrix();
  }
}

// 回退方案：当变换矩阵无效时使用
glm::mat4 GltfCamera::createFallbackViewMatrix() const {
  // 创建一个默认的摄像机位置：在+Z方向看向原点
  glm::vec3 defaultPosition(0.0f, 1.0f, 5.0f);
  return glm::translate(glm::mat4(1.0f), defaultPosition);
}

std::unordered_map<std::string, std::any>
GltfCamera::getDescription(std::shared_ptr<Gltf> gltf) const {
  std::unordered_map<std::string, std::any> result;

  // Asset信息
  std::unordered_map<std::string, std::any> asset;
  asset["generator"] = std::string("gltf-sample-renderer");
  asset["version"] = std::string("2.0");
  result["asset"] = asset;

  // 摄像机信息
  std::unordered_map<std::string, std::any> camera;
  camera["type"] = typeToString(type);

  if (!name.empty()) {
    camera["name"] = name;
  }

  if (type == Type::PERSPECTIVE) {
    std::unordered_map<std::string, std::any> perspectiveData;

    if (perspective.getAspectRatio().has_value()) {
      perspectiveData["aspectRatio"] = perspective.getAspectRatio().value();
    }
    perspectiveData["yfov"] = perspective.getYfov();

    if (perspective.getZfar() != std::numeric_limits<float>::infinity()) {
      perspectiveData["zfar"] = perspective.getZfar();
    }
    perspectiveData["znear"] = perspective.getZnear();

    camera["perspective"] = perspectiveData;
  } else if (type == Type::ORTHOGRAPHIC) {
    std::unordered_map<std::string, std::any> orthographicData;
    orthographicData["xmag"] = orthographic.getXmag();
    orthographicData["ymag"] = orthographic.getYmag();
    orthographicData["zfar"] = orthographic.getZfar();
    orthographicData["znear"] = orthographic.getZnear();

    camera["orthographic"] = orthographicData;
  }

  std::vector<std::any> cameras;
  cameras.push_back(camera);
  result["cameras"] = cameras;

  // 节点信息
  std::unordered_map<std::string, std::any> nodeData;
  nodeData["camera"] = 0;

  try {
    const glm::mat4 mat = getTransformMatrix(gltf);
    std::vector<float> matrix;
    matrix.reserve(16);

    // GLM是列主序，需要转换为行主序存储
    for (int col = 0; col < 4; ++col) {
      for (int row = 0; row < 4; ++row) {
        matrix.push_back(mat[col][row]);
      }
    }

    nodeData["matrix"] = matrix;

    if (node.has_value() && gltf) {
      const auto nodePtr = getNode(gltf);
      if (nodePtr && !nodePtr->getName().empty()) {
        nodeData["name"] = nodePtr->getName();
      }
    }
  } catch (const std::exception &) {
    // 如果获取变换矩阵失败，使用单位矩阵
    std::vector<float> identityMatrix = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    nodeData["matrix"] = identityMatrix;
  }

  std::vector<std::any> nodes;
  nodes.push_back(nodeData);
  result["nodes"] = nodes;

  return result;
}


std::string GltfCamera::typeToString(Type type) {
  switch (type) {
    case Type::PERSPECTIVE:
      return "perspective";
    case Type::ORTHOGRAPHIC:
      return "orthographic";
    default:
      return "perspective";
  }
}

GltfCamera::Type GltfCamera::stringToType(const std::string &str) {
  if (str == "orthographic") {
    return Type::ORTHOGRAPHIC;
  }
  return Type::PERSPECTIVE; // 默认为透视
}

} // namespace digitalhumans