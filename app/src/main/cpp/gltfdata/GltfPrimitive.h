//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFPRIMITIVE_H
#define LIGHTDIGITALHUMAN_GLTFPRIMITIVE_H

#include "GltfObject.h"
#include "GltfTexture.h"
#include "GltfAccessor.h"
#include "vec3.hpp"
#include <GLES3/gl3.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <array>
#include <map>        // 添加这行！

namespace digitalhumans {

class GltfPrimitive;

/**
 * @brief glTF图元顶点属性信息
 */
struct GLAttribute {
  std::string attribute;      ///< 属性名称 (如 "POSITION")
  std::string name;          ///< OpenGL属性名称 (如 "a_position")
  int accessor;              ///< 访问器索引
};

/**
 * @brief 材质变体映射信息
 */
struct MaterialMapping {
  int material;                           ///< 材质索引
  std::vector<int> variants;              ///< 变体索引数组
};

/**
 * @brief glTF图元类
 * 表示渲染的基本几何单元
 */
class GltfPrimitive: public GltfObject {
 public:
  bool
      skip;                                                          ///< 是否跳过渲染

  GltfPrimitive();
  /**
   * @brief 虚析构函数
   */
  ~GltfPrimitive() override = default;

  void initGl(std::shared_ptr<Gltf> gltf,
              std::shared_ptr<GltfOpenGLContext> openGlContext);

  /**
   * @brief 虚析构函数
   * @brief 计算图元重心
   * @param gltf glTF根对象
   */
  void computeCentroid(std::shared_ptr<Gltf> gltf);

  // === Getter/Setter方法 ===
  const std::map<std::string, int> &getAttributes() const { return attributes; }
  void setAttributes(const std::map<std::string, int> &attributes) {
    this->attributes = attributes;
  }
  void setAttribute(const std::string &name, int accessorIndex) {
    attributes[name] = accessorIndex;
  }

  const std::vector<std::map<std::string, int>> &
  getTargets() const { return targets; }
  void setTargets(const std::vector<std::map<std::string, int>> &targets) {
    this->targets = targets;
  }

  std::optional<int> getIndices() const { return indices; }
  void setIndices(std::optional<int> indices) { this->indices = indices; }

  std::optional<int> getMaterial() const { return material; }
  void setMaterial(std::optional<int> material) { this->material = material; }

  int getMode() const { return mode; }
  void setMode(int mode) { this->mode = mode; }

  const std::vector<GLAttribute> &
  getGLAttributes() const { return glAttributes; }
  std::shared_ptr<GltfTextureInfo>
  getMorphTargetTextureInfo() const { return morphTargetTextureInfo; }

  const std::vector<std::string> &getDefines() const { return defines; }
  bool shouldSkip() const { return skip; }

  // === 属性查询方法 ===
  bool hasWeights() const { return m_hasWeights; }
  bool hasJoints() const { return m_hasJoints; }
  bool hasNormals() const { return m_hasNormals; }
  bool hasTangents() const { return m_hasTangents; }
  bool hasTexcoord() const { return m_hasTexcoord; }
  bool hasColor() const { return m_hasColor; }

  const glm::vec3 &getCentroid() const { return centroid; }

  // === 材质变体支持 ===
  const std::vector<MaterialMapping> &getMappings() const { return mappings; }
  std::vector<uint32_t>
  getIndicesAsUint32(std::shared_ptr<GltfAccessor> accessor, const Gltf &gltf);

  /**
* @brief 获取可动画属性名称列表
* @return 属性名称列表
*/


 private:


  /**
   * @brief 处理Draco压缩网格解压缩
   * @param gltf glTF根对象
   */
  void handleDracoCompression(std::shared_ptr<Gltf> gltf);

  /**
   * @brief 处理变形目标
   * @param gltf glTF根对象
   * @param webGlContext WebGL上下文
   */
  void processMorphTargets(std::shared_ptr<Gltf> gltf,
                           std::shared_ptr<GltfOpenGLContext> webGlContext);

  /**
   * @brief 生成切线向量
   * @param gltf glTF根对象
   */
  void generateTangents(std::shared_ptr<Gltf> gltf);

  /**
   * @brief 展开顶点索引
   * @param gltf glTF根对象
   */
  void unweld(std::shared_ptr<Gltf> gltf);

  /**
   * @brief 展开单个访问器
   * @param gltf glTF根对象
   * @param accessor 访问器对象
   * @param indices 索引数组
   * @return 新的访问器索引
   */
  int unweldAccessor(std::shared_ptr<Gltf> gltf,
                     std::shared_ptr<GltfAccessor> accessor,
                     const std::vector<uint32_t> &indices);

  /**
   * @brief 创建缓冲区和访问器
   * @param gltf glTF根对象
   * @param data 数据指针
   * @param byteLength 数据长度
   * @param target 缓冲区目标
   * @param count 元素数量
   * @param type 访问器类型
   * @param componentType 组件类型
   * @return 访问器索引
   */
  int createBufferAndAccessor(std::shared_ptr<Gltf> gltf,
                              const void *data,
                              size_t byteLength,
                              GLenum target,
                              int count,
                              const std::string &type,
                              GLenum componentType);


 private:
  // === glTF标准属性 ===
  std::map<std::string, int> attributes;                    ///< 顶点属性映射
  std::vector<std::map<std::string, int>> targets;          ///< 变形目标
  std::optional<int>
      indices = -1;                                         ///< 索引访问器
  std::optional<int>
      material = -1;                                        ///< 材质索引
  int mode;                                                        ///< 图元模式

  // === 非glTF标准属性（运行时数据） ===
  std::vector<GLAttribute>
       glAttributes;                              ///< OpenGL属性信息
  std::shared_ptr<GltfTextureInfo>
      morphTargetTextureInfo;            ///< 变形目标纹理信息
  std::vector<std::string>
      defines;                                   ///< 着色器宏定义

  // === 属性标志 ===
  bool m_hasWeights;                                                  ///< 是否有权重
  bool m_hasJoints;                                                   ///< 是否有关节
  bool m_hasNormals;                                                  ///< 是否有法线
  bool m_hasTangents;                                                 ///< 是否有切线
  bool
      m_hasTexcoord;                                                 ///< 是否有纹理坐标
  bool m_hasColor;                                                    ///< 是否有颜色

  // === 几何信息 ===
  glm::vec3 centroid;                                     ///< 重心坐标

  // === 材质变体扩展 ===
  std::vector<MaterialMapping>
      mappings;                              ///< 材质变体映射
};

} // namespace digitalhumans


#endif //LIGHTDIGITALHUMAN_GLTFPRIMITIVE_H
