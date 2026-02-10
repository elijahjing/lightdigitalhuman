//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_USERCAMERA_H
#define LIGHTDIGITALHUMAN_USERCAMERA_H

#include "mat4x4.hpp"
#include "vec3.hpp"
#include "gtc/quaternion.hpp"
#include "GltfCamera.h"

namespace digitalhumans {

// 常量定义
constexpr float PAN_SPEED_DENOMINATOR = 3500.0f;
constexpr float MAX_NEAR_FAR_RATIO = 10000.0f;

/**
 * @brief 场景范围结构体
 */
struct SceneExtents {
  glm::vec3 min;    ///< 最小值
  glm::vec3 max;    ///< 最大值

  SceneExtents() : min(0.0f), max(0.0f) {}

  SceneExtents(const glm::vec3 &min, const glm::vec3 &max)
      : min(min), max(max) {}
};

/**
 * @brief 用户摄像机类
 * 继承自GltfCamera，提供便捷的摄像机控制功能
 */
class UserCamera: public GltfCamera {
 public:
  /**
   * @brief 构造函数
   * 创建一个新的用户摄像机
   */
  UserCamera();

  /**
   * @brief 虚析构函数
   */
  virtual ~UserCamera() = default;

  /**
   * @brief 获取变换矩阵
   * @return 4x4变换矩阵
   */
  glm::mat4 getTransformMatrix(std::shared_ptr<Gltf> gltf) const override;

  /**
   * @brief 设置垂直视场角
   * @param yfov 垂直视场角（弧度）
   */
  void setVerticalFoV(float yfov);

  /**
   * @brief 获取摄像机当前位置
   * @return 位置向量
   */
  glm::vec3 getPosition(std::shared_ptr<Gltf> gltf) const override;

/*        *//**
         * @brief 获取视图矩阵
         * @return 4x4视图矩阵
         *//*
        glm::mat4 getViewMatrix(std::shared_ptr<Gltf> gltf) const override;*/


  /**
   * @brief 获取摄像机当前旋转
   * @return 旋转四元数
   */
  glm::quat getRotation() const;

  /**
   * @brief 获取摄像机观察方向（归一化）
   * @return 方向向量
   */
  glm::vec3 getLookDirection() const;

  /**
   * @brief 获取摄像机观察目标点
   * @return 目标位置向量
   */
  glm::vec3 getTarget() const;

  /**
   * @brief 设置摄像机从from位置看向to位置
   * @param from 摄像机位置
   * @param to 目标位置
   */
  void lookAt(const glm::vec3 &from, const glm::vec3 &to);

  /**
   * @brief 设置摄像机位置
   * @param position 位置向量
   */
  void setPosition(const glm::vec3 &position);

  /**
   * @brief 设置摄像机目标，并根据当前距离调整位置
   * @param target 目标位置
   */
  void setTarget(const glm::vec3 &target);

  /**
   * @brief 设置摄像机旋转（欧拉角，度数）
   * @param yaw 偏航角（度）
   * @param pitch 俯仰角（度）
   */
  void setRotation(float yaw, float pitch);

  /**
   * @brief 设置摄像机距离目标的距离
   * @param distance 距离
   * @param target 目标位置
   */
  void setDistanceFromTarget(float distance, const glm::vec3 &target);

  /**
   * @brief 指数缩放
   * @param value 缩放值（推荐范围[-1,1]）
   */
  void zoomBy(float value);


  /**
   * @brief 绕目标点轨道运动
   * @param x x轴旋转量（弧度）
   * @param y y轴旋转量（弧度）
   */
  void orbit(float x, float y);

  /**
   * @brief 平移摄像机
   * @param x x轴平移量
   * @param y y轴平移量
   */
  void pan(float x, float y);

  /**
   * @brief 根据场景调整平移速度
   * @param min 场景最小值
   * @param max 场景最大值
   */
  void fitPanSpeedToScene(const glm::vec3 &min, const glm::vec3 &max);

  /**
   * @brief 重置摄像机
   */
  void reset();

  /**
   * @brief 重置视图以适应场景
   * @param gltf glTF对象
   * @param sceneIndex 场景索引
   */
  void resetView(std::shared_ptr<Gltf> gltf, int sceneIndex);

  /**
   * @brief 调整视图以适应场景但保持旋转
   * @param gltf glTF对象
   * @param sceneIndex 场景索引
   */
  void fitViewToScene(std::shared_ptr<Gltf> gltf, int sceneIndex);

  /**
   * @brief 根据范围调整距离
   * @param min 最小值
   * @param max 最大值
   */
  void fitDistanceToExtents(const glm::vec3 &min, const glm::vec3 &max);

  /**
   * @brief 根据范围调整摄像机目标
   * @param min 最小值
   * @param max 最大值
   */
  void fitCameraTargetToExtents(const glm::vec3 &min, const glm::vec3 &max);

  /**
   * @brief 根据范围调整摄像机近远平面
   * @param min 最小值
   * @param max 最大值
   */
  void fitCameraPlanesToExtents(const glm::vec3 &min, const glm::vec3 &max);

  // === Getter/Setter方法 ===
  float getDistance() const { return distance; }

  void setDistance(float distance) { this->distance = distance; }

  float getBaseDistance() const { return baseDistance; }

  void setBaseDistance(float baseDistance) {
    this->baseDistance = baseDistance;
  }

  float getZoomExponent() const { return zoomExponent; }

  void setZoomExponent(float zoomExponent) {
    this->zoomExponent = zoomExponent;
  }

  float getZoomFactor() const { return zoomFactor; }

  void setZoomFactor(float zoomFactor) { this->zoomFactor = zoomFactor; }

  float getOrbitSpeed() const { return orbitSpeed; }

  void setOrbitSpeed(float orbitSpeed) { this->orbitSpeed = orbitSpeed; }

  float getPanSpeed() const { return panSpeed; }

  void setPanSpeed(float panSpeed) { this->panSpeed = panSpeed; }

  const SceneExtents &getSceneExtents() const { return sceneExtents; }

  void setSceneExtents(const SceneExtents &extents) { sceneExtents = extents; }

 private:
  glm::mat4 transform;        ///< 变换矩阵
  float rotAroundY;           ///< 绕Y轴旋转角度
  float rotAroundX;           ///< 绕X轴旋转角度
  float distance;             ///< 到目标的距离
  float baseDistance;         ///< 基础距离
  float zoomExponent;         ///< 缩放指数
  float zoomFactor;           ///< 缩放因子
  float orbitSpeed;           ///< 轨道运动速度
  float panSpeed;             ///< 平移速度
  SceneExtents sceneExtents;  ///< 场景范围
};

} // namespace digitalhumans


#endif //LIGHTDIGITALHUMAN_USERCAMERA_H
