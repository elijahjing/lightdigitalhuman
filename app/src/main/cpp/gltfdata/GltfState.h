//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFSTATE_H
#define LIGHTDIGITALHUMAN_GLTFSTATE_H


#include "../utils/utils.h"
#include "UserCamera.h"
#include "GltfEnvironment.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace digitalhumans {
/**
* @brief 色调映射枚举
*/
enum class ToneMap {
  KHR_PBR_NEUTRAL,           ///< Khronos PBR neutral tone mapping
  ACES_HILL_EXPOSURE_BOOST,  ///< ACES sRGB RRT+ODT implementation with exposure boost
  ACES_NARKOWICZ,           ///< fast implementation of ACES sRGB RRT+ODT
  ACES_HILL,                ///< accurate implementation of ACES sRGB RRT+ODT
  NONE                      ///< Linear mapping, clamped at 1.0
};

using AnimationEntry = std::pair<int, int>;  // first=index, second=loopCount
/**
 * @brief 调试输出枚举
 */
enum class DebugOutput {
  // 标准渲染
  NONE,

  // 通用调试输出
  UV_COORDS_0,
  UV_COORDS_1,
  NORMAL,
  GEOMETRYNORMAL,
  TANGENT,
  BITANGENT,
  WORLDSPACENORMAL,
  ALPHA,
  OCCLUSION,
  EMISSIVE,

  // 金属粗糙度
  BASECOLOR,
  METALLIC,
  ROUGHNESS,

  // KHR_materials_clearcoat
  CLEARCOAT_FACTOR,
  CLEARCOAT_ROUGHNESS,
  CLEARCOAT_NORMAL,

  // KHR_materials_sheen
  SHEEN_COLOR,
  SHEEN_ROUGHNESS,

  // KHR_materials_specular
  SPECULAR_FACTOR,
  SPECULAR_COLOR,

  // KHR_materials_transmission
  TRANSMISSION_FACTOR,
  VOLUME_THICKNESS,

  // KHR_materials_diffuse_transmission
  DIFFUSE_TRANSMISSION_FACTOR,
  DIFFUSE_TRANSMISSION_COLOR_FACTOR,

  // KHR_materials_iridescence
  IRIDESCENCE_FACTOR,
  IRIDESCENCE_THICKNESS,

  // KHR_materials_anisotropy
  ANISOTROPIC_STRENGTH,
  ANISOTROPIC_DIRECTION
};

/**
 * @brief 扩展启用状态
 */
struct ExtensionSettings {
  bool KHR_materials_clearcoat = true;
  bool KHR_materials_sheen = true;
  bool KHR_materials_transmission = true;
  bool KHR_materials_volume = true;
  bool KHR_materials_ior = true;
  bool KHR_materials_specular = true;
  bool KHR_materials_iridescence = true;
  bool KHR_materials_diffuse_transmission = true;
  bool KHR_materials_anisotropy = true;
  bool KHR_materials_dispersion = true;
  bool KHR_materials_emissive_strength = true;
};

/**
 * @brief 渲染参数配置
 */
struct RenderingParameters {
  bool morphing = true;                           ///< 顶点变形
  bool skinning = true;                           ///< 骨骼/蒙皮
  ExtensionSettings enabledExtensions;            ///< 启用的扩展
  glm::vec4 clearColor = {1.0f, 1.0f, 1.0f, 0.0f};  // RGBA: 白色完全透明

  // glm::vec4 clearColor = {58.0f/255.0f, 64.0f/255.0f, 74.0f/255.0f, 1.0f}; // [0, 1] 范围
  float exposure = 1.0f;                          ///< 曝光因子
  bool usePunctual = true;                        ///< 使用点光源
  bool useIBL = true;                             ///< 使用基于图像的光照
  float iblIntensity = 1.0f;                      ///< IBL强度
  bool renderEnvironmentMap = true;               ///< 渲染环境贴图
  bool blurEnvironmentMap = false;                 ///< 模糊环境贴图
  ToneMap toneMap = ToneMap::NONE;    ///< 色调映射
  DebugOutput debugOutput = DebugOutput::NONE;    ///< 调试输出
  float environmentRotation = 90.0f;              ///< 环境旋转角度
  bool useDirectionalLightsWithDisabledIBL = true; ///< IBL禁用时使用方向光
  int internalMSAA = 4;                          ///< 内部MSAA
};

/**
 * @brief GltfState类
 * 包含在GltfView中用于可视化的状态
 *
 * GltfState表示可以在视图中可视化的所有状态。您可以配置多个GltfState并按需切换它们。
 */
class GltfState {
 public:
  /**
   * @brief 构造函数
   * @param view 此状态所属的GltfView
   */
  GltfState();

  /**
   * @brief 析构函数
   */
  ~GltfState() = default;

  // === 状态数据访问器 ===

  /**
   * @brief 获取加载的glTF数据
   * @return glTF数据指针
   */
  std::shared_ptr<Gltf> getGltf() const { return gltf; }

  /**
   * @brief 设置glTF数据
   * @param gltf glTF数据指针
   */
  void setGltf(std::shared_ptr<Gltf> gltf) {
    this->gltf = gltf;
    if (environment) {
      environment->setGltf(gltf);
    }
  }

  /**
   * @brief 获取加载的环境数据
   * @return 环境数据指针
   */
  std::shared_ptr<GltfEnvironment>
  getEnvironment() const { return environment; }

  /**
   * @brief 设置环境数据
   * @param environment 环境数据指针
   */
  void setEnvironment(std::shared_ptr<GltfEnvironment> environment) {
    this->environment = environment;
  }

  /**
   * @brief 获取用户摄像机
   * @return 用户摄像机引用
   */
  std::shared_ptr<UserCamera> getUserCamera() { return userCamera; }

  void setUserCamera(const std::shared_ptr<UserCamera> &userCamera);

  /**
   * @brief 获取场景索引
   * @return 场景索引
   */
  int getSceneIndex() const { return sceneIndex; }

  /**
   * @brief 设置场景索引
   * @param sceneIndex 场景索引
   */
  void setSceneIndex(int sceneIndex) { this->sceneIndex = sceneIndex; }

  /**
   * @brief 获取摄像机节点索引
   * @return 摄像机节点索引（可选）
   */
  std::optional<int> getCameraNodeIndex() const { return cameraNodeIndex; }

  /**
   * @brief 设置摄像机节点索引
   * @param cameraNodeIndex 摄像机节点索引
   */
  void setCameraNodeIndex(std::optional<int> cameraNodeIndex) {
    this->cameraNodeIndex = cameraNodeIndex;
  }

  /**
   * @brief 获取活动动画索引
   * @return 动画索引数组
   */
  const std::vector<AnimationEntry> &
  getAnimationIndices() const { return animationIndices; }

  /**
   * @brief 设置活动动画索引
   * @param animationIndices 动画索引数组
   */
  void
  setAnimationIndices(const std::vector<AnimationEntry> &animationIndices) {
    this->animationIndices =
        animationIndices;
  }

  /**
   * @brief 获取动画计时器
   * @return 动画计时器引用
   */
  utils::AnimationTimer &getAnimationTimer() { return animationTimer; }
  const utils::AnimationTimer &
  getAnimationTimer() const { return animationTimer; }

  /**
   * @brief 获取材质变体
   * @return 材质变体（可选）
   */
  std::optional<std::string> getVariant() const { return variant; }

  /**
   * @brief 设置材质变体
   * @param variant 材质变体名称
   */
  void setVariant(std::optional<std::string> variant) {
    this->variant = variant;
  }

  /**
   * @brief 获取渲染参数
   * @return 渲染参数引用
   */
  RenderingParameters &getRenderingParameters() { return renderingParameters; }
  const RenderingParameters &
  getRenderingParameters() const { return renderingParameters; }



  // === 工具方法 ===

  /**
   * @brief 添加动画索引
   * @param animationIndex 动画索引
   */
  void addAnimationIndex(int animationIndex);

  /**
   * @brief 移除动画索引
   * @param animationIndex 动画索引
   */
  void removeAnimationIndex(int animationIndex);

  /**
   * @brief 清除所有动画索引
   */
  void clearAnimationIndices();

  /**
   * @brief 检查动画是否活动
   * @param animationIndex 动画索引
   * @return true如果动画活动
   */
  bool isAnimationActive(int animationIndex) const;

  // === 静态工具方法 ===

  /**
   * @brief 色调映射枚举到字符串
   * @param toneMap 色调映射枚举
   * @return 字符串描述
   */
  static std::string toneMapToString(ToneMap toneMap);

  /**
   * @brief 字符串到色调映射枚举
   * @param str 字符串描述
   * @return 色调映射枚举
   */
  static ToneMap stringToToneMap(const std::string &str);

  /**
   * @brief 调试输出枚举到字符串
   * @param debugOutput 调试输出枚举
   * @return 字符串描述
   */
  static std::string debugOutputToString(DebugOutput debugOutput);

  /**
   * @brief 字符串到调试输出枚举
   * @param str 字符串描述
   * @return 调试输出枚举
   */
  static DebugOutput stringToDebugOutput(const std::string &str);

  void addAnimationIndex(int animationIndex, int loopCount);


 private:
  // === 核心状态数据 ===
  std::shared_ptr<Gltf> gltf;                     ///< 加载的glTF数据
  std::shared_ptr<GltfEnvironment> environment;        ///< 加载的环境数据

  std::shared_ptr<UserCamera> userCamera;         ///< 用户摄像机
  int sceneIndex;                                 ///< 可见的glTF场景
  std::optional<int> cameraNodeIndex = {-1};             ///< 渲染视图的摄像机节点索引
  std::vector<AnimationEntry> animationIndices;              ///< 活动动画索引
  utils::AnimationTimer animationTimer;           ///< 动画计时器
  std::optional<std::string> variant;             ///< KHR_materials_variants

  // === 渲染配置 ===
  RenderingParameters renderingParameters;        ///< 渲染参数配置

};

} // namespace digitalhumans

#endif //LIGHTDIGITALHUMAN_GLTFSTATE_H
