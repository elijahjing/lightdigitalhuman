//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFLIGHT_H
#define LIGHTDIGITALHUMAN_GLTFLIGHT_H


#include "GltfObject.h"
#include "../utils/utils.h"
#include "UniformStruct.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "gtx/quaternion.hpp"
#include <string>
#include <memory>
#include <optional>

namespace digitalhumans {
class GltfNode;

/**
 * @brief 灯光类型枚举
 */
enum class LightType: int {
  DIRECTIONAL = 0,
  POINT = 1,
  SPOT = 2
};

/**
 * @brief 聚光灯参数
 */
class GltfLightSpot: public GltfObject {
 public:
  /**
   * @brief 构造函数
   */
  GltfLightSpot();

  /**
   * @brief 虚析构函数
   */
  virtual ~GltfLightSpot() = default;



  // === Getter/Setter方法 ===
  float getInnerConeAngle() const { return innerConeAngle; }

  void setInnerConeAngle(float angle) { innerConeAngle = angle; }

  float getOuterConeAngle() const { return outerConeAngle; }

  void setOuterConeAngle(float angle) { outerConeAngle = angle; }

 private:
  float innerConeAngle;  ///< 内锥角（弧度）
  float outerConeAngle;  ///< 外锥角（弧度）
};

/**
 * @brief 着色器Uniform灯光结构体
 */
class UniformLight: public UniformStruct {
 public:
  /**
   * @brief 构造函数
   */
  UniformLight();

  /**
   * @brief 虚析构函数
   */
  virtual ~UniformLight() = default;

  // === 公共成员变量 ===
  glm::vec3 direction;    ///< 灯光方向
  float range;            ///< 灯光范围（-1表示无限）
  glm::vec3 color;        ///< 灯光颜色
  float intensity;        ///< 灯光强度
  glm::vec3 position;     ///< 灯光位置
  float innerConeCos;     ///< 内锥角余弦值
  float outerConeCos;     ///< 外锥角余弦值
  LightType type;         ///< 灯光类型
};

/**
 * @brief glTF灯光类
 */
class GltfLight: public GltfObject {
 public:
  /**
   * @brief 构造函数
   */
  GltfLight();


  /**
   * @brief 虚析构函数
   */
  virtual ~GltfLight() = default;


  /**
   * @brief 转换为Uniform结构体
   * @param node 关联的节点（用于获取变换信息）
   * @return Uniform灯光结构体
   */
  UniformLight toUniform(std::shared_ptr<GltfNode> node = nullptr) const;


  // === Getter/Setter方法 ===
  const std::string &getName() const { return name; }

  void setName(const std::string &name) { this->name = name; }

  LightType getType() const { return type; }

  void setType(LightType type) { this->type = type; }

  const glm::vec3 &getColor() const { return color; }

  void setColor(const glm::vec3 &color) { this->color = color; }

  void setColor(float r, float g, float b) { this->color = glm::vec3(r, g, b); }

  float getIntensity() const { return intensity; }

  void setIntensity(float intensity) { this->intensity = intensity; }

  float getRange() const { return range; }

  void setRange(float range) { this->range = range; }

  std::shared_ptr<GltfLightSpot> getSpot() const { return spot; }

  void setSpot(std::shared_ptr<GltfLightSpot> spot) { this->spot = spot; }

  const std::optional<glm::vec3> &getDirection() const { return direction; }

  void setDirection(const glm::vec3 &dir) { direction = dir; }

  void setDirection(float x, float y, float z) {
    direction = glm::vec3(x, y, z);
  }

  void clearDirection() { direction.reset(); }

  bool hasDirection() const { return direction.has_value(); }

  /**
   * @brief 获取光源影响范围（AABB）
   * @param lightPosition 光源位置
   * @return 包围盒 {min, max}
   */
  std::pair<glm::vec3, glm::vec3>
  getInfluenceBounds(const glm::vec3 &lightPosition) const;

  /**
   * @brief 计算在给定点的光照强度
   * @param worldPosition 世界空间位置
   * @param lightPosition 光源世界空间位置
   * @param lightDirection 光源方向（世界空间）
   * @return 光照强度
   */
  float calculateIntensityAtPoint(const glm::vec3 &worldPosition,
                                  const glm::vec3 &lightPosition,
                                  const glm::vec3 &lightDirection = glm::vec3(0,
                                                                              0,
                                                                              -1)) const;

  /**
   * @brief 获取光源的衰减系数
   * @param distance 距离
   * @return 衰减系数 [0, 1]
   */
  float getAttenuation(float distance) const;

  void reset();
  std::shared_ptr<GltfLight> createDirectional(
      const std::string &name,
      const glm::vec3 &direction,
      const glm::vec3 &color,
      float intensity);

  std::shared_ptr<GltfLight> createPoint(
      const std::string &name,
      const glm::vec3 &color,
      float intensity,
      float range);

  std::shared_ptr<GltfLight> createSpot(
      const std::string &name,
      const glm::vec3 &direction,
      float innerConeAngle,
      float outerConeAngle,
      const glm::vec3 &color,
      float intensity,
      float range);

 private:
  // === 私有成员变量 ===
  std::string name;                        ///< 灯光名称
  LightType type;                          ///< 灯光类型
  glm::vec3 color;                         ///< 灯光颜色
  float intensity;                         ///< 灯光强度
  float range;                             ///< 灯光范围
  std::shared_ptr<GltfLightSpot> spot;     ///< 聚光灯参数
  std::optional<glm::vec3> direction;      ///< 覆盖方向（可选）

  // === 私有辅助方法 ===
  /**
   * @brief 从4x4矩阵提取四元数
   * @param matrix 4x4变换矩阵
   * @return 四元数
   */
  static glm::quat extractQuaternionFromMatrix(const glm::mat4 &matrix);

  /**
   * @brief 使用四元数变换向量
   * @param vector 输入向量
   * @param quaternion 四元数
   * @return 变换后的向量
   */
  static glm::vec3 transformVector3ByQuaternion(const glm::vec3 &vector,
                                                const glm::quat &quaternion);
};

} // namespace digitalhumans

#endif //LIGHTDIGITALHUMAN_GLTFLIGHT_H