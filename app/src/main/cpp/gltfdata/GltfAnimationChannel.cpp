//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfAnimationChannel.h"

#include "../utils/utils.h"
#include <unordered_map>
#include "../utils/LogUtils.h"
#include "Gltf.h"


namespace digitalhumans {

// ===== InterpolationPathUtils实现 =====

const std::string InterpolationPathUtils::TRANSLATION_STR = "translation";
const std::string InterpolationPathUtils::ROTATION_STR = "rotation";
const std::string InterpolationPathUtils::SCALE_STR = "scale";
const std::string InterpolationPathUtils::WEIGHTS_STR = "weights";
const std::string InterpolationPathUtils::POINTER_STR = "pointer";

// 字符串到枚举的映射表
static const std::unordered_map<std::string, InterpolationPath> stringToPath = {
    {InterpolationPathUtils::TRANSLATION_STR, InterpolationPath::TRANSLATION},
    {InterpolationPathUtils::ROTATION_STR, InterpolationPath::ROTATION},
    {InterpolationPathUtils::SCALE_STR, InterpolationPath::SCALE},
    {InterpolationPathUtils::WEIGHTS_STR, InterpolationPath::WEIGHTS},
    {InterpolationPathUtils::POINTER_STR, InterpolationPath::POINTER}
};

// 枚举到字符串的映射表
static const std::unordered_map<InterpolationPath, std::string> pathToString = {
    {InterpolationPath::TRANSLATION, InterpolationPathUtils::TRANSLATION_STR},
    {InterpolationPath::ROTATION, InterpolationPathUtils::ROTATION_STR},
    {InterpolationPath::SCALE, InterpolationPathUtils::SCALE_STR},
    {InterpolationPath::WEIGHTS, InterpolationPathUtils::WEIGHTS_STR},
    {InterpolationPath::POINTER, InterpolationPathUtils::POINTER_STR}
};

InterpolationPath
InterpolationPathUtils::fromString(const std::string &pathString) {
  auto it = stringToPath.find(pathString);
  return (it != stringToPath.end()) ? it->second : InterpolationPath::UNKNOWN;
}

std::string InterpolationPathUtils::toString(InterpolationPath path) {
  auto it = pathToString.find(path);
  return (it != pathToString.end()) ? it->second : "";
}

bool InterpolationPathUtils::isValid(InterpolationPath path) {
  return path != InterpolationPath::UNKNOWN;
}

std::vector<std::string> InterpolationPathUtils::getAllPathStrings() {
  return {TRANSLATION_STR, ROTATION_STR, SCALE_STR, WEIGHTS_STR, POINTER_STR};
}

// ===== GltfAnimationTarget实现 =====

GltfAnimationTarget::GltfAnimationTarget()
    : GltfObject(), node(), path(InterpolationPath::UNKNOWN) {
}


std::shared_ptr<GltfAnimationTarget> GltfAnimationTarget::clone() const {
  auto cloned = std::make_shared<GltfAnimationTarget>();
  cloned->node = node;
  cloned->path = path;
  return cloned;
}

void GltfAnimationTarget::setPathFromString(const std::string &pathString) {
  path = InterpolationPathUtils::fromString(pathString);
  if (path == InterpolationPath::UNKNOWN) {
    LOGW("Unknown animation path: %s", pathString.c_str());
  }
}

std::string GltfAnimationTarget::getPathString() const {
  return InterpolationPathUtils::toString(path);
}

bool GltfAnimationTarget::isPathValid() const {
  return InterpolationPathUtils::isValid(path);
}

// ===== GltfAnimationChannel实现 =====

GltfAnimationChannel::GltfAnimationChannel()
    : GltfObject(), target(nullptr), sampler() {
}



std::shared_ptr<GltfAnimationChannel> GltfAnimationChannel::clone() const {
  auto cloned = std::make_shared<GltfAnimationChannel>();
  cloned->target = target ? target->clone() : nullptr;
  cloned->sampler = sampler;
  return cloned;
}

bool GltfAnimationChannel::validate(std::shared_ptr<Gltf> gltf) const {
  if (!gltf) {
    LOGE("Invalid glTF object for validation");
    return false;
  }

  // 检查是否有目标
  if (!target) {
    LOGE("Animation channel missing target");
    return false;
  }

  // 检查是否有采样器
  if (!sampler.has_value()) {
    LOGE("Animation channel missing sampler");
    return false;
  }

  // 验证目标节点索引
  if (target->hasNode()) {
    int nodeIndex = target->getNode().value();
    const auto &nodes = gltf->getNodes();
    if (nodeIndex < 0 || nodeIndex >= static_cast<int>(nodes.size())) {
      LOGE("Invalid target node index: %d", nodeIndex);
      return false;
    }
  }


  // 验证目标路径
  if (!target->isPathValid()) {
    LOGE("Invalid animation target path: %s", target->getPathString().c_str());
    return false;
  }

  return true;
}

void GltfAnimationChannel::createTarget(int nodeIndex, InterpolationPath path) {
  target = std::make_shared<GltfAnimationTarget>();
  target->setNode(nodeIndex);
  target->setPath(path);
}

void GltfAnimationChannel::createTarget(int nodeIndex,
                                        const std::string &pathString) {
  InterpolationPath path = InterpolationPathUtils::fromString(pathString);
  createTarget(nodeIndex, path);
}

std::optional<int> GltfAnimationChannel::getTargetNode() const {
  if (target && target->hasNode()) {
    return target->getNode();
  }
  return std::nullopt;
}

InterpolationPath GltfAnimationChannel::getTargetPath() const {
  return target ? target->getPath() : InterpolationPath::UNKNOWN;
}

std::string GltfAnimationChannel::getTargetPathString() const {
  return target ? target->getPathString() : "";
}

bool GltfAnimationChannel::isComplete() const {
  return target != nullptr && sampler.has_value() && target->hasNode()
      && target->isPathValid();
}

} // namespace digitalhumans