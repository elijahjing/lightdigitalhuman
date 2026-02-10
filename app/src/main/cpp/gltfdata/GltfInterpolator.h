//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFINTERPOLATOR_H
#define LIGHTDIGITALHUMAN_GLTFINTERPOLATOR_H

#include <vector>
#include <memory>
#include <array>
#include <cstddef>
#include "GltfAnimationSampler.h"
#include "GltfAnimationChannel.h"

namespace digitalhumans {


/**
 * @brief glTF动画插值器
 */
class GltfInterpolator {
 public:
  /**
   * @brief 构造函数
   */
  GltfInterpolator();

  /**
   * @brief 析构函数
   */
  ~GltfInterpolator() = default;

  /**
   * @brief 四元数球面线性插值
   * @param q1 起始四元数
   * @param q2 结束四元数
   * @param t 插值参数 [0, 1]
   * @return 插值结果四元数
   */
  std::array<float, 4> slerpQuat(const std::array<float, 4> &q1,
                                 const std::array<float, 4> &q2,
                                 float t);

  /**
   * @brief 阶跃插值
   * @param prevKey 前一个关键帧索引
   * @param output 输出数据数组
   * @param stride 每个关键帧的组件数量
   * @return 插值结果
   */
  std::vector<float>
  step(int prevKey, const std::vector<float> &output, int stride);

  /**
   * @brief 线性插值
   * @param prevKey 前一个关键帧索引
   * @param nextKey 后一个关键帧索引
   * @param output 输出数据数组
   * @param t 插值参数 [0, 1]
   * @param stride 每个关键帧的组件数量
   * @return 插值结果
   */
  std::vector<float>
  linear(int prevKey, int nextKey, const std::vector<float> &output,
         float t, int stride);

  /**
   * @brief 三次样条插值
   * @param prevKey 前一个关键帧索引
   * @param nextKey 后一个关键帧索引
   * @param output 输出数据数组
   * @param keyDelta 关键帧时间差
   * @param t 插值参数 [0, 1]
   * @param stride 每个关键帧的组件数量
   * @return 插值结果
   */
  std::vector<float>
  cubicSpline(int prevKey, int nextKey, const std::vector<float> &output,
              float keyDelta, float t, int stride);

  /**
   * @brief 重置关键帧索引
   */
  void resetKey();

  /**
   * @brief 执行插值计算
   * @param gltf glTF根对象
   * @param channel 动画通道
   * @param sampler 动画采样器
   * @param t 当前时间
   * @param stride 每个关键帧的组件数量
   * @param maxTime 动画最大时间
   * @return 插值结果，如果无效则返回空vector
   */
  std::vector<float> interpolate(std::shared_ptr<Gltf> gltf,
                                 std::shared_ptr<GltfAnimationChannel> channel,
                                 std::shared_ptr<GltfAnimationSampler> sampler,
                                 float t, int stride, float maxTime);

  /**
   * @brief 从输出数组中获取四元数
   * @param output 输出数据数组
   * @param index 关键帧索引
   * @return 四元数
   */
  std::array<float, 4> getQuat(const std::vector<float> &output, int index);

 private:
  /**
   * @brief 四元数归一化
   * @param quat 输入四元数
   * @return 归一化后的四元数
   */
  std::array<float, 4> normalizeQuat(const std::array<float, 4> &quat);

  /**
   * @brief 计算四元数点积
   * @param q1 四元数1
   * @param q2 四元数2
   * @return 点积结果
   */
  float quatDot(const std::array<float, 4> &q1, const std::array<float, 4> &q2);

  /**
   * @brief 限制数值范围
   * @param value 输入值
   * @param min 最小值
   * @param max 最大值
   * @return 限制后的值
   */
  float clamp(float value, float min, float max);

  /**
   * @brief 限制整数范围
   * @param value 输入值
   * @param min 最小值
   * @param max 最大值
   * @return 限制后的值
   */
  int clamp(int value, int min, int max);

 private:
  int prevKey;    ///< 前一个关键帧索引
  float prevT;    ///< 前一个时间值
};

} // namespace digitalhumans

#endif //LIGHTDIGITALHUMAN_GLTFINTERPOLATOR_H
