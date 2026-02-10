//
// Created by vincentsyan on 2025/8/18.
//
#include "UserCamera.h"
#include "gtx/matrix_decompose.hpp"
#include <algorithm>
#include <cmath>
#include "GltfCamera.h"
#include "../utils/LogUtils.h"
#include "GltfUtils.h"

namespace digitalhumans {
template<typename T>
constexpr T clamp(const T &value, const T &min_val, const T &max_val) {
  return std::min(max_val, std::max(min_val, value));
}

/**
* Create a new user camera.
*/
UserCamera::UserCamera()
    : transform(1.0f), rotAroundY(0.0f), rotAroundX(0.0f), distance(1.0f),
      baseDistance(1.0f),
      zoomExponent(100.0f), zoomFactor(0.01f), orbitSpeed(1.0f / 180.0f),
      panSpeed(1.0f),
      sceneExtents{glm::vec3(0.0f), glm::vec3(0.0f)} {
}

/**
 * Get the transform matrix.
 */
glm::mat4 UserCamera::getTransformMatrix(std::shared_ptr<Gltf> gltf) const {
  return transform;
}

/**
 * Sets the vertical FoV of the user camera.
 */
void UserCamera::setVerticalFoV(float yfov) {
  // 假设基类有perspective成员
  perspective.yfov = yfov;
}

/**
 * Returns the current position of the user camera as a vec3.
 */
glm::vec3 UserCamera::getPosition(std::shared_ptr<Gltf> gltf) const {
  return glm::vec3(transform[3]);
}

/**
 * Returns the current rotation of the user camera as quat.
 */
glm::quat UserCamera::getRotation() const {
  glm::vec3 scale;
  glm::quat rotation;
  glm::vec3 translation;
  glm::vec3 skew;
  glm::vec4 perspective;
  glm::decompose(transform, scale, rotation, translation, skew, perspective);
  return rotation;
}

/**
 * Returns the normalized direction the user camera looks at as vec3.
 */
glm::vec3 UserCamera::getLookDirection() const {
  glm::vec3 dir(-transform[2][0], -transform[2][1], -transform[2][2]);
  return glm::normalize(dir);
}

/**
 * Returns the current target the camera looks at as vec3.
 * This multiplies the viewing direction with the distance.
 * For distance 0 the normalized viewing direction is used.
 */
glm::vec3 UserCamera::getTarget() const {
  const glm::vec3 position = getPosition(nullptr);
  glm::vec3 lookDirection = getLookDirection();

  if (distance != 0.0f && distance != 1.0f) {
    lookDirection *= distance;
  }

  return position + lookDirection;
}

/**
 * Look from user camera to target.
 * This changes the transformation of the user camera.
 */
void UserCamera::lookAt(const glm::vec3 &from, const glm::vec3 &to) {
  transform = glm::lookAt(from, to, glm::vec3(0.0f, 1.0f, 0.0f));
}

/**
 * Sets the position of the user camera.
 */
void UserCamera::setPosition(const glm::vec3 &position) {
  transform[3][0] = position.x;
  transform[3][1] = position.y;
  transform[3][2] = position.z;
}

/**
 * This rotates the user camera towards the target and sets the position of the user camera
 * according to the current distance.
 */
void UserCamera::setTarget(const glm::vec3 &target) {
  glm::vec3 pos = getPosition(nullptr);
  transform = glm::lookAt(pos, target, glm::vec3(0.0f, 1.0f, 0.0f));
  setDistanceFromTarget(distance, target);
}

/**
 * Sets the rotation of the camera.
 * Yaw and pitch in euler angles (degrees).
 */
void UserCamera::setRotation(float yaw, float pitch) {
  const glm::vec3 tmpPos = getPosition(nullptr);

  glm::mat4
      mat4x = glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::mat4
      mat4y = glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0.0f, 1.0f, 0.0f));

  transform = mat4y;
  setPosition(tmpPos);
  transform = transform * mat4x;
}

/**
 * Transforms the user camera to look at a target from a specific distance using the current rotation.
 * This will only change the position of the user camera, not the rotation.
 * Use this function to set the distance.
 */
void
UserCamera::setDistanceFromTarget(float distance, const glm::vec3 &target) {
  const glm::vec3 lookDirection = getLookDirection();
  const glm::vec3 distVec = lookDirection * (-distance);
  const glm::vec3 pos = target + distVec;

  setPosition(pos);
  this->distance = distance;
}

/**
 * Zoom exponentially according to this.zoomFactor and this.zoomExponent.
 * The default zoomFactor provides good zoom speed for values from [-1,1].
 */
void UserCamera::zoomBy(float value) {
  glm::vec3 target = getTarget();

  // zoom exponentially
  float zoomDistance = std::pow(distance / baseDistance, 1.0f / zoomExponent);
  zoomDistance += zoomFactor * (1 - value);
  zoomDistance = std::max(zoomDistance, 0.0001f);
  distance = std::pow(zoomDistance, zoomExponent) * baseDistance;

  setDistanceFromTarget(distance, target);
  fitCameraPlanesToExtents(sceneExtents.min, sceneExtents.max);
}

/**
 * Orbit around the target.
 * x and y should be in radians and are added to the current rotation.
 * The rotation around the x-axis is limited to 180 degrees.
 * The axes are inverted: e.g. if y is positive the camera will look further down.
 */
void UserCamera::orbit(float x, float y) {
  const glm::vec3 target = getTarget();
  const float rotAroundXMax = glm::pi<float>() / 2.0f - 0.01f;

  rotAroundY += (x * orbitSpeed);
  rotAroundX += (y * orbitSpeed);
  rotAroundX = clamp(rotAroundX, -rotAroundXMax, rotAroundXMax);

  setRotation(rotAroundY, rotAroundX);
  setDistanceFromTarget(distance, target);
}

/**
 * Pan the user camera.
 * The axes are inverted: e.g. if y is positive the camera will move down.
 */
void UserCamera::pan(float x, float y) {
  glm::vec3 right(transform[0][0], transform[0][1], transform[0][2]);
  right = glm::normalize(right);
  right *= (-x * panSpeed * (distance / baseDistance));

  glm::vec3 up(transform[1][0], transform[1][1], transform[1][2]);
  up = glm::normalize(up);
  up *= (-y * panSpeed * (distance / baseDistance));

  glm::vec3 pos = getPosition(nullptr);
  pos += up + right;
  setPosition(pos);
}

/**
 * Fit pan speed to scene extents.
 */
void
UserCamera::fitPanSpeedToScene(const glm::vec3 &min, const glm::vec3 &max) {
  const float longestDistance = glm::distance(min, max);
  panSpeed = longestDistance / PAN_SPEED_DENOMINATOR;
}

/**
 * Reset camera to default state.
 */
void UserCamera::reset() {
  transform = glm::mat4(1.0f);
  rotAroundX = 0.0f;
  rotAroundY = 0.0f;
  fitDistanceToExtents(sceneExtents.min, sceneExtents.max);
  fitCameraTargetToExtents(sceneExtents.min, sceneExtents.max);
}

/**
 * Calculates a camera position which looks at the center of the scene from an appropriate distance.
 * This calculates near and far plane as well.
 */
void UserCamera::resetView(std::shared_ptr<Gltf> gltf, int sceneIndex) {
  transform = glm::mat4(1.0f);
  rotAroundX = 0.0f;
  rotAroundY = 0.0f;

  GltfUtils::getSceneExtents(gltf,
                             sceneIndex,
                             sceneExtents.min,
                             sceneExtents.max);
  fitDistanceToExtents(sceneExtents.min, sceneExtents.max);
  fitCameraTargetToExtents(sceneExtents.min, sceneExtents.max);

  fitPanSpeedToScene(sceneExtents.min, sceneExtents.max);
  fitCameraPlanesToExtents(sceneExtents.min, sceneExtents.max);
}

/**
 * Fit view to updated canvas size without changing rotation if distance is incorrect.
 */
void UserCamera::fitViewToScene(std::shared_ptr<Gltf> gltf, int sceneIndex) {
  transform = glm::mat4(1.0f);

  GltfUtils::getSceneExtents(gltf,
                             sceneIndex,
                             sceneExtents.min,
                             sceneExtents.max);
  LOGE("Invalid gltf object");
  LOGE("SceneExtents - min: (%.3f, %.3f, %.3f), max: (%.3f, %.3f, %.3f)",
       sceneExtents.min.x, sceneExtents.min.y, sceneExtents.min.z,
       sceneExtents.max.x, sceneExtents.max.y, sceneExtents.max.z);

  fitDistanceToExtents(sceneExtents.min, sceneExtents.max);
  fitCameraTargetToExtents(sceneExtents.min, sceneExtents.max);

  fitPanSpeedToScene(sceneExtents.min, sceneExtents.max);
  fitCameraPlanesToExtents(sceneExtents.min, sceneExtents.max);
}

/**
 * Fit distance to scene extents.
 */
void
UserCamera::fitDistanceToExtents(const glm::vec3 &min, const glm::vec3 &max) {
  const float maxAxisLength = std::max({max.x - min.x, max.y - min.y});
  const float yfov = perspective.yfov;
  const float
      aspectRatio = perspective.aspectRatio ? *perspective.aspectRatio : 1.0f;
  const float xfov = perspective.yfov * aspectRatio;

  const float yZoom = (maxAxisLength / 2.0f) / std::tan(yfov / 2.0f);
  const float xZoom = (maxAxisLength / 2.0f) / std::tan(xfov / 2.0f);

  distance = std::max(xZoom, yZoom);
  baseDistance = distance;
}

/**
 * Fit camera target to scene extents.
 */
void UserCamera::fitCameraTargetToExtents(const glm::vec3 &min,
                                          const glm::vec3 &max) {
  glm::vec3 target = (max + min) * 0.5f;

  setRotation(rotAroundY, rotAroundX);
  setDistanceFromTarget(distance, target);
}

/**
 * Fit camera planes to scene extents.
 */
void UserCamera::fitCameraPlanesToExtents(const glm::vec3 &min,
                                          const glm::vec3 &max) {
  // depends only on scene min/max and the camera distance

  // Manually increase scene extent just for the camera planes to avoid camera clipping in most situations.
  const float longestDistance = 1000.0f * glm::distance(min, max);
  float zNear = distance - (longestDistance * 0.6f);
  float zFar = distance + (longestDistance * 0.6f);

  // minimum near plane value needs to depend on far plane value to avoid z fighting or too large near planes
  zNear = std::max(zNear, zFar / MAX_NEAR_FAR_RATIO);

  perspective.znear = zNear;
  perspective.zfar = zFar;

}
}// namespace digitalhumans