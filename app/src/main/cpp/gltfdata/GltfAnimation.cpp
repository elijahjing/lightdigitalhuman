//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfAnimation.h"

#include "../utils/utils.h"
#include <sstream>
#include <algorithm>
#include <regex>
#include "../utils/LogUtils.h"
#include "Gltf.h"

#include "GltfNode.h"
#include "GltfAccessor.h"

#include "GltfState.h"


namespace digitalhumans {

GltfAnimation::GltfAnimation()
    : GltfObject(), channels(), samplers(), name(""), interpolators(),
      maxTime(0.0f), disjointAnimations(),
      errors(), reportedErrors() {
}


void GltfAnimation::initGl(std::shared_ptr<Gltf> gltf,
                           std::shared_ptr<GltfOpenGLContext> openGlContext) {
  initializeInterpolators();
}


std::shared_ptr<GltfAnimation> GltfAnimation::clone() const {
  auto cloned = std::make_shared<GltfAnimation>();

  // 克隆动画通道
  for (const auto &channel: channels) {
    if (channel) {
      cloned->channels.push_back(channel->clone());
    }
  }

  // 克隆动画采样器
  for (const auto &sampler: samplers) {
    if (sampler) {
      cloned->samplers.push_back(sampler->clone());
    }
  }

  cloned->name = name;
  cloned->maxTime = maxTime;
  cloned->errors = errors;

  // 重新初始化插值器
  cloned->initializeInterpolators();

  return cloned;
}

bool GltfAnimation::validate(std::shared_ptr<Gltf> gltf) const {
  if (!gltf) {
    LOGE("Invalid glTF object for validation");
    return false;
  }

  // 检查是否有动画通道
  if (channels.empty()) {
    LOGE("Animation has no channels");
    return false;
  }

  // 检查是否有动画采样器
  if (samplers.empty()) {
    LOGE("Animation has no samplers");
    return false;
  }

  // 验证每个动画通道
  for (size_t i = 0; i < channels.size(); ++i) {
    auto channel = channels[i];
    if (!channel) {
      LOGE("Animation channel %zu is null", i);
      return false;
    }

    if (!channel->validate(gltf)) {
      LOGE("Animation channel %zu validation failed", i);
      return false;
    }

    // 检查采样器索引
    if (!channel->hasSampler()) {
      LOGE("Animation channel %zu has no sampler", i);
      return false;
    }

    int samplerIndex = channel->getSampler().value();
    if (samplerIndex < 0 || samplerIndex >= static_cast<int>(samplers.size())) {
      LOGE("Animation channel %zu has invalid sampler index: %d",
           i,
           samplerIndex);
      return false;
    }
  }

  // 验证每个动画采样器
  for (size_t i = 0; i < samplers.size(); ++i) {
    auto sampler = samplers[i];
    if (!sampler) {
      LOGE("Animation sampler %zu is null", i);
      return false;
    }

    if (!sampler->validate(gltf)) {
      LOGE("Animation sampler %zu validation failed", i);
      return false;
    }
  }

  // 检查插值器数量是否与通道数量匹配
  if (interpolators.size() != channels.size()) {
    LOGE("Interpolator count (%zu) does not match channel count (%zu)",
         interpolators.size(), channels.size());
    return false;
  }

  return true;
}

float GltfAnimation::getCurrentTime() {
  auto now = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      now - gameStartTime);
  return duration.count() / 1000000.0f;  // 转换为秒
}

void GltfAnimation::advance(std::shared_ptr<GltfState> state,
                            std::optional<float> totalTime,
                            int time,
                            int index) {
  auto gltf = state->getGltf();
  if (!gltf || channels.empty()) {
    return;
  }

  // 如果没有指定时间，停用动画
  if (!totalTime.has_value()) {
    reset(gltf);
    return;
  }

  // 计算最大时间（如果还未计算）
  if (maxTime == 0.0f) {
    maxTime = calculateMaxTime(gltf);
  }
  loopCount = time;
  if (startTime == 0.0f) {
    startTime = getCurrentTime();
  }
  // 处理每个动画通道
  for (size_t i = 0; i < channels.size() && i < interpolators.size(); ++i) {
    bool stop = processChannel(gltf, i, totalTime.value());
    if (!stop) {
      state->removeAnimationIndex(index);
      return;
    }
  }
}

void GltfAnimation::reset(std::shared_ptr<Gltf> gltf) {
  if (!gltf) {
    return;
  }

  // 重置所有动画属性到初始值
  for (const auto &channel: channels) {
    if (!channel || !channel->hasTarget()) {
      continue;
    }

    std::string property = getPropertyPath(gltf, channel);
    if (!property.empty()) {
      // 这里需要实现属性重置逻辑
      // 具体实现取决于AnimatableProperty的接口
      LOGI("Resetting property: %s", property.c_str());
    }
  }
}

float GltfAnimation::calculateMaxTime(std::shared_ptr<Gltf> gltf) {
  if (!gltf) {
    return 0.0f;
  }

  float calculatedMaxTime = 0.0f;
  const auto &accessors = gltf->getAccessors();

  for (const auto &channel: channels) {
    if (!channel || !channel->hasSampler()) {
      continue;
    }

    int samplerIndex = channel->getSampler().value();
    if (samplerIndex < 0 || samplerIndex >= static_cast<int>(samplers.size())) {
      continue;
    }

    auto sampler = samplers[samplerIndex];
    if (!sampler || !sampler->hasInput()) {
      continue;
    }

    int inputAccessorIndex = sampler->getInput().value();
    if (inputAccessorIndex < 0
        || inputAccessorIndex >= static_cast<int>(accessors.size())) {
      continue;
    }

    auto inputAccessor = accessors[inputAccessorIndex];
    if (!inputAccessor) {
      continue;
    }

    // 获取时间数据
    std::pair<const void *, size_t>
        timeData = inputAccessor->getDeinterlacedView(*gltf);
    if (timeData.second > 0) {
      // 获取访问器的组件类型和数量
      const size_t componentCount = inputAccessor->getCount().value();
      const GLenum componentType = inputAccessor->getComponentType().value();

      if (componentCount > 0) {
        float maxTimeValue = 0.0f;

        // 根据组件类型转换数据指针
        switch (componentType) {
          case GL_FLOAT: {
            const float *floatData = static_cast<const float *>(timeData.first);
            maxTimeValue = floatData[componentCount - 1]; // 获取最后一个元素
            break;
          }
          case GL_UNSIGNED_SHORT: {
            const uint16_t
                *shortData = static_cast<const uint16_t *>(timeData.first);
            maxTimeValue = static_cast<float>(shortData[componentCount - 1]);
            break;
          }
          case GL_UNSIGNED_BYTE: {
            const uint8_t
                *byteData = static_cast<const uint8_t *>(timeData.first);
            maxTimeValue = static_cast<float>(byteData[componentCount - 1]);
            break;
          }
            // 添加其他需要的类型...
          default:
            LOGW("Unsupported component type for time data: %d", componentType);
            continue;
        }

        if (maxTimeValue > calculatedMaxTime) {
          calculatedMaxTime = maxTimeValue;
        }
      }
    }
  }

  return calculatedMaxTime;
}

float GltfAnimation::getDuration(std::shared_ptr<Gltf> gltf) {
  if (maxTime == 0.0f) {
    maxTime = calculateMaxTime(gltf);
  }
  return maxTime;
}

bool GltfAnimation::isActiveAtTime(float time) const {
  return time >= 0.0f && time <= maxTime;
}

std::vector<int> GltfAnimation::getAffectedNodes() const {
  std::vector<int> affectedNodes;
  std::unordered_set<int> nodeSet; // 避免重复

  for (const auto &channel: channels) {
    if (channel && channel->hasTarget()) {
      auto targetNode = channel->getTargetNode();
      if (targetNode.has_value()
          && nodeSet.find(targetNode.value()) == nodeSet.end()) {
        nodeSet.insert(targetNode.value());
        affectedNodes.push_back(targetNode.value());
      }
    }
  }

  return affectedNodes;
}

std::vector<std::string> GltfAnimation::getAnimatedProperties() const {
  std::vector<std::string> properties;

  for (const auto &channel: channels) {
    if (channel && channel->hasTarget()) {
      properties.push_back(channel->getTargetPathString());
    }
  }

  return properties;
}

// === Getter/Setter方法实现 ===

void GltfAnimation::addChannel(std::shared_ptr<GltfAnimationChannel> channel) {
  if (channel) {
    channels.push_back(channel);
    // 添加对应的插值器
    interpolators.push_back(std::make_shared<GltfInterpolator>());
  }
}

bool GltfAnimation::removeChannel(size_t index) {
  if (index >= channels.size()) {
    return false;
  }

  channels.erase(channels.begin() + index);

  // 同时移除对应的插值器
  if (index < interpolators.size()) {
    interpolators.erase(interpolators.begin() + index);
  }

  return true;
}

std::shared_ptr<GltfAnimationChannel>
GltfAnimation::getChannel(size_t index) const {
  return (index < channels.size()) ? channels[index] : nullptr;
}

void GltfAnimation::addSampler(std::shared_ptr<GltfAnimationSampler> sampler) {
  if (sampler) {
    samplers.push_back(sampler);
  }
}

bool GltfAnimation::removeSampler(size_t index) {
  if (index >= samplers.size()) {
    return false;
  }

  samplers.erase(samplers.begin() + index);
  return true;
}

std::shared_ptr<GltfAnimationSampler>
GltfAnimation::getSampler(size_t index) const {
  return (index < samplers.size()) ? samplers[index] : nullptr;
}

std::string GltfAnimation::getDescription() const {
  std::ostringstream oss;

  oss << "Animation";
  if (!name.empty()) {
    oss << " '" << name << "'";
  }
  oss << ":\n";

  oss << "  Channels: " << channels.size() << "\n";
  oss << "  Samplers: " << samplers.size() << "\n";
  oss << "  Duration: " << maxTime << " seconds\n";

  if (!errors.empty()) {
    oss << "  Errors: " << errors.size();
  }

  return oss.str();
}

std::string GltfAnimation::getStatistics() const {
  std::ostringstream oss;

  oss << "Animation Statistics:\n";
  oss << "  Name: " << (name.empty() ? "(unnamed)" : name) << "\n";
  oss << "  Channels: " << channels.size() << "\n";
  oss << "  Samplers: " << samplers.size() << "\n";
  oss << "  Interpolators: " << interpolators.size() << "\n";
  oss << "  Duration: " << maxTime << " seconds\n";
  oss << "  Affected Nodes: " << getAffectedNodes().size() << "\n";
  oss << "  Error Count: " << errors.size();

  return oss.str();
}

std::shared_ptr<GltfAnimation>
GltfAnimation::create(const std::string &animationName) {
  auto animation = std::make_shared<GltfAnimation>();
  animation->setName(animationName);
  return animation;
}

// === 私有方法实现 ===

void GltfAnimation::initializeInterpolators() {
  interpolators.clear();
  interpolators.reserve(channels.size());

  for (size_t i = 0; i < channels.size(); ++i) {
    interpolators.push_back(std::make_shared<GltfInterpolator>());
  }
}


std::string
GltfAnimation::getPropertyPath(std::shared_ptr<Gltf> gltf,
                               std::shared_ptr<GltfAnimationChannel> channel) const {
  if (!channel || !channel->hasTarget()) {
    return "";
  }

  auto target = channel->getTarget();
  if (!target || !target->hasNode()) {
    return "";
  }

  int nodeIndex = target->getNode().value();
  InterpolationPath path = target->getPath();

  std::string property;

  switch (path) {
    case InterpolationPath::TRANSLATION:
      property = "/nodes/" + std::to_string(nodeIndex) + "/translation";
      break;

    case InterpolationPath::ROTATION:
      property = "/nodes/" + std::to_string(nodeIndex) + "/rotation";
      break;

    case InterpolationPath::SCALE:
      property = "/nodes/" + std::to_string(nodeIndex) + "/scale";
      break;

    case InterpolationPath::WEIGHTS: {
      const auto &nodes = gltf->getNodes();
      if (nodeIndex >= 0 && nodeIndex < static_cast<int>(nodes.size())) {
        auto node = nodes[nodeIndex];
        if (node && node->hasWeights()) {
          property = "/nodes/" + std::to_string(nodeIndex) + "/weights";
        } else if (node && node->hasMesh()) {
          int meshIndex = node->getMesh().value();
          property = "/meshes/" + std::to_string(meshIndex) + "/weights";
        }
      }
    }
      break;

    case InterpolationPath::POINTER: {
      const auto &nodes = gltf->getNodes();
      if (nodeIndex >= 0 && nodeIndex < static_cast<int>(nodes.size())) {
        auto node = nodes[nodeIndex];
        if (node && node->hasMesh()) {
          int meshIndex = node->getMesh().value();
          property = "/meshes/" + std::to_string(meshIndex)
              + "/primitives/0/attributes/POSITION";
        }
      }
    }
      break;

    default:
      break;
  }

  return property;
}

// 替换原来的getPropertyPath方法
GltfAnimationTarget
GltfAnimation::getAnimationTarget(std::shared_ptr<Gltf> gltf,
                                  std::shared_ptr<GltfAnimationChannel> channel) const {
  GltfAnimationTarget target;

  if (!channel || !channel->hasTarget()) {
    return target;
  }

  auto channelTarget = channel->getTarget();
  if (!channelTarget || !channelTarget->hasNode()) {
    return target;
  }

  int nodeIndex = channelTarget->getNode().value();
  const auto &nodes = gltf->getNodes();

  if (nodeIndex < 0 || nodeIndex >= static_cast<int>(nodes.size())) {
    return target;
  }

  target.setNode(nodeIndex);
  target.setPath(channelTarget->getPath());
  return target;
}

// 重构后的processChannel方法
bool GltfAnimation::processChannel(std::shared_ptr<Gltf> gltf,
                                   size_t channelIndex,
                                   float totalTime) {
  if (channelIndex >= channels.size() || channelIndex >= interpolators.size()) {
    return true;
  }

  auto channel = channels[channelIndex];
  auto interpolator = interpolators[channelIndex];

  if (!channel || !interpolator || !channel->hasSampler()) {
    return true;
  }

  int samplerIndex = channel->getSampler().value();
  if (samplerIndex < 0 || samplerIndex >= static_cast<int>(samplers.size())) {
    return true;
  }

  auto sampler = samplers[samplerIndex];
  if (!sampler) {
    return true;
  }
  GltfAnimationTarget target = getAnimationTarget(gltf, channel);
  if (!target.getNode().has_value()) {
    return true;
  }
  float currentTime = getCurrentTime() - startTime;
  // 检查动画是否应该结束
  if (shouldAnimationStop(currentTime)) {
    startTime = 0.0f;
    handleAnimationComplete(gltf, target);
    //   resetProperty(gltf, target);

    return false;
  }
  // 确定属性维度
  int stride = getPropertyStride(target.getPath());


  float loopTime = fmod(totalTime, maxTime);

  // 执行插值
  std::vector<float> interpolant = interpolator->interpolate(
      gltf, channel, sampler, loopTime, stride, maxTime);

  if (interpolant.empty()) {
    resetProperty(gltf, target);
    return true;
  }
  applyAnimationToTarget(gltf, target, interpolant);
  return true;
}

// 处理动画完成
void GltfAnimation::handleAnimationComplete(std::shared_ptr<Gltf> gltf,
                                            const GltfAnimationTarget &target) {

  LOGI("🏁 动画完成: %s", getName().c_str());

  // 设置到最终状态
  if (loopCount != -1) {
    setToFinalFrame(gltf, target);
  }
}

// 直接应用动画到目标对象
void GltfAnimation::applyAnimationToTarget(std::shared_ptr<Gltf> gltf,
                                           const GltfAnimationTarget &target,
                                           const std::vector<float> &interpolant) {
//        if (!target.getPath()) {
//            return;
//        }

  auto targetNode = gltf->getNodes()[target.getNode().value()];


  switch (target.getPath()) {
    case InterpolationPath::TRANSLATION: {
      if (interpolant.size() >= 3) {
        glm::vec3 translation(interpolant[0], interpolant[1], interpolant[2]);
        targetNode->setTranslation(translation);
      }
      break;
    }

    case InterpolationPath::ROTATION: {
      if (interpolant.size() >= 4) {
        glm::quat rotation(interpolant[3],
                           interpolant[0],
                           interpolant[1],
                           interpolant[2]); // w, x, y, z
        targetNode->setRotation(rotation);
      }
      break;
    }

    case InterpolationPath::SCALE: {
      if (interpolant.size() >= 3) {
        glm::vec3 scale(interpolant[0], interpolant[1], interpolant[2]);
        targetNode->setScale(scale);
      }
      break;
    }

    case InterpolationPath::WEIGHTS: {
      // 设置morph target权重
      std::vector<double> weights(interpolant.begin(), interpolant.end());
      LOGW("Unsupported animation path=======: %f", weights[0]);

      targetNode->setWeights(weights);
      break;
    }

    default:
      LOGW("Unsupported animation path: %d",
           static_cast<int>(target.getPath()));
      break;
  }

  // 标记节点需要更新变换矩阵
  //  target.targetNode->markTransformDirty();
}

// 重置属性到初始值
void GltfAnimation::resetProperty(std::shared_ptr<Gltf> gltf,
                                  const GltfAnimationTarget &target) {
  if (!target.getNode().has_value()) {
    return;
  }

  auto targetNode = gltf->getNodes()[target.getNode().value()];

  switch (target.getPath()) {
    case InterpolationPath::TRANSLATION:
      targetNode->resetTranslation();
      break;
    case InterpolationPath::ROTATION:
      targetNode->resetRotation();
      break;
    case InterpolationPath::SCALE:
      targetNode->resetScale();
      break;
    case InterpolationPath::WEIGHTS:
      targetNode->resetWeights();
      break;
  }

  // targetNode->markTransformDirty();
}

// 获取属性的维度/步长
int GltfAnimation::getPropertyStride(InterpolationPath path) const {
  switch (path) {
    case InterpolationPath::TRANSLATION:
    case InterpolationPath::SCALE:
      return 3; // vec3
    case InterpolationPath::ROTATION:
      return 4; // quaternion
    case InterpolationPath::WEIGHTS:
      return 1; // 每个权重是标量，但数量可变
    default:
      return 1;
  }
}

std::string
GltfAnimation::processLightProperty(const std::string &property) const {
  // 处理KHR_lights_punctual扩展的特殊情况
  const std::string lightPrefix = "/extensions/KHR_lights_punctual/";
  if (property.find(lightPrefix) == 0) {
    std::string suffix = property.substr(lightPrefix.length());
    return "/" + suffix;
  }

  return property;
}

// 检查动画是否应该停止
bool GltfAnimation::shouldAnimationStop(float adjustedTime) {
  if (maxTime <= 0.0f) {
    return false;
  }

  // 如果是无限循环
  if (loopCount == -1) {
    return false;
  }
  // 如果有限循环，检查循环次数
  if (loopCount > 0) {
    int completedLoops = static_cast<int>(adjustedTime / maxTime);
    if (completedLoops >= loopCount) {
      LOGE("Invalid glTF object for validation=========== %f,%f",
           adjustedTime,
           maxTime);

      return true;
    }
    currentLoop = completedLoops;
  }


  return false;
}

// 设置到最终帧
void GltfAnimation::setToFinalFrame(std::shared_ptr<Gltf> gltf,
                                    const GltfAnimationTarget &target) {

  // 找到对应的channel和sampler
  for (size_t i = 0; i < channels.size(); ++i) {
    auto channel = channels[i];
    if (!channel || !channel->hasSampler()) {
      continue;
    }

    GltfAnimationTarget channelTarget = getAnimationTarget(gltf, channel);
    if (channelTarget.getNode() == target.getNode() &&
        channelTarget.getPath() == target.getPath()) {

      // 获取最后一帧的数据
      int samplerIndex = channel->getSampler().value();
      auto sampler = samplers[samplerIndex];

      if (sampler && i < interpolators.size()) {
        auto interpolator = interpolators[i];
        int stride = getPropertyStride(target.getPath());
        // 使用maxTime获取最终状态
        std::vector<float> finalInterpolant = interpolator->interpolate(
            gltf, channel, sampler, maxTime, stride, maxTime);

        if (!finalInterpolant.empty()) {
          applyAnimationToTarget(gltf, target, finalInterpolant);
        }
      }
      break;
    }
  }
}

} // namespace digitalhumans