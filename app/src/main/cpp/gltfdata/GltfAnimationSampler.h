//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFANIMATIONSAMPLER_H
#define LIGHTDIGITALHUMAN_GLTFANIMATIONSAMPLER_H

#include "GltfObject.h"
#include <string>
#include <memory>
#include <optional>
#include <vector>

namespace digitalhumans {

/**
 * @brief 插值模式枚举
 */
enum class InterpolationMode {
  LINEAR,         ///< 线性插值
  STEP,           ///< 阶跃插值（无插值）
  CUBICSPLINE,    ///< 三次样条插值
  UNKNOWN         ///< 未知模式
};

/**
 * @brief 插值模式工具类
 */
class InterpolationModeUtils {
 public:
  /**
   * @brief 从字符串获取插值模式枚举
   * @param modeString 模式字符串
   * @return 插值模式枚举值
   */
  static InterpolationMode fromString(const std::string &modeString);

  /**
   * @brief 从插值模式枚举获取字符串
   * @param mode 插值模式枚举值
   * @return 模式字符串
   */
  static std::string toString(InterpolationMode mode);

  /**
   * @brief 检查模式是否有效
   * @param mode 插值模式枚举值
   * @return 如果模式有效返回true
   */
  static bool isValid(InterpolationMode mode);

  /**
   * @brief 获取所有支持的模式字符串
   * @return 包含所有模式字符串的vector
   */
  static std::vector<std::string> getAllModeStrings();

  /**
   * @brief 获取默认插值模式
   * @return 默认插值模式（线性插值）
   */
  static InterpolationMode getDefault() { return InterpolationMode::LINEAR; }

  // 模式字符串常量
  static const std::string LINEAR_STR;
  static const std::string STEP_STR;
  static const std::string CUBICSPLINE_STR;
};

/**
 * @brief glTF动画采样器类
 *
 * 动画采样器定义了如何从时间轴上的关键帧数据中采样和插值出中间值
 */
class GltfAnimationSampler: public GltfObject {
 public:
  /**
   * @brief 构造函数
   */
  GltfAnimationSampler();

  /**
   * @brief 析构函数
   */
  virtual ~GltfAnimationSampler() = default;


  /**
   * @brief 克隆对象
   * @return 克隆的对象
   */
  std::shared_ptr<GltfAnimationSampler> clone() const;

  /**
   * @brief 验证采样器数据的有效性
   * @param gltf glTF根对象
   * @return 如果数据有效返回true
   */
  bool validate(std::shared_ptr<class Gltf> gltf) const;

  /**
   * @brief 重置为默认值
   */
  void reset();

  /**
   * @brief 检查采样器是否完整（有输入和输出访问器）
   * @return 如果采样器完整返回true
   */
  bool isComplete() const;

  /**
   * @brief 获取采样器的时间范围
   * @param gltf glTF根对象
   * @param outMinTime 输出最小时间
   * @param outMaxTime 输出最大时间
   * @return 如果成功获取时间范围返回true
   */
  bool getTimeRange(std::shared_ptr<class Gltf> gltf,
                    float &outMinTime,
                    float &outMaxTime) const;

  /**
   * @brief 获取关键帧数量
   * @param gltf glTF根对象
   * @return 关键帧数量，如果无法确定则返回0
   */
  size_t getKeyFrameCount(std::shared_ptr<class Gltf> gltf) const;

  /**
   * @brief 检查输入和输出访问器是否兼容
   * @param gltf glTF根对象
   * @return 如果兼容返回true
   */
  bool areAccessorsCompatible(std::shared_ptr<class Gltf> gltf) const;

  // === Getter/Setter方法 ===

  /**
   * @brief 获取输入访问器索引（时间轴数据）
   * @return 输入访问器索引，如果未设置则返回std::nullopt
   */
  const std::optional<int> &getInput() const { return input; }

  /**
   * @brief 设置输入访问器索引
   * @param accessorIndex 访问器索引
   */
  void setInput(int accessorIndex) { input = accessorIndex; }

  /**
   * @brief 清除输入访问器索引
   */
  void clearInput() { input.reset(); }

  /**
   * @brief 检查是否有输入访问器
   * @return 如果有输入访问器返回true
   */
  bool hasInput() const { return input.has_value(); }

  /**
   * @brief 获取插值模式
   * @return 插值模式枚举值
   */
  InterpolationMode getInterpolation() const { return interpolation; }

  /**
   * @brief 设置插值模式
   * @param mode 插值模式枚举值
   */
  void setInterpolation(InterpolationMode mode) { interpolation = mode; }

  /**
   * @brief 从字符串设置插值模式
   * @param modeString 插值模式字符串
   */
  void setInterpolationFromString(const std::string &modeString);

  /**
   * @brief 获取插值模式字符串
   * @return 插值模式字符串
   */
  std::string getInterpolationString() const;

  /**
   * @brief 检查是否使用线性插值
   * @return 如果使用线性插值返回true
   */
  bool isLinearInterpolation() const {
    return interpolation == InterpolationMode::LINEAR;
  }

  /**
   * @brief 检查是否使用阶跃插值
   * @return 如果使用阶跃插值返回true
   */
  bool isStepInterpolation() const {
    return interpolation == InterpolationMode::STEP;
  }

  /**
   * @brief 检查是否使用三次样条插值
   * @return 如果使用三次样条插值返回true
   */
  bool isCubicSplineInterpolation() const {
    return interpolation == InterpolationMode::CUBICSPLINE;
  }

  /**
   * @brief 获取输出访问器索引（关键帧数据）
   * @return 输出访问器索引，如果未设置则返回std::nullopt
   */
  const std::optional<int> &getOutput() const { return output; }

  /**
   * @brief 设置输出访问器索引
   * @param accessorIndex 访问器索引
   */
  void setOutput(int accessorIndex) { output = accessorIndex; }

  /**
   * @brief 清除输出访问器索引
   */
  void clearOutput() { output.reset(); }

  /**
   * @brief 检查是否有输出访问器
   * @return 如果有输出访问器返回true
   */
  bool hasOutput() const { return output.has_value(); }

  /**
   * @brief 获取采样器描述信息
   * @return 描述信息字符串
   */
  std::string getDescription() const;

  /**
   * @brief 创建标准的线性插值采样器
   * @param inputAccessor 输入访问器索引
   * @param outputAccessor 输出访问器索引
   * @return 新创建的采样器
   */
  static std::shared_ptr<GltfAnimationSampler>
  createLinear(int inputAccessor, int outputAccessor);

  /**
   * @brief 创建阶跃插值采样器
   * @param inputAccessor 输入访问器索引
   * @param outputAccessor 输出访问器索引
   * @return 新创建的采样器
   */
  static std::shared_ptr<GltfAnimationSampler>
  createStep(int inputAccessor, int outputAccessor);

  /**
   * @brief 创建三次样条插值采样器
   * @param inputAccessor 输入访问器索引
   * @param outputAccessor 输出访问器索引
   * @return 新创建的采样器
   */
  static std::shared_ptr<GltfAnimationSampler>
  createCubicSpline(int inputAccessor, int outputAccessor);

 private:
  /**
   * @brief 验证访问器索引的有效性
   * @param gltf glTF根对象
   * @param accessorIndex 访问器索引
   * @param accessorName 访问器名称（用于错误日志）
   * @return 如果访问器有效返回true
   */
  bool validateAccessor(std::shared_ptr<class Gltf> gltf,
                        int accessorIndex,
                        const std::string &accessorName) const;

 private:
  std::optional<int> input;               ///< 输入访问器索引（时间轴）
  InterpolationMode interpolation;        ///< 插值模式
  std::optional<int> output;              ///< 输出访问器索引（关键帧数据）
};

} // namespace digitalhumans


#endif //LIGHTDIGITALHUMAN_GLTFANIMATIONSAMPLER_H
