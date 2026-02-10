//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFANIMATION_H
#define LIGHTDIGITALHUMAN_GLTFANIMATION_H

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <unordered_set>
#include "GltfAnimationChannel.h"
#include "GltfAnimationSampler.h"
#include "GltfInterpolator.h"

namespace digitalhumans {
class GltfState;

/**
 * @brief glTF动画类
 *
 * 管理动画通道、采样器和插值器，用于驱动glTF场景中对象的动画
 */
class GltfAnimation: public GltfObject {
 public:
  /**
   * @brief 构造函数
   */
  GltfAnimation();

  /**
   * @brief 析构函数
   */
  virtual ~GltfAnimation() = default;



  /**
   * @brief 克隆对象
   * @return 克隆的对象
   */
  std::shared_ptr<GltfAnimation> clone() const;

  /**
   * @brief 验证动画数据的有效性
   * @param gltf glTF根对象
   * @return 如果数据有效返回true
   */
  bool validate(std::shared_ptr<Gltf> gltf) const;

  /**
   * @brief 推进动画到指定时间
   * @param gltf glTF根对象
   * @param totalTime 动画时间，如果为std::nullopt则停用动画
   */
  void advance(std::shared_ptr<Gltf> gltf,
               std::optional<float> totalTime,
               int timeIndex);

  /**
   * @brief 重置动画到初始状态
   * @param gltf glTF根对象
   */
  void reset(std::shared_ptr<Gltf> gltf);

  /**
   * @brief 计算动画的最大时间
   * @param gltf glTF根对象
   * @return 最大时间值
   */
  float calculateMaxTime(std::shared_ptr<Gltf> gltf);

  /**
   * @brief 获取动画时长
   * @param gltf glTF根对象
   * @return 动画时长
   */
  float getDuration(std::shared_ptr<Gltf> gltf);

  /**
   * @brief 检查动画是否在指定时间处于活动状态
   * @param time 时间值
   * @return 如果动画在该时间活动返回true
   */
  bool isActiveAtTime(float time) const;

  /**
   * @brief 获取动画影响的节点列表
   * @return 节点索引列表
   */
  std::vector<int> getAffectedNodes() const;

  /**
   * @brief 获取动画使用的属性路径列表
   * @return 属性路径字符串列表
   */
  std::vector<std::string> getAnimatedProperties() const;

  /**
   * @brief 检查动画是否有错误
   * @return 如果有错误返回true
   */
  bool hasErrors() const { return !errors.empty(); }

  /**
   * @brief 获取错误列表
   * @return 错误信息列表
   */
  const std::vector<std::string> &getErrors() const { return errors; }

  /**
   * @brief 清除错误列表
   */
  void clearErrors() { errors.clear(); }

  // === Getter/Setter方法 ===

  /**
   * @brief 获取动画通道列表
   * @return 动画通道列表
   */
  const std::vector<std::shared_ptr<GltfAnimationChannel>> &
  getChannels() const { return channels; }

  /**
   * @brief 设置动画通道列表
   * @param animationChannels 动画通道列表
   */
  void setChannels(
      const std::vector<std::shared_ptr<GltfAnimationChannel>> &animationChannels) {
    channels = animationChannels;
  }

  /**
   * @brief 添加动画通道
   * @param channel 动画通道
   */
  void addChannel(std::shared_ptr<GltfAnimationChannel> channel);

  /**
   * @brief 移除动画通道
   * @param index 通道索引
   * @return 如果成功移除返回true
   */
  bool removeChannel(size_t index);

  /**
   * @brief 获取动画通道数量
   * @return 通道数量
   */
  size_t getChannelCount() const { return channels.size(); }

  /**
   * @brief 获取指定索引的动画通道
   * @param index 通道索引
   * @return 动画通道，如果索引无效返回nullptr
   */
  std::shared_ptr<GltfAnimationChannel> getChannel(size_t index) const;

  /**
   * @brief 获取动画采样器列表
   * @return 动画采样器列表
   */
  const std::vector<std::shared_ptr<GltfAnimationSampler>> &
  getSamplers() const { return samplers; }

  /**
   * @brief 设置动画采样器列表
   * @param animationSamplers 动画采样器列表
   */
  void setSamplers(
      const std::vector<std::shared_ptr<GltfAnimationSampler>> &animationSamplers) {
    samplers = animationSamplers;
  }

  /**
   * @brief 添加动画采样器
   * @param sampler 动画采样器
   */
  void addSampler(std::shared_ptr<GltfAnimationSampler> sampler);

  /**
   * @brief 移除动画采样器
   * @param index 采样器索引
   * @return 如果成功移除返回true
   */
  bool removeSampler(size_t index);

  /**
   * @brief 获取动画采样器数量
   * @return 采样器数量
   */
  size_t getSamplerCount() const { return samplers.size(); }

  /**
   * @brief 获取指定索引的动画采样器
   * @param index 采样器索引
   * @return 动画采样器，如果索引无效返回nullptr
   */
  std::shared_ptr<GltfAnimationSampler> getSampler(size_t index) const;

  /**
   * @brief 获取动画名称
   * @return 动画名称
   */
  const std::string &getName() const { return name; }

  /**
   * @brief 设置动画名称
   * @param animationName 动画名称
   */
  void setName(const std::string &animationName) { name = animationName; }

  /**
   * @brief 检查是否有动画名称
   * @return 如果有名称返回true
   */
  bool hasName() const { return !name.empty(); }

  /**
   * @brief 获取最大时间
   * @return 最大时间值
   */
  float getMaxTime() const { return maxTime; }

  /**
   * @brief 获取动画描述信息
   * @return 描述信息字符串
   */
  std::string getDescription() const;

  /**
   * @brief 获取动画统计信息
   * @return 统计信息字符串
   */
  std::string getStatistics() const;

  /**
   * @brief 创建简单的动画对象
   * @param name 动画名称
   * @return 新创建的动画对象
   */
  static std::shared_ptr<GltfAnimation> create(const std::string &name = "");

  void advance(std::shared_ptr<GltfState> state,
               std::optional<float> totalTime,
               int time,
               int index);
  void initGl(std::shared_ptr<Gltf> gltf,
              std::shared_ptr<GltfOpenGLContext> openGlContext);

 private:
  /**
   * @brief 初始化插值器
   */
  void initializeInterpolators();

  /**
   * @brief 处理单个通道的动画
   * @param gltf glTF根对象
   * @param channelIndex 通道索引
   * @param totalTime 动画时间
   */
  bool processChannel(std::shared_ptr<Gltf> gltf,
                      size_t channelIndex,
                      float totalTime);

  GltfAnimationTarget getAnimationTarget(std::shared_ptr<Gltf> gltf,
                                         std::shared_ptr<GltfAnimationChannel> channel) const;

  /**
* @brief 获取属性路径
* @param gltf glTF根对象
* @param channel 动画通道
* @return 属性路径字符串，如果无法确定返回空字符串
*/
  std::string getPropertyPath(std::shared_ptr<Gltf> gltf,
                              std::shared_ptr<GltfAnimationChannel> channel) const;


  int getPropertyStride(InterpolationPath path) const;

  void
  resetProperty(std::shared_ptr<Gltf> gltf, const GltfAnimationTarget &target);

  void applyAnimationToTarget(std::shared_ptr<Gltf> gltf,
                              const GltfAnimationTarget &target,
                              const std::vector<float> &interpolant);


  /**
   * @brief 处理灯光动画的特殊情况
   * @param property 原始属性路径
   * @return 处理后的属性路径
   */
  std::string processLightProperty(const std::string &property) const;



 private:
  std::vector<std::shared_ptr<GltfAnimationChannel>>
      channels;        ///< 动画通道列表
  std::vector<std::shared_ptr<GltfAnimationSampler>>
      samplers;        ///< 动画采样器列表
  std::string name;                                                   ///< 动画名称

  // 非glTF标准属性
  std::vector<std::shared_ptr<GltfInterpolator>> interpolators;       ///< 插值器列表
  float maxTime;                                                      ///< 最大时间
  std::vector<std::shared_ptr<GltfAnimation>>
      disjointAnimations;     ///< 分离的动画列表

  std::vector<std::string>
      errors;                                    ///< 错误信息列表
  std::unordered_set<std::string>
      reportedErrors;                     ///< 已报告的错误（避免重复）
  void setToFinalFrame(std::shared_ptr<Gltf> gltf,
                       const GltfAnimationTarget &target);

  bool shouldAnimationStop(float adjustedTime);


  float startTime = 0.0f;          // 动画开始时间
  int loopCount = -1;              // 循环次数，-1表示无限循环，0表示不循环，>0表示循环次数
  int currentLoop = 0;             // 当前循环次数
  float animationSpeed = 1.0f;     // 动画播放速度
  void handleAnimationComplete(std::shared_ptr<Gltf> gltf,
                               const GltfAnimationTarget &target);

  std::chrono::steady_clock::time_point gameStartTime;

  float getCurrentTime();

};

} // namespace digitalhumans


#endif //LIGHTDIGITALHUMAN_GLTFANIMATION_H
