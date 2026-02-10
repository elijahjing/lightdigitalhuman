//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFUTILS_H
#define LIGHTDIGITALHUMAN_GLTFUTILS_H

#ifndef GLTF_UTILS_H
#define GLTF_UTILS_H

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include <memory>
#include <vector>
#include <array>
#include "GltfAccessor.h"
#include "GltfNode.h"

namespace digitalhumans {
class Gltf;

/**
 * @brief glTF工具函数集合
 */
class GltfUtils {
 public:
  /**
   * @brief 获取场景的包围盒范围
   * @param gltf glTF根对象
   * @param sceneIndex 场景索引
   * @param outMin 输出最小坐标
   * @param outMax 输出最大坐标
   */
  static void getSceneExtents(std::shared_ptr<Gltf> gltf,
                              int sceneIndex,
                              glm::vec3 &outMin,
                              glm::vec3 &outMax);

  /**
   * @brief 从访问器获取变换后的包围盒范围
   * @param accessor glTF访问器
   * @param worldTransform 世界变换矩阵
   * @param outMin 输出最小坐标
   * @param outMax 输出最大坐标
   */
  static void getExtentsFromAccessor(std::shared_ptr<GltfAccessor> accessor,
                                     const glm::mat4 &worldTransform,
                                     glm::vec3 &outMin,
                                     glm::vec3 &outMax);

  /**
   * @brief 获取节点的包围盒范围（递归处理子节点）
   * @param gltf glTF根对象
   * @param node 节点对象
   * @param outMin 输出最小坐标
   * @param outMax 输出最大坐标
   */
  static void getNodeExtents(std::shared_ptr<Gltf> gltf,
                             std::shared_ptr<GltfNode> node,
                             glm::vec3 &outMin,
                             glm::vec3 &outMax);

  /**
   * @brief 获取网格的包围盒范围
   * @param gltf glTF根对象
   * @param meshIndex 网格索引
   * @param worldTransform 世界变换矩阵
   * @param outMin 输出最小坐标
   * @param outMax 输出最大坐标
   */
  static void getMeshExtents(std::shared_ptr<Gltf> gltf,
                             int meshIndex,
                             const glm::mat4 &worldTransform,
                             glm::vec3 &outMin,
                             glm::vec3 &outMax);

  /**
   * @brief 计算场景的包围球
   * @param gltf glTF根对象
   * @param sceneIndex 场景索引
   * @param outCenter 输出球心坐标
   * @param outRadius 输出半径
   */
  static void getSceneBoundingSphere(std::shared_ptr<Gltf> gltf,
                                     int sceneIndex,
                                     glm::vec3 &outCenter,
                                     float &outRadius);

  /**
   * @brief 变换3D向量
   * @param vector 输入向量
   * @param transform 变换矩阵
   * @return 变换后的向量
   */
  static glm::vec3 transformVec3(const glm::vec3 &vector,
                                 const glm::mat4 &transform);

  /**
   * @brief 变换3D点（齐次坐标）
   * @param point 输入点
   * @param transform 变换矩阵
   * @return 变换后的点
   */
  static glm::vec3 transformPoint(const glm::vec3 &point,
                                  const glm::mat4 &transform);

  /**
   * @brief 变换方向向量（不包含平移）
   * @param direction 输入方向向量
   * @param transform 变换矩阵
   * @return 变换后的方向向量
   */
  static glm::vec3 transformDirection(const glm::vec3 &direction,
                                      const glm::mat4 &transform);

  /**
   * @brief 计算两个包围盒的并集
   * @param min1 第一个包围盒最小坐标
   * @param max1 第一个包围盒最大坐标
   * @param min2 第二个包围盒最小坐标
   * @param max2 第二个包围盒最大坐标
   * @param outMin 输出并集最小坐标
   * @param outMax 输出并集最大坐标
   */
  static void unionBoundingBoxes(const glm::vec3 &min1,
                                 const glm::vec3 &max1,
                                 const glm::vec3 &min2,
                                 const glm::vec3 &max2,
                                 glm::vec3 &outMin,
                                 glm::vec3 &outMax);

  /**
   * @brief 检查包围盒是否有效
   * @param min 最小坐标
   * @param max 最大坐标
   * @return 如果包围盒有效返回true
   */
  static bool isValidBoundingBox(const glm::vec3 &min,
                                 const glm::vec3 &max);

  /**
   * @brief 计算包围盒的中心点
   * @param min 最小坐标
   * @param max 最大坐标
   * @return 中心点坐标
   */
  static glm::vec3 getBoundingBoxCenter(const glm::vec3 &min,
                                        const glm::vec3 &max);

  /**
   * @brief 计算包围盒的尺寸
   * @param min 最小坐标
   * @param max 最大坐标
   * @return 尺寸 [width, height, depth]
   */
  static glm::vec3 getBoundingBoxSize(const glm::vec3 &min,
                                      const glm::vec3 &max);

  /**
   * @brief 计算包围盒的对角线长度
   * @param min 最小坐标
   * @param max 最大坐标
   * @return 对角线长度
   */
  static float getBoundingBoxDiagonal(const glm::vec3 &min,
                                      const glm::vec3 &max);

  /**
   * @brief 扩展包围盒以包含一个点
   * @param min 包围盒最小坐标（输入/输出）
   * @param max 包围盒最大坐标（输入/输出）
   * @param point 要包含的点
   */
  static void expandBoundingBox(glm::vec3 &min,
                                glm::vec3 &max,
                                const glm::vec3 &point);

  /**
   * @brief 检查点是否在包围盒内
   * @param point 测试点
   * @param min 包围盒最小坐标
   * @param max 包围盒最大坐标
   * @return 如果点在包围盒内返回true
   */
  static bool isPointInsideBoundingBox(const glm::vec3 &point,
                                       const glm::vec3 &min,
                                       const glm::vec3 &max);

  /**
   * @brief 检查两个包围盒是否相交
   * @param min1 第一个包围盒最小坐标
   * @param max1 第一个包围盒最大坐标
   * @param min2 第二个包围盒最小坐标
   * @param max2 第二个包围盒最大坐标
   * @return 如果包围盒相交返回true
   */
  static bool boundingBoxesIntersect(const glm::vec3 &min1,
                                     const glm::vec3 &max1,
                                     const glm::vec3 &min2,
                                     const glm::vec3 &max2);

  /**
   * @brief 计算包围球半径
   * @param center 球心
   * @param min 包围盒最小坐标
   * @param max 包围盒最大坐标
   * @return 半径
   */
  static float calculateBoundingSphereRadius(const glm::vec3 &center,
                                             const glm::vec3 &min,
                                             const glm::vec3 &max);

  /**
   * @brief 获取包围盒的8个顶点
   * @param min 最小坐标
   * @param max 最大坐标
   * @return 8个顶点的数组
   */
  static std::array<glm::vec3, 8> getBoundingBoxVertices(const glm::vec3 &min,
                                                         const glm::vec3 &max);

  /**
   * @brief 矩阵分解为平移、旋转、缩放
   * @param matrix 输入矩阵
   * @param outTranslation 输出平移向量
   * @param outRotation 输出旋转四元数
   * @param outScale 输出缩放向量
   * @return 分解是否成功
   */
  static bool decomposeMatrix(const glm::mat4 &matrix,
                              glm::vec3 &outTranslation,
                              glm::quat &outRotation,
                              glm::vec3 &outScale);

  /**
   * @brief 从平移、旋转、缩放组合矩阵
   * @param translation 平移向量
   * @param rotation 旋转四元数
   * @param scale 缩放向量
   * @return 组合后的矩阵
   */
  static glm::mat4 composeMatrix(const glm::vec3 &translation,
                                 const glm::quat &rotation,
                                 const glm::vec3 &scale);

 private:
  /**
   * @brief 初始化包围盒为无效状态
   * @param outMin 输出最小坐标
   * @param outMax 输出最大坐标
   */
  static void initInvalidBoundingBox(glm::vec3 &outMin,
                                     glm::vec3 &outMax);

  /**
   * @brief 从8个包围盒顶点计算轴对齐包围盒
   * @param boxVertices 8个顶点坐标
   * @param outMin 输出最小坐标
   * @param outMax 输出最大坐标
   */
  static void
  computeAABBFromVertices(const std::array<glm::vec3, 8> &boxVertices,
                          glm::vec3 &outMin,
                          glm::vec3 &outMax);

  // 常量定义
  static constexpr float FLOAT_MAX = std::numeric_limits<float>::max();
  static constexpr float FLOAT_MIN = std::numeric_limits<float>::lowest();
  static constexpr float EPSILON = 1e-6f;
};

} // namespace digitalhumans

#endif // GLTF_UTILS_H

#endif //LIGHTDIGITALHUMAN_GLTFUTILS_H