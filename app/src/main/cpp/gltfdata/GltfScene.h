//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFSCENE_H
#define LIGHTDIGITALHUMAN_GLTFSCENE_H


#include "GltfObject.h"
#include "mat4x4.hpp"
#include "gtc/quaternion.hpp"
#include <vector>
#include <string>
#include <memory>
#include "GltfNode.h"

namespace digitalhumans {
class ImageBasedLight;

/**
 * @brief glTF场景类
 * 管理场景中的节点层次结构和变换计算
 */
class GltfScene: public GltfObject {
 public:
  /**
   * @brief 构造函数
   * @param nodes 场景根节点索引列表
   * @param name 场景名称
   */
  explicit GltfScene(const std::vector<int> &nodes = {},
                     std::string name = "");

  /**
   * @brief 虚析构函数
   */
  virtual ~GltfScene() = default;


  /**
   * @brief 应用变换层次结构
   * 计算所有节点的世界变换、逆变换和法线矩阵
   * @param gltf glTF根对象
   * @param rootTransform 根变换矩阵
   */
  void applyTransformHierarchy(std::shared_ptr<Gltf> gltf,
                               const glm::mat4 &rootTransform = glm::mat4(1.0f));

  /**
   * @brief 收集场景中的所有节点
   * @param gltf glTF根对象
   * @return 节点对象列表
   */
  std::vector<std::shared_ptr<GltfNode>>
  gatherNodes(const std::shared_ptr<Gltf> &gltf);

  /**
   * @brief 检查场景是否包含指定节点
   * @param gltf glTF根对象
   * @param nodeIndex 节点索引
   * @return true如果场景包含该节点
   */
  bool includesNode(std::shared_ptr<Gltf> gltf, int nodeIndex);

  // Getter和Setter
  const std::vector<int> &getNodes() const { return nodes; }
  void setNodes(const std::vector<int> &nodes) { this->nodes = nodes; }

  const std::string &getName() const { return name; }
  void setName(const std::string &name) { this->name = name; }

  std::shared_ptr<ImageBasedLight>
  getImageBasedLight() const { return imageBasedLight; }


 private:
  std::vector<int> nodes;                                    ///< 场景根节点索引列表
  std::string name;                                         ///< 场景名称
  std::shared_ptr<ImageBasedLight> imageBasedLight;        ///< 基于图像的光照（非glTF标准）

  /**
   * @brief 递归应用变换
   * @param gltf glTF根对象
   * @param node 当前节点
   * @param parentTransform 父变换矩阵
   */
  void applyTransform(std::shared_ptr<Gltf> gltf,
                      std::shared_ptr<GltfNode> node,
                      const glm::mat4 &parentTransform);

  /**
   * @brief 递归应用世界旋转
   * @param gltf glTF根对象
   * @param node 当前节点
   * @param parentRotation 父旋转四元数
   */
  void applyWorldRotation(std::shared_ptr<Gltf> gltf,
                          std::shared_ptr<GltfNode> node,
                          const glm::quat &parentRotation);

  /**
   * @brief 递归收集节点
   * @param gltf glTF根对象
   * @param nodeIndex 节点索引
   * @param nodes 输出节点列表
   */
  void gatherNode(std::shared_ptr<Gltf> gltf,
                  int nodeIndex,
                  std::vector<std::shared_ptr<GltfNode>> &nodes);
};

} // namespace digitalhumans

#endif //LIGHTDIGITALHUMAN_GLTFSCENE_H
