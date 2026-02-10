//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFMESH_H
#define LIGHTDIGITALHUMAN_GLTFMESH_H


#include "GltfObject.h"
#include <vector>
#include <string>
#include <memory>

namespace digitalhumans {
class GltfPrimitive;


/**
 * @brief glTF网格类
 * 包含几何图元和morph目标权重
 */
class GltfMesh: public GltfObject {
 public:
  /**
   * @brief 构造函数
   */
  GltfMesh();

  /**
   * @brief 虚析构函数
   */
  virtual ~GltfMesh() = default;


  // === Getter方法 ===
  const std::vector<std::shared_ptr<GltfPrimitive>> &
  getPrimitives() { return primitives; }
  const std::string &getName() const { return name; }
  const std::vector<double> &getWeights() const { return weights; }

  // === Setter方法 ===
  void
  setPrimitives(const std::vector<std::shared_ptr<GltfPrimitive>> &primitives) {
    this->primitives = primitives;
  }
  void setName(const std::string &name) { this->name = name; }
  void setWeights(const std::vector<double> &weights) {
    this->weights = weights;
  }

  /**
   * @brief 添加图元到网格
   * @param primitive 图元对象
   */
  void addPrimitive(std::shared_ptr<GltfPrimitive> primitive);

  /**
   * @brief 获取图元数量
   * @return 图元数量
   */
  size_t getPrimitiveCount() const { return primitives.size(); }

  /**
   * @brief 检查是否有权重数据
   * @return true如果有权重数据
   */
  bool hasWeights() const { return !weights.empty(); }


 private:
  std::vector<std::shared_ptr<GltfPrimitive>> primitives;  ///< 网格图元数组
  std::string name;                                        ///< 网格名称
  std::vector<double> weights;                             ///< morph目标权重数组
};

} // namespace digitalhumans



#endif //LIGHTDIGITALHUMAN_GLTFMESH_H
