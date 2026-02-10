//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_UNIFORMSTRUCT_H
#define LIGHTDIGITALHUMAN_UNIFORMSTRUCT_H

#include <string>
#include <unordered_map>
#include <vector>
#include <variant>
#include <memory>
#include "glm.hpp"
#include "UniformTypes.h"

namespace digitalhumans {


/**
 * @brief 表示 GLSL 中的结构体 uniform
 *
 * 例如 GLSL 中的：
 * struct Material {
 *     vec3 baseColorFactor;
 *     float metallicFactor;
 *     float roughnessFactor;
 * };
 * uniform Material u_Material;
 */
class UniformStruct {
 public:
  UniformStruct() = default;
  virtual ~UniformStruct() = default;

  /**
   * @brief 设置成员变量的值
   * @param memberName 成员名称
   * @param value 成员值
   */
  void setMember(const std::string &memberName, const UniformValue &value);

  /**
   * @brief 获取成员变量的值
   * @param memberName 成员名称
   * @return 成员值，如果不存在则返回默认值
   */
  UniformValue getMemberValue(const std::string &memberName) const;

  /**
   * @brief 检查是否包含某个成员
   * @param memberName 成员名称
   * @return 是否存在
   */
  bool hasMember(const std::string &memberName) const;

  /**
   * @brief 获取所有成员名称
   * @return 成员名称列表
   */
  std::vector<std::string> getMemberNames() const;

  /**
   * @brief 获取成员数量
   * @return 成员数量
   */
  size_t getMemberCount() const;

  /**
   * @brief 清空所有成员
   */
  void clear();

  /**
   * @brief 移除指定成员
   * @param memberName 成员名称
   */
  void removeMember(const std::string &memberName);

  // 便利的类型特化设置方法
  void setBool(const std::string &memberName, bool value);
  void setInt(const std::string &memberName, int value);
  void setFloat(const std::string &memberName, float value);
  void setVec2(const std::string &memberName, const glm::vec2 &value);
  void setVec3(const std::string &memberName, const glm::vec3 &value);
  void setVec4(const std::string &memberName, const glm::vec4 &value);
  void setMat3(const std::string &memberName, const glm::mat3 &value);
  void setMat4(const std::string &memberName, const glm::mat4 &value);

  // 便利的类型特化获取方法
  bool getBool(const std::string &memberName, bool defaultValue = false) const;
  int getInt(const std::string &memberName, int defaultValue = 0) const;
  float
  getFloat(const std::string &memberName, float defaultValue = 0.0f) const;
  glm::vec2 getVec2(const std::string &memberName,
                    const glm::vec2 &defaultValue = glm::vec2(0.0f)) const;
  glm::vec3 getVec3(const std::string &memberName,
                    const glm::vec3 &defaultValue = glm::vec3(0.0f)) const;
  glm::vec4 getVec4(const std::string &memberName,
                    const glm::vec4 &defaultValue = glm::vec4(0.0f)) const;
  glm::mat3 getMat3(const std::string &memberName,
                    const glm::mat3 &defaultValue = glm::mat3(1.0f)) const;
  glm::mat4 getMat4(const std::string &memberName,
                    const glm::mat4 &defaultValue = glm::mat4(1.0f)) const;

  /**
   * @brief 调试用途，打印所有成员
   */
  std::string toString() const;

 protected:
  std::unordered_map<std::string, UniformValue> members;
};

/**
 * @brief 材质 uniform 结构体
 */
class MaterialUniformStruct: public UniformStruct {
 public:
  MaterialUniformStruct();

  // 材质特定的便利方法
  void setBaseColorFactor(const glm::vec4 &color);
  void setMetallicFactor(float metallic);
  void setRoughnessFactor(float roughness);
  void setEmissiveFactor(const glm::vec3 &emissive);
  void setNormalScale(float scale);
  void setOcclusionStrength(float strength);
  void setAlphaCutoff(float cutoff);

  glm::vec4 getBaseColorFactor() const;
  float getMetallicFactor() const;
  float getRoughnessFactor() const;
  glm::vec3 getEmissiveFactor() const;
  float getNormalScale() const;
  float getOcclusionStrength() const;
  float getAlphaCutoff() const;
};

/**
 * @brief 光照 uniform 结构体
 */
class LightUniformStruct: public UniformStruct {
 public:
  LightUniformStruct();

  // 光照特定的便利方法
  void setColor(const glm::vec3 &color);
  void setIntensity(float intensity);
  void setPosition(const glm::vec3 &position);
  void setDirection(const glm::vec3 &direction);
  void setRange(float range);
  void setInnerConeCos(float innerConeCos);
  void setOuterConeCos(float outerConeCos);
  void setType(int type);

  glm::vec3 getColor() const;
  float getIntensity() const;
  glm::vec3 getPosition() const;
  glm::vec3 getDirection() const;
  float getRange() const;
  float getInnerConeCos() const;
  float getOuterConeCos() const;
  int getType() const;
};

} // namespace digitalhumans


#endif //LIGHTDIGITALHUMAN_UNIFORMSTRUCT_H
