//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_UTILS_H
#define LIGHTDIGITALHUMAN_UTILS_H


#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <chrono>
#include <any>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include "glm.hpp"
#include "gtc/quaternion.hpp"

namespace digitalhumans {
class GltfOpenGLContext;
class Gltf;
class GltfObject;

namespace utils {

/**
 * @brief 将JavaScript数组转换为GLM类型
 * @tparam T GLM类型（vec3, vec4, quat, mat4等）
 * @param array 浮点数数组
 * @return GLM类型对象
 */
template<typename T>
T jsToGl(const std::vector<float> &array);

/**
 * @brief 将JavaScript数组的一部分转换为GLM类型
 * @tparam T GLM类型
 * @param array 浮点数数组
 * @param offset 起始偏移
 * @param stride 元素数量
 * @return GLM类型对象
 */
template<typename T>
T jsToGlSlice(const std::vector<float> &array, size_t offset, size_t stride);

/**
 * @brief 为对象成员初始化WebGL资源
 * @param gltfObj glTF对象
 * @param gltf glTF根对象
 * @param webGlContext WebGL上下文
 */
void initGlForMembers(GltfObject &gltfObj,
                      std::shared_ptr<Gltf> gltf,
                      std::shared_ptr<GltfOpenGLContext> webGlContext);

/**
 * @brief 从JSON对象数组创建glTF对象数组
 * @tparam GltfType glTF对象类型
 * @param jsonObjects JSON对象数组
 * @return glTF对象数组
 */
template<typename GltfType>
std::vector<std::shared_ptr<GltfType>> objectsFromJsons(
    const std::vector<std::any> &jsonObjects);

/**
 * @brief 从JSON对象创建glTF对象
 * @tparam GltfType glTF对象类型
 * @param jsonObject JSON对象
 * @return glTF对象指针
 */
template<typename GltfType>
std::shared_ptr<GltfType> objectFromJson(const std::any &jsonObject);

/**
 * @brief 从JSON对象的键值对复制到目标对象
 * @param target 目标对象
 * @param jsonObj JSON对象
 * @param ignore 要忽略的键列表
 */
void fromKeys(std::any &target,
              const std::unordered_map<std::string, std::any> &jsonObj,
              const std::vector<std::string> &ignore = {});

/**
 * @brief 从JSON对象复制指定参数到目标对象
 * @param parameters 参数名列表
 * @param target 目标对象
 * @param jsonObj JSON对象
 */
void fromParams(const std::vector<std::string> &parameters,
                std::any &target,
                const std::unordered_map<std::string, std::any> &jsonObj);

/**
 * @brief 计算字符串哈希值
 * @param str 输入字符串
 * @param seed 种子值
 * @return 哈希值
 */
int32_t stringHash(const std::string &str, int32_t seed = 0);

/**
 * @brief 将数值限制在指定范围内
 * @param number 输入数值
 * @param min 最小值
 * @param max 最大值
 * @return 限制后的数值
 */
template<typename T>
T clamp(T number, T min, T max);

/**
 * @brief 检查文件是否为GLB格式
 * @param filename 文件名
 * @return true如果是GLB文件
 */
bool getIsGlb(const std::string &filename);

/**
 * @brief 检查文件是否为glTF格式
 * @param filename 文件名
 * @return true如果是glTF文件
 */
bool getIsGltf(const std::string &filename);

/**
 * @brief 检查文件是否为HDR格式
 * @param filename 文件名
 * @return true如果是HDR文件
 */
bool getIsHdr(const std::string &filename);

/**
 * @brief 获取文件扩展名
 * @param filename 文件名
 * @return 扩展名（不包含点）
 */
std::optional<std::string> getExtension(const std::string &filename);

/**
 * @brief 从文件路径获取文件名
 * @param filePath 文件路径
 * @return 文件名
 */
std::string getFileName(const std::string &filePath);

/**
 * @brief 获取不包含扩展名的文件名
 * @param filePath 文件路径
 * @return 不包含扩展名的文件名
 */
std::string getFileNameWithoutExtension(const std::string &filePath);

/**
 * @brief 获取包含文件的文件夹路径
 * @param filePath 文件路径
 * @return 文件夹路径
 */
std::string getContainingFolder(const std::string &filePath);

/**
 * @brief 组合多个路径
 * @param paths 路径列表
 * @return 组合后的路径
 */
std::string combinePaths(const std::vector<std::string> &paths);


/**
 * @brief 定时器类
 * 用于测量代码执行时间
 */
class Timer {
 public:
  Timer();

  /**
   * @brief 开始计时
   */
  void start();

  /**
   * @brief 停止计时
   */
  void stop();

  /**
   * @brief 获取执行时间（秒）
   * @return 执行时间
   */
  double getSeconds() const { return seconds; }

 private:
  std::chrono::high_resolution_clock::time_point startTime;
  std::chrono::high_resolution_clock::time_point endTime;
  double seconds;
};

/**
 * @brief 动画定时器类
 * 用于动画系统的时间管理
 */
class AnimationTimer {
 public:
  AnimationTimer();

  /**
   * @brief 获取已过去的秒数
   * @return 已过去的时间（秒）
   */
  double elapsedSec();

  /**
   * @brief 切换暂停/继续状态
   */
  void toggle();

  /**
   * @brief 开始计时
   */
  void start();

  /**
   * @brief 暂停计时
   */
  void pause();

  /**
   * @brief 继续计时
   */
  void unpause();

  /**
   * @brief 重置计时器
   */
  void reset();

  /**
   * @brief 设置固定时间
   * @param timeInSec 时间（秒）
   */
  void setFixedTime(double timeInSec);

  /**
   * @brief 检查是否暂停
   * @return true如果暂停
   */
  bool isPaused() const { return paused; }

 private:
  std::chrono::high_resolution_clock::time_point startTimePoint;
  bool paused;
  std::optional<double> fixedTime;
  double pausedTime;
};

} // namespace utils
} // namespace digitalhumans


#endif //LIGHTDIGITALHUMAN_UTILS_H
