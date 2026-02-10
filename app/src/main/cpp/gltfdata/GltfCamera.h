//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFCAMERA_H
#define LIGHTDIGITALHUMAN_GLTFCAMERA_H

#include "GltfObject.h"
#include "GltfNode.h"
#include "mat4x4.hpp"
#include "vec3.hpp"
#include "gtc/quaternion.hpp"
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <limits>
#include <any>
#include <unordered_map>

namespace digitalhumans {

class Drawable;

class GltfRenderer;

/**
 * @brief 透视摄像机参数类
 */
class PerspectiveCamera: public GltfObject {
 public:
  float yfov;                         ///< 垂直视场角（弧度）
  std::optional<float> aspectRatio;   ///< 宽高比（可选）
  float znear;                        ///< 近平面
  float zfar;                         ///< 远平面

  PerspectiveCamera();

  virtual ~PerspectiveCamera() = default;

  // === Getter/Setter方法 ===
  float getYfov() const { return yfov; }

  void setYfov(float yfov) { this->yfov = yfov; }

  std::optional<float> getAspectRatio() const { return aspectRatio; }

  void setAspectRatio(std::optional<float> aspectRatio) {
    this->aspectRatio = aspectRatio;
  }

  float getZnear() const { return znear; }

  void setZnear(float znear) { this->znear = znear; }

  float getZfar() const { return zfar; }

  void setZfar(float zfar) { this->zfar = zfar; }

};

/**
 * @brief 正交摄像机参数类
 */
class OrthographicCamera: public GltfObject {
 public:
  OrthographicCamera();

  virtual ~OrthographicCamera() = default;

  // === Getter/Setter方法 ===
  float getXmag() const { return xmag; }

  void setXmag(float xmag) { this->xmag = xmag; }

  float getYmag() const { return ymag; }

  void setYmag(float ymag) { this->ymag = ymag; }

  float getZnear() const { return znear; }

  void setZnear(float znear) { this->znear = znear; }

  float getZfar() const { return zfar; }

  void setZfar(float zfar) { this->zfar = zfar; }


 private:
  float xmag;     ///< 水平放大倍数
  float ymag;     ///< 垂直放大倍数
  float znear;    ///< 近平面
  float zfar;     ///< 远平面
};

/**
 * @brief glTF摄像机类
 */
class GltfCamera: public GltfObject {
 public:
  /**
   * @brief 摄像机类型枚举
   */
  enum class Type {
    PERSPECTIVE,
    ORTHOGRAPHIC
  };

  /**
   * @brief 构造函数
   */
  GltfCamera();

  /**
   * @brief 虚析构函数
   */
  virtual ~GltfCamera() = default;


  /**
   * @brief 按深度排序图元
   * @param gltf glTF根对象
   * @param drawables 可绘制对象列表
   * @return 排序后的可绘制对象列表
   */
  std::vector<Drawable> sortPrimitivesByDepth(std::shared_ptr<Gltf> gltf,
                                              std::vector<Drawable> drawables);

  /**
   * @brief 获取投影矩阵
   * @param aspectRatio 宽高比
   * @return 投影矩阵
   */
  glm::mat4 getProjectionMatrix(float aspectRatio) const;

  /**
   * @brief 获取视图矩阵
   * @param gltf glTF根对象
   * @return 视图矩阵
   */
  glm::mat4 getViewMatrix(std::shared_ptr<Gltf> gltf) const;

  /**
   * @brief 获取摄像机目标点
   * @param gltf glTF根对象
   * @return 目标位置
   */
  glm::vec3 getTarget(const std::shared_ptr<Gltf> &gltf) const;

  /**
   * @brief 获取摄像机位置
   * @param gltf glTF根对象
   * @return 摄像机位置
   */
  virtual glm::vec3 getPosition(std::shared_ptr<Gltf> gltf) const;

  /**
   * @brief 获取观察方向
   * @param gltf glTF根对象
   * @return 观察方向向量
   */
  glm::vec3 getLookDirection(std::shared_ptr<Gltf> gltf) const;

  /**
   * @brief 获取旋转矩阵
   * @param gltf glTF根对象
   * @return 旋转矩阵
   */
  glm::mat4 getRotationMatrix(std::shared_ptr<Gltf> gltf) const;

  /**
   * @brief 获取旋转四元数
   * @param gltf glTF根对象
   * @return 旋转四元数
   */
  glm::quat getRotation(std::shared_ptr<Gltf> gltf) const;

  /**
   * @brief 设置关联的节点
   * @param gltf glTF根对象
   * @param nodeIndex 节点索引
   */
  void setNode(std::shared_ptr<Gltf> gltf, int nodeIndex);

  /**
   * @brief 获取关联的节点
   * @param gltf glTF根对象
   * @return 节点对象
   */
  std::shared_ptr<GltfNode> getNode(std::shared_ptr<Gltf> gltf) const;

  /**
   * @brief 获取变换矩阵
   * @param gltf glTF根对象
   * @return 变换矩阵
   */
  virtual glm::mat4 getTransformMatrix(std::shared_ptr<Gltf> gltf) const;

  /**
   * @brief 获取摄像机描述JSON
   * @param gltf glTF根对象
   * @return JSON描述对象
   */
  std::unordered_map<std::string, std::any>
  getDescription(std::shared_ptr<Gltf> gltf) const;

  // === Getter/Setter方法 ===
  const std::string &getName() const { return name; }

  void setName(const std::string &name) { this->name = name; }

  std::optional<int> getNodeIndex() const { return node; }

  void setNodeIndex(std::optional<int> node) { this->node = node; }

  Type getType() const { return type; }

  void setType(Type type) { this->type = type; }

  PerspectiveCamera &getPerspective() { return perspective; }

  const PerspectiveCamera &getPerspective() const { return perspective; }

  OrthographicCamera &getOrthographic() { return orthographic; }

  const OrthographicCamera &getOrthographic() const { return orthographic; }

 protected:


  /**
   * @brief 摄像机类型到字符串转换
   * @param type 摄像机类型
   * @return 类型字符串
   */
  static std::string typeToString(Type type);

  /**
   * @brief 字符串到摄像机类型转换
   * @param str 类型字符串
   * @return 摄像机类型
   */
  static Type stringToType(const std::string &str);

 public:
  PerspectiveCamera perspective;      ///< 透视摄像机参数（公共访问以保持兼容性）
  OrthographicCamera orthographic;    ///< 正交摄像机参数（公共访问以保持兼容性）

 private:
  std::string name;           ///< 摄像机名称
  std::optional<int> node;    ///< 关联的节点索引
  Type type;                  ///< 摄像机类型

  float aspect;

  glm::mat4 createFallbackViewMatrix() const;
};

} // namespace digitalhumans


#endif //LIGHTDIGITALHUMAN_GLTFCAMERA_H
