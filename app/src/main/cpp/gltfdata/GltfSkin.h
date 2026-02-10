//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFSKIN_H
#define LIGHTDIGITALHUMAN_GLTFSKIN_H
#include "GltfObject.h"
#include "GltfTexture.h"
#include "../utils/LogUtils.h"
#include "mat4x4.hpp"
#include <vector>
#include <string>
#include <memory>
#include <optional>

namespace digitalhumans {


/**
 * @brief glTF蒙皮类
 * 处理骨骼动画和蒙皮变形
 */
class GltfSkin: public GltfObject {
 public:
  /**
   * @brief 构造函数
   */
  GltfSkin();

  /**
   * @brief 虚析构函数
   */
  virtual ~GltfSkin() = default;


  /**
   * @brief 初始化WebGL资源
   * @param gltf glTF根对象
   * @param openGlContext 上下文
   */
  void initGl(std::shared_ptr<Gltf> gltf,
              std::shared_ptr<GltfOpenGLContext> openGlContext);

  /**
   * @brief 计算关节矩阵并更新纹理
   * @param gltf glTF根对象
   * @param openGlContext GL上下文
   */
  void computeJoints(std::shared_ptr<Gltf> gltf,
                     std::shared_ptr<GltfOpenGLContext> openGlContext);

  // === Getter/Setter方法 ===

  /**
   * @brief 获取蒙皮名称
   * @return 蒙皮名称
   */
  const std::string &getName() const { return name; }

  /**
   * @brief 设置蒙皮名称
   * @param name 蒙皮名称
   */
  void setName(const std::string &name) { this->name = name; }

  /**
   * @brief 获取逆绑定矩阵访问器索引
   * @return 访问器索引（可选）
   */
  std::optional<int>
  getInverseBindMatrices() const { return inverseBindMatrices; }

  /**
   * @brief 设置逆绑定矩阵访问器索引
   * @param inverseBindMatrices 访问器索引
   */
  void setInverseBindMatrices(std::optional<int> inverseBindMatrices) {
    this->inverseBindMatrices = inverseBindMatrices;
  }

  GLenum getJointWebGlTexture() const;

  /**
   * @brief 获取关节节点索引数组
   * @return 关节索引数组
   */
  const std::vector<int> &getJoints() const { return joints; }

  /**
   * @brief 设置关节节点索引数组
   * @param joints 关节索引数组
   */
  void setJoints(const std::vector<int> &joints) { this->joints = joints; }

  /**
   * @brief 获取骨架根节点索引
   * @return 骨架根节点索引（可选）
   */
  std::optional<int> getSkeleton() const { return skeleton; }

  /**
   * @brief 设置骨架根节点索引
   * @param skeleton 骨架根节点索引
   */
  void setSkeleton(std::optional<int> skeleton) { this->skeleton = skeleton; }

  /**
   * @brief 获取关节纹理信息
   * @return 关节纹理信息指针
   */
  std::shared_ptr<GltfTextureInfo> getJointTextureInfo() const {

    try {
      if (!jointTextureInfo) {
        return nullptr;
      }

      if (jointTextureInfo.use_count() <= 0) {
        LOGE("jointTextureInfo 引用计数异常: %ld",
             jointTextureInfo.use_count());
        return nullptr;
      }

      return jointTextureInfo;

    } catch (const std::exception &e) {
      LOGE("获取关节纹理信息异常: %s", e.what());
      return nullptr;
    } catch (...) {
      LOGE("获取关节纹理信息未知异常");
      return nullptr;
    }

    return jointTextureInfo;
  }

  /**
   * @brief 获取关节矩阵数组
   * @return 关节矩阵数组
   */
  const std::vector<glm::mat4> &
  getJointMatrices() const { return jointMatrices; }

  /**
   * @brief 获取关节法线矩阵数组
   * @return 关节法线矩阵数组
   */
  const std::vector<glm::mat4> &
  getJointNormalMatrices() const { return jointNormalMatrices; }

  /**
   * @brief 添加关节索引
   * @param jointIndex 关节节点索引
   */
  void addJoint(int jointIndex);

  /**
   * @brief 获取关节数量
   * @return 关节数量
   */
  size_t getJointCount() const { return joints.size(); }

 private:
  /**
   * @brief 创建关节纹理资源
   * @param gltf glTF根对象
   */
  void createJointTextureResources(std::shared_ptr<Gltf> gltf);

  /**
   * @brief 计算纹理尺寸
   * @param jointCount 关节数量
   * @return 纹理宽度
   */
  int calculateTextureWidth(size_t jointCount) const;

 private:
  // === glTF标准属性 ===
  std::string name;                           ///< 蒙皮名称
  std::optional<int> inverseBindMatrices;     ///< 逆绑定矩阵访问器索引
  std::vector<int> joints;                    ///< 关节节点索引数组
  std::optional<int> skeleton;                ///< 骨架根节点索引

  // === 非glTF标准属性（运行时数据） ===
  std::shared_ptr<GltfTextureInfo> jointTextureInfo;  ///< 关节纹理信息
  GLenum jointWebGlTexture;                      ///< WebGL关节纹理对象
  std::vector<glm::mat4> jointMatrices;                ///< 关节变换矩阵
  std::vector<glm::mat4> jointNormalMatrices;          ///< 关节法线矩阵
  bool webglResourcesInitialized;                      ///< WebGL资源初始化标志
  glm::mat4 simulateShaderMatrixRead(const std::vector<float> &textureData,
                                     int width,
                                     int shaderIndex);

};

} // namespace digitalhumans


#endif //LIGHTDIGITALHUMAN_GLTFSKIN_H
