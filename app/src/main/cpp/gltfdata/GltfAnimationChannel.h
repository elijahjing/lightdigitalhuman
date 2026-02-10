//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFANIMATIONCHANNEL_H
#define LIGHTDIGITALHUMAN_GLTFANIMATIONCHANNEL_H


#include "GltfObject.h"
#include <string>
#include <memory>
#include <optional>

namespace digitalhumans {

/**
 * @brief 插值路径枚举
 */
enum class InterpolationPath {
  TRANSLATION,    ///< 平移
  ROTATION,       ///< 旋转
  SCALE,          ///< 缩放
  WEIGHTS,        ///< 权重
  POINTER,        ///< 指针
  UNKNOWN         ///< 未知
};

/**
 * @brief 插值路径工具类
 */
class InterpolationPathUtils {
 public:
  /**
   * @brief 从字符串获取插值路径枚举
   * @param pathString 路径字符串
   * @return 插值路径枚举值
   */
  static InterpolationPath fromString(const std::string &pathString);

  /**
   * @brief 从插值路径枚举获取字符串
   * @param path 插值路径枚举值
   * @return 路径字符串
   */
  static std::string toString(InterpolationPath path);

  /**
   * @brief 检查路径是否有效
   * @param path 插值路径枚举值
   * @return 如果路径有效返回true
   */
  static bool isValid(InterpolationPath path);

  /**
   * @brief 获取所有支持的路径字符串
   * @return 包含所有路径字符串的vector
   */
  static std::vector<std::string> getAllPathStrings();

  // 路径字符串常量
  static const std::string TRANSLATION_STR;
  static const std::string ROTATION_STR;
  static const std::string SCALE_STR;
  static const std::string WEIGHTS_STR;
  static const std::string POINTER_STR;
};

/**
 * @brief glTF动画目标类
 */
class GltfAnimationTarget: public GltfObject {
 public:
  /**
   * @brief 构造函数
   */
  GltfAnimationTarget();

  /**
   * @brief 析构函数
   */
  ~GltfAnimationTarget() override = default;


  /**
   * @brief 克隆对象
   * @return 克隆的对象
   */
  std::shared_ptr<GltfAnimationTarget> clone() const;

  // === Getter/Setter方法 ===

  /**
   * @brief 获取目标节点索引
   * @return 节点索引，如果未设置则返回std::nullopt
   */
  const std::optional<int> &getNode() const { return node; }

  /**
   * @brief 设置目标节点索引
   * @param nodeIndex 节点索引
   */
  void setNode(int nodeIndex) { node = nodeIndex; }

  /**
   * @brief 清除目标节点索引
   */
  void clearNode() { node.reset(); }

  /**
   * @brief 检查是否有目标节点
   * @return 如果有目标节点返回true
   */
  bool hasNode() const { return node.has_value(); }

  /**
   * @brief 获取动画路径
   * @return 动画路径枚举值
   */
  InterpolationPath getPath() const { return path; }

  /**
   * @brief 设置动画路径
   * @param animationPath 动画路径枚举值
   */
  void setPath(InterpolationPath animationPath) { path = animationPath; }

  /**
   * @brief 从字符串设置动画路径
   * @param pathString 路径字符串
   */
  void setPathFromString(const std::string &pathString);

  /**
   * @brief 获取动画路径字符串
   * @return 路径字符串
   */
  std::string getPathString() const;

  /**
   * @brief 检查动画路径是否有效
   * @return 如果路径有效返回true
   */
  bool isPathValid() const;

  /**
   * @brief 检查是否为平移动画
   * @return 如果是平移动画返回true
   */
  bool isTranslationAnimation() const {
    return path == InterpolationPath::TRANSLATION;
  }

  /**
   * @brief 检查是否为旋转动画
   * @return 如果是旋转动画返回true
   */
  bool isRotationAnimation() const {
    return path == InterpolationPath::ROTATION;
  }

  /**
   * @brief 检查是否为缩放动画
   * @return 如果是缩放动画返回true
   */
  bool isScaleAnimation() const { return path == InterpolationPath::SCALE; }

  /**
   * @brief 检查是否为权重动画
   * @return 如果是权重动画返回true
   */
  bool isWeightsAnimation() const { return path == InterpolationPath::WEIGHTS; }

  /**
   * @brief 检查是否为指针动画
   * @return 如果是指针动画返回true
   */
  bool isPointerAnimation() const { return path == InterpolationPath::POINTER; }

 private:
  std::optional<int> node;        ///< 目标节点索引
  InterpolationPath path;         ///< 动画路径
};

/**
 * @brief glTF动画通道类
 */
class GltfAnimationChannel: public GltfObject {
 public:
  /**
   * @brief 构造函数
   */
  GltfAnimationChannel();

  /**
   * @brief 析构函数
   */
  virtual ~GltfAnimationChannel() = default;



  /**
   * @brief 克隆对象
   * @return 克隆的对象
   */
  std::shared_ptr<GltfAnimationChannel> clone() const;

  /**
   * @brief 验证通道数据的有效性
   * @param gltf glTF根对象
   * @return 如果数据有效返回true
   */
  bool validate(std::shared_ptr<class Gltf> gltf) const;

  // === Getter/Setter方法 ===

  /**
   * @brief 获取动画目标
   * @return 动画目标对象
   */
  std::shared_ptr<GltfAnimationTarget> getTarget() const { return target; }

  /**
   * @brief 设置动画目标
   * @param animationTarget 动画目标对象
   */
  void setTarget(std::shared_ptr<GltfAnimationTarget> animationTarget) {
    target = animationTarget;
  }

  /**
   * @brief 创建并设置动画目标
   * @param nodeIndex 目标节点索引
   * @param path 动画路径
   */
  void createTarget(int nodeIndex, InterpolationPath path);

  /**
   * @brief 创建并设置动画目标（从字符串路径）
   * @param nodeIndex 目标节点索引
   * @param pathString 动画路径字符串
   */
  void createTarget(int nodeIndex, const std::string &pathString);

  /**
   * @brief 检查是否有动画目标
   * @return 如果有动画目标返回true
   */
  bool hasTarget() const { return target != nullptr; }

  /**
   * @brief 获取采样器索引
   * @return 采样器索引，如果未设置则返回std::nullopt
   */
  const std::optional<int> &getSampler() const { return sampler; }

  /**
   * @brief 设置采样器索引
   * @param samplerIndex 采样器索引
   */
  void setSampler(int samplerIndex) { sampler = samplerIndex; }

  /**
   * @brief 清除采样器索引
   */
  void clearSampler() { sampler.reset(); }

  /**
   * @brief 检查是否有采样器
   * @return 如果有采样器返回true
   */
  bool hasSampler() const { return sampler.has_value(); }

  /**
   * @brief 获取目标节点索引（便捷方法）
   * @return 目标节点索引，如果没有目标或目标没有节点则返回std::nullopt
   */
  std::optional<int> getTargetNode() const;

  /**
   * @brief 获取目标路径（便捷方法）
   * @return 目标路径，如果没有目标则返回UNKNOWN
   */
  InterpolationPath getTargetPath() const;

  /**
   * @brief 获取目标路径字符串（便捷方法）
   * @return 目标路径字符串
   */
  std::string getTargetPathString() const;

  /**
   * @brief 检查通道是否完整（有目标和采样器）
   * @return 如果通道完整返回true
   */
  bool isComplete() const;

 private:
  std::shared_ptr<GltfAnimationTarget> target;    ///< 动画目标
  std::optional<int> sampler;                     ///< 采样器索引
};

} // namespace digitalhumans

#endif //LIGHTDIGITALHUMAN_GLTFANIMATIONCHANNEL_H
