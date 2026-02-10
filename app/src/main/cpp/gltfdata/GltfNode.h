//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFNODE_H
#define LIGHTDIGITALHUMAN_GLTFNODE_H
#include "GltfObject.h"
#include "mat4x4.hpp"
#include "gtc/quaternion.hpp"
#include "vec3.hpp"
#include <vector>
#include <string>
#include <memory>
#include <optional>

namespace digitalhumans {

/**
 * @brief glTF节点类
 * 包含变换信息和子节点索引（引用场景节点数组）
 */
class GltfNode: public GltfObject {
 public:
  /**
   * @brief 构造函数
   */
  GltfNode();

  /**
   * @brief 虚析构函数
   */
  virtual ~GltfNode() = default;


  /**
   * @brief 获取权重数据
   * @param gltf glTF根对象
   * @return 权重数组
   */
  std::vector<double> getWeights(std::shared_ptr<Gltf> gltf);

  /**
   * @brief 应用变换矩阵
   * 从4x4矩阵中提取平移、旋转、缩放分量
   * @param matrixData 4x4变换矩阵数据
   */
  void applyMatrix(const glm::mat4 &matrixData);

  /**
   * @brief 获取本地变换矩阵
   * @return 从TRS构建的4x4变换矩阵
   */
  glm::mat4 getLocalTransform();

  // === 判断方法 ===
  /**
   * @brief 检查节点是否有有效的世界变换矩阵
   * @return true 如果世界变换矩阵不是单位矩阵
   */
  bool hasWorldTransform() const { return worldTransform != glm::mat4(1.0f); }

  /**
   * @brief 检查节点是否有权重数据
   * @return true 如果权重数组不为空
   */
  bool hasWeights() const { return !weights.empty(); }

  /**
   * @brief 检查节点是否有网格
   * @return true 如果网格索引有效（不是-1）
   */
  bool hasMesh() const { return mesh.has_value() && mesh.value() >= 0; }

  /**
   * @brief 检查节点是否有摄像机
   * @return true 如果摄像机索引有效
   */
  bool hasCamera() const { return camera.has_value() && camera.value() >= 0; }

  /**
   * @brief 检查节点是否有蒙皮
   * @return true 如果蒙皮索引有效
   */
  bool hasSkin() const { return skin.has_value() && skin.value() >= 0; }

  /**
   * @brief 检查节点是否有光源
   * @return true 如果光源索引有效
   */
  bool hasLight() const { return light.has_value() && light.value() >= 0; }

  /**
   * @brief 检查节点是否有子节点
   * @return true 如果子节点数组不为空
   */
  bool hasChildren() const { return !children.empty(); }

  /**
   * @brief 检查节点是否有变换矩阵
   * @return true 如果变换矩阵有值
   */
  bool hasMatrix() const { return matrix.has_value(); }

  /**
   * @brief 检查节点是否有实例数据
   * @return true 如果实例矩阵数组不为空
   */
  bool hasInstances() const { return !instanceMatrices.empty(); }

  // === Getter方法 ===
  std::optional<int> getCamera() const { return camera; }
  const std::vector<int> &getChildren() const { return children; }
  std::optional<glm::mat4> getMatrix() const { return matrix; }
  const glm::quat &getRotation() const { return rotation; }
  const glm::vec3 &getScale() const { return scale; }
  const glm::vec3 &getTranslation() const { return translation; }
  const std::string &getName() const { return name; }
  std::optional<int> getMesh() const { return mesh; }
  std::optional<int> getSkin() const { return skin; }
  const std::vector<double> &getWeights() const { return weights; }

  // === 非glTF标准属性的Getter ===
  const glm::mat4 &getWorldTransform() const { return worldTransform; }
  const glm::quat &getWorldQuaternion() const { return worldQuaternion; }
  const glm::mat4 &
  getInverseWorldTransform() const { return inverseWorldTransform; }
  const glm::mat4 &getNormalMatrix() const { return normalMatrix; }
  std::optional<int> getLight() const { return light; }
  const std::vector<glm::mat4> &
  getInstanceMatrices() const { return instanceMatrices; }
  const std::vector<glm::mat4> &
  getInstanceWorldTransforms() const { return instanceWorldTransforms; }

  // === Setter方法 ===
  void setCamera(std::optional<int> camera) { this->camera = camera; }
  void setChildren(const std::vector<int> &children) {
    this->children = children;
  }
  void setMatrix(std::optional<glm::mat4> matrix) {
    applyMatrix(matrix.value());
  }
  void setRotation(const glm::quat &rotation) { this->rotation = rotation; }
  void setScale(const glm::vec3 &scale) { this->scale = scale; }
  void setTranslation(const glm::vec3 &translation) {
    this->translation = translation;
  }
  void setName(const std::string &name) { this->name = name; }
  void setMesh(std::optional<int> mesh) { this->mesh = mesh; }
  void setSkin(std::optional<int> skin) { this->skin = skin; }
  void setWeights(const std::vector<double> &weights) {
    this->weights = weights;
  }

  // === 非glTF标准属性的Setter ===
  void setWorldTransform(const glm::mat4 &worldTransform) {
    this->worldTransform = worldTransform;
  }
  void setWorldQuaternion(const glm::quat &worldQuaternion) {
    this->worldQuaternion = worldQuaternion;
  }
  void setInverseWorldTransform(const glm::mat4 &inverseWorldTransform) {
    this->inverseWorldTransform = inverseWorldTransform;
  }
  void setNormalMatrix(const glm::mat4 &normalMatrix) {
    this->normalMatrix = normalMatrix;
  }
  void setLight(std::optional<int> light) { this->light = light; }
  void setInstanceMatrices(const std::vector<glm::mat4> &instanceMatrices) {
    this->instanceMatrices = instanceMatrices;
  }
  void
  setInstanceWorldTransforms(const std::vector<glm::mat4> &instanceWorldTransforms) {
    this->instanceWorldTransforms = instanceWorldTransforms;
  }

  void setInitialRotation(const glm::quat &initialRotation);

  void setInitialScale(const glm::vec3 &initialScale);

  void setInitialTranslation(const glm::vec3 &initialTranslation);

  void setInitialWeights(const std::vector<double> &initialWeights);

  // 重置方法
  void resetTranslation() { translation = initialTranslation; }
  void resetRotation() { rotation = initialRotation; }
  void resetScale() { scale = initialScale; }
  void resetWeights() { weights = initialWeights; }

 private:
  // === glTF标准属性 ===
  std::optional<int> camera;          ///< 摄像机索引
  std::vector<int> children;          ///< 子节点索引数组
  std::optional<glm::mat4> matrix;    ///< 4x4变换矩阵（可选）
  glm::quat rotation;                 ///< 旋转四元数
  glm::vec3 scale;                    ///< 缩放向量
  glm::vec3 translation;              ///< 平移向量

  glm::quat initialRotation;                 ///< 旋转四元数
  glm::vec3 initialScale;                    ///< 缩放向量
  glm::vec3 initialTranslation;              ///< 平移向量
  std::vector<double> initialWeights;         ///< morph目标权重


  std::string name;                   ///< 节点名称
  std::optional<int> mesh = -1;            ///< 网格索引
  std::optional<int> skin = -1;            ///< 蒙皮索引
  std::vector<double> weights;         ///< morph目标权重

  // === 非glTF标准属性 ===
  glm::mat4 worldTransform;           ///< 世界变换矩阵
  glm::quat worldQuaternion;          ///< 世界旋转四元数
  glm::mat4 inverseWorldTransform;    ///< 逆世界变换矩阵
  glm::mat4 normalMatrix;             ///< 法线变换矩阵
  std::optional<int> light;           ///< 光源索引
  std::vector<glm::mat4> instanceMatrices;        ///< 实例变换矩阵数组
  std::vector<glm::mat4> instanceWorldTransforms; ///< 实例世界变换矩阵数组
};

} // namespace digitalhumans

#endif //LIGHTDIGITALHUMAN_GLTFNODE_H
