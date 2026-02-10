//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfLight.h"
#include "math_utils.h"
#include <android/log.h>
#include <cmath>
#include <algorithm>
#include "glm.hpp"
#include "gtc/quaternion.hpp"
#include "gtc/matrix_transform.hpp"
#include "GltfNode.h"
#include "../utils/LogUtils.h"


namespace digitalhumans {

// ===== GltfLightSpot实现 =====

GltfLightSpot::GltfLightSpot()
    : GltfObject(), innerConeAngle(0.0f),
      outerConeAngle(static_cast<float>(M_PI) / 4.0f) {
}


// ===== UniformLight实现 =====

UniformLight::UniformLight()
    : UniformStruct(), direction(glm::vec3(-0.7399f, -0.6428f, -0.1983f)),
      range(-1.0f),
      color(glm::vec3(1.0f, 1.0f, 1.0f)), intensity(1.0f),
      position(glm::vec3(0.0f, 0.0f, 0.0f)),
      innerConeCos(0.0f),
      outerConeCos(std::cos(static_cast<float>(M_PI) / 4.0f)),
      type(LightType::DIRECTIONAL) {
}

// ===== GltfLight实现 =====

GltfLight::GltfLight()
    : GltfObject(), name(), type(LightType::DIRECTIONAL),
      color(glm::vec3(1.0f, 1.0f, 1.0f)), intensity(1.0f),
      range(-1.0f), spot(std::make_shared<GltfLightSpot>()), direction() {
}


UniformLight GltfLight::toUniform(std::shared_ptr<GltfNode> node) const {
  UniformLight uLight;

  // 获取世界变换矩阵
  glm::mat4 worldMatrix(1.0f);
  if (node && node->hasWorldTransform()) {
    worldMatrix = node->getWorldTransform();
  }

  // 提取缩放分量
  glm::vec3 scale = glm::vec3(
      glm::length(glm::vec3(worldMatrix[0])),
      glm::length(glm::vec3(worldMatrix[1])),
      glm::length(glm::vec3(worldMatrix[2]))
  );

  // 创建无缩放的旋转矩阵
  glm::mat3 rotationMatrix;
  rotationMatrix[0] = glm::vec3(worldMatrix[0]) / scale.x;
  rotationMatrix[1] = glm::vec3(worldMatrix[1]) / scale.y;
  rotationMatrix[2] = glm::vec3(worldMatrix[2]) / scale.z;

  // 从旋转矩阵提取四元数
  glm::quat rotation = glm::quat_cast(rotationMatrix);
  rotation = glm::normalize(rotation);

  // 计算灯光方向（沿着负Z轴）
  glm::vec3 alongNegativeZ(0.0f, 0.0f, -1.0f);
  uLight.direction = rotation * alongNegativeZ;

  // 提取位置
  uLight.position = glm::vec3(worldMatrix[3]);

  // 如果有覆盖方向，则使用覆盖方向
  if (direction.has_value()) {
    uLight.direction = direction.value();
  }

  // 设置其他属性
  uLight.range = range;
  uLight.color = color;
  uLight.intensity = intensity;

  // 设置聚光灯参数
  if (spot) {
    uLight.innerConeCos = std::cos(spot->getInnerConeAngle());
    uLight.outerConeCos = std::cos(spot->getOuterConeAngle());
  } else {
    uLight.innerConeCos = 0.0f;
    uLight.outerConeCos = std::cos(static_cast<float>(M_PI) / 4.0f);
  }

  // 设置灯光类型
  uLight.type = type;

  return uLight;
}

float GltfLight::getAttenuation(float distance) const {
  if (range <= 0.0f) {
    // 无限范围光源
    return 1.0f / (distance * distance + 1.0f);
  }

  if (distance >= range) {
    return 0.0f;
  }

  // 基于距离的线性衰减
  float attenuation = std::max(0.0f, 1.0f - (distance / range));
  return attenuation * attenuation;
}

float GltfLight::calculateIntensityAtPoint(const glm::vec3 &worldPosition,
                                           const glm::vec3 &lightPosition,
                                           const glm::vec3 &lightDirection) const {
  float finalIntensity = intensity;

  switch (type) {
    case LightType::DIRECTIONAL:
      // 方向光不受距离影响
      return finalIntensity;

    case LightType::POINT: {
      float distance = glm::length(worldPosition - lightPosition);
      return finalIntensity * getAttenuation(distance);
    }

    case LightType::SPOT: {
      float distance = glm::length(worldPosition - lightPosition);
      float distanceAttenuation = getAttenuation(distance);

      // 计算角度衰减
      glm::vec3 lightToPoint = glm::normalize(worldPosition - lightPosition);
      float cosAngle = glm::dot(lightDirection, lightToPoint);

      if (spot) {
        float outerCos = std::cos(spot->getOuterConeAngle());
        float innerCos = std::cos(spot->getInnerConeAngle());

        if (cosAngle < outerCos) {
          return 0.0f;  // 在聚光灯锥外
        }

        // 在内锥和外锥之间的平滑过渡
        float angularAttenuation = 1.0f;
        if (cosAngle < innerCos) {
          angularAttenuation = (cosAngle - outerCos) / (innerCos - outerCos);
        }

        return finalIntensity * distanceAttenuation * angularAttenuation;
      }

      return finalIntensity * distanceAttenuation;
    }

    default:
      return 0.0f;
  }
}

std::pair<glm::vec3, glm::vec3>
GltfLight::getInfluenceBounds(const glm::vec3 &lightPosition) const {
  switch (type) {
    case LightType::DIRECTIONAL:
      // 方向光影响无限范围
      return {
          glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX),
          glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX)
      };

    case LightType::POINT: {
      if (range <= 0.0f) {
        // 无限范围点光源
        return {
            glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX),
            glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX)
        };
      }
      // 球形影响范围
      glm::vec3 rangeVec(range, range, range);
      return {lightPosition - rangeVec, lightPosition + rangeVec};
    }

    case LightType::SPOT: {
      if (range <= 0.0f || !spot) {
        // 无限范围或无效聚光灯
        return {
            glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX),
            glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX)
        };
      }

      // 计算聚光灯锥形影响范围
      float outerAngle = spot->getOuterConeAngle();
      float coneRadius = range * std::tan(outerAngle);

      // 简化为包含锥形的AABB
      glm::vec3 extents(coneRadius, coneRadius, range);
      return {lightPosition - extents, lightPosition + extents};
    }

    default:
      return {lightPosition, lightPosition};
  }
}

void GltfLight::reset() {
  name.clear();
  type = LightType::DIRECTIONAL;
  color = glm::vec3(1.0f, 1.0f, 1.0f);
  intensity = 1.0f;
  range = -1.0f;
  spot = std::make_shared<GltfLightSpot>();
  direction.reset();
}

// 静态创建方法实现
std::shared_ptr<GltfLight> GltfLight::createDirectional(
    const std::string &name,
    const glm::vec3 &direction,
    const glm::vec3 &color,
    float intensity) {
  auto light = std::make_shared<GltfLight>();
  light->setName(name);
  light->setType(LightType::DIRECTIONAL);
  light->setDirection(glm::normalize(direction));
  light->setColor(color);
  light->setIntensity(intensity);
  return light;
}

std::shared_ptr<GltfLight> GltfLight::createPoint(
    const std::string &name,
    const glm::vec3 &color,
    float intensity,
    float range) {
  auto light = std::make_shared<GltfLight>();
  light->setName(name);
  light->setType(LightType::POINT);
  light->setColor(color);
  light->setIntensity(intensity);
  light->setRange(range);
  return light;
}

std::shared_ptr<GltfLight> GltfLight::createSpot(
    const std::string &name,
    const glm::vec3 &direction,
    float innerConeAngle,
    float outerConeAngle,
    const glm::vec3 &color,
    float intensity,
    float range) {
  auto light = std::make_shared<GltfLight>();
  light->setName(name);
  light->setType(LightType::SPOT);
  light->setDirection(glm::normalize(direction));
  light->setColor(color);
  light->setIntensity(intensity);
  light->setRange(range);

  auto spotLight = std::make_shared<GltfLightSpot>();
  spotLight->setInnerConeAngle(innerConeAngle);
  spotLight->setOuterConeAngle(outerConeAngle);
  light->setSpot(spotLight);

  return light;
}

} // namespace digitalhumans