//
// Created by vincentsyan on 2025/8/18.
//

#include "UniformStruct.h"
#include <sstream>
#include <algorithm>
#include "UniformTypes.h"


namespace digitalhumans {

void UniformStruct::setMember(const std::string &memberName,
                              const UniformValue &value) {
  members[memberName] = value;
}

UniformValue
UniformStruct::getMemberValue(const std::string &memberName) const {
  auto it = members.find(memberName);
  if (it != members.end()) {
    return it->second;
  }
  // 返回默认值 (int 0)
  return UniformValue(0);
}

bool UniformStruct::hasMember(const std::string &memberName) const {
  return members.find(memberName) != members.end();
}

std::vector<std::string> UniformStruct::getMemberNames() const {
  std::vector<std::string> names;
  names.reserve(members.size());

  for (const auto &pair: members) {
    names.push_back(pair.first);
  }

  // 可选：排序以确保一致的顺序
  std::sort(names.begin(), names.end());

  return names;
}

size_t UniformStruct::getMemberCount() const {
  return members.size();
}

void UniformStruct::clear() {
  members.clear();
}

void UniformStruct::removeMember(const std::string &memberName) {
  members.erase(memberName);
}

// 便利设置方法
void UniformStruct::setBool(const std::string &memberName, bool value) {
  setMember(memberName, value);
}

void UniformStruct::setInt(const std::string &memberName, int value) {
  setMember(memberName, value);
}

void UniformStruct::setFloat(const std::string &memberName, float value) {
  setMember(memberName, value);
}

void
UniformStruct::setVec2(const std::string &memberName, const glm::vec2 &value) {
  setMember(memberName, value);
}

void
UniformStruct::setVec3(const std::string &memberName, const glm::vec3 &value) {
  setMember(memberName, value);
}

void
UniformStruct::setVec4(const std::string &memberName, const glm::vec4 &value) {
  setMember(memberName, value);
}

void
UniformStruct::setMat3(const std::string &memberName, const glm::mat3 &value) {
  setMember(memberName, value);
}

void
UniformStruct::setMat4(const std::string &memberName, const glm::mat4 &value) {
  setMember(memberName, value);
}

// 便利获取方法
bool
UniformStruct::getBool(const std::string &memberName, bool defaultValue) const {
  auto it = members.find(memberName);
  if (it != members.end()) {
    if (auto value = std::get_if<bool>(&it->second)) {
      return *value;
    }
  }
  return defaultValue;  // 直接返回默认值，避免类型转换问题
}

int
UniformStruct::getInt(const std::string &memberName, int defaultValue) const {
  auto it = members.find(memberName);
  if (it != members.end()) {
    if (auto value = std::get_if<int>(&it->second)) {
      return *value;
    }
  }
  return defaultValue;
}

float UniformStruct::getFloat(const std::string &memberName,
                              float defaultValue) const {
  auto it = members.find(memberName);
  if (it != members.end()) {
    if (auto value = std::get_if<float>(&it->second)) {
      return *value;
    }
  }
  return defaultValue;
}

glm::vec2 UniformStruct::getVec2(const std::string &memberName,
                                 const glm::vec2 &defaultValue) const {
  auto it = members.find(memberName);
  if (it != members.end()) {
    if (auto value = std::get_if<glm::vec2>(&it->second)) {
      return *value;
    }
  }
  return defaultValue;
}

glm::vec3 UniformStruct::getVec3(const std::string &memberName,
                                 const glm::vec3 &defaultValue) const {
  auto it = members.find(memberName);
  if (it != members.end()) {
    if (auto value = std::get_if<glm::vec3>(&it->second)) {
      return *value;
    }
  }
  return defaultValue;
}

glm::vec4 UniformStruct::getVec4(const std::string &memberName,
                                 const glm::vec4 &defaultValue) const {
  auto it = members.find(memberName);
  if (it != members.end()) {
    if (auto value = std::get_if<glm::vec4>(&it->second)) {
      return *value;
    }
  }
  return defaultValue;
}

glm::mat3 UniformStruct::getMat3(const std::string &memberName,
                                 const glm::mat3 &defaultValue) const {
  auto it = members.find(memberName);
  if (it != members.end()) {
    if (auto value = std::get_if<glm::mat3>(&it->second)) {
      return *value;
    }
  }
  return defaultValue;
}

glm::mat4 UniformStruct::getMat4(const std::string &memberName,
                                 const glm::mat4 &defaultValue) const {
  auto it = members.find(memberName);
  if (it != members.end()) {
    if (auto value = std::get_if<glm::mat4>(&it->second)) {
      return *value;
    }
  }
  return defaultValue;
}

std::string UniformStruct::toString() const {
  std::ostringstream oss;
  oss << "UniformStruct{";

  bool first = true;
  for (const auto &pair: members) {
    if (!first) oss << ", ";
    first = false;

    oss << pair.first << ": ";

    // 这里可以添加类型特定的字符串转换
    std::visit([&oss](const auto &value) {
      using T = std::decay_t<decltype(value)>;
      if constexpr (std::is_same_v<T, int>) {
        oss << value;
      } else if constexpr (std::is_same_v<T, float>) {
        oss << value;
      } else if constexpr (std::is_same_v<T, bool>) {
        oss << (value ? "true" : "false");
      } else {
        oss << "[complex_type]";
      }
    }, pair.second);
  }

  oss << "}";
  return oss.str();
}

// MaterialUniformStruct 实现
MaterialUniformStruct::MaterialUniformStruct() {
  // 设置默认值
  setBaseColorFactor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
  setMetallicFactor(1.0f);
  setRoughnessFactor(1.0f);
  setEmissiveFactor(glm::vec3(0.0f, 0.0f, 0.0f));
  setNormalScale(1.0f);
  setOcclusionStrength(1.0f);
  setAlphaCutoff(0.5f);
}

void MaterialUniformStruct::setBaseColorFactor(const glm::vec4 &color) {
  setVec4("baseColorFactor", color);
}

void MaterialUniformStruct::setMetallicFactor(float metallic) {
  setFloat("metallicFactor", metallic);
}

void MaterialUniformStruct::setRoughnessFactor(float roughness) {
  setFloat("roughnessFactor", roughness);
}

void MaterialUniformStruct::setEmissiveFactor(const glm::vec3 &emissive) {
  setVec3("emissiveFactor", emissive);
}

void MaterialUniformStruct::setNormalScale(float scale) {
  setFloat("normalScale", scale);
}

void MaterialUniformStruct::setOcclusionStrength(float strength) {
  setFloat("occlusionStrength", strength);
}

void MaterialUniformStruct::setAlphaCutoff(float cutoff) {
  setFloat("alphaCutoff", cutoff);
}

glm::vec4 MaterialUniformStruct::getBaseColorFactor() const {
  return getVec4("baseColorFactor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
}

float MaterialUniformStruct::getMetallicFactor() const {
  return getFloat("metallicFactor", 1.0f);
}

float MaterialUniformStruct::getRoughnessFactor() const {
  return getFloat("roughnessFactor", 1.0f);
}

glm::vec3 MaterialUniformStruct::getEmissiveFactor() const {
  return getVec3("emissiveFactor", glm::vec3(0.0f));
}

float MaterialUniformStruct::getNormalScale() const {
  return getFloat("normalScale", 1.0f);
}

float MaterialUniformStruct::getOcclusionStrength() const {
  return getFloat("occlusionStrength", 1.0f);
}

float MaterialUniformStruct::getAlphaCutoff() const {
  return getFloat("alphaCutoff", 0.5f);
}

} // namespace digitalhumans