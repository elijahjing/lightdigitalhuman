//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_ENVIRONMENTRENDERER_H
#define LIGHTDIGITALHUMAN_ENVIRONMENTRENDERER_H

#include <memory>
#include <vector>
#include <string>
#include <GLES3/gl3.h>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

namespace digitalhumans {
class ShaderCache;
class GltfOpenGLContext;
class GltfState;

/**
 * @brief 环境渲染器类
 *
 * 用于渲染环境贴图（天空盒），支持IBL（基于图像的照明）环境
 */
class EnvironmentRenderer {
 public:

  explicit EnvironmentRenderer(std::shared_ptr<GltfOpenGLContext> context);

  /**
   * @brief 析构函数
   */
  virtual ~EnvironmentRenderer();

  /**
   * @brief 禁用拷贝构造
   */
  EnvironmentRenderer(const EnvironmentRenderer &) = delete;

  /**
   * @brief 禁用拷贝赋值
   */
  EnvironmentRenderer &operator=(const EnvironmentRenderer &) = delete;

  /**
   * @brief 移动构造函数
   */
  EnvironmentRenderer(EnvironmentRenderer &&other) noexcept;

  /**
   * @brief 移动赋值操作符
   */
  EnvironmentRenderer &operator=(EnvironmentRenderer &&other) noexcept;

  /**
   * @brief 绘制环境贴图
   * @param context 上下文
   * @param viewProjectionMatrix 视图投影矩阵
   * @param state 渲染状态
   * @param shaderCache 着色器缓存
   * @param fragDefines 片段着色器定义
   */
  void drawEnvironmentMap(std::shared_ptr<GltfOpenGLContext> context,
                          const glm::mat4 &viewProjectionMatrix,
                          std::shared_ptr<GltfState> state,
                          std::shared_ptr<ShaderCache> shaderCache,
                          const std::vector<std::string> &fragDefines);

  /**
   * @brief 检查渲染器是否已初始化
   * @return 如果已初始化返回true
   */
  bool isInitialized() const { return initialized; }

  /**
   * @brief 获取天空盒立方体的顶点数量
   * @return 顶点数量
   */
  static constexpr int getVertexCount() { return 8; }

  /**
   * @brief 获取天空盒立方体的三角形数量
   * @return 三角形数量
   */
  static constexpr int getTriangleCount() { return 12; }

  /**
   * @brief 获取天空盒立方体的索引数量
   * @return 索引数量
   */
  static constexpr int getIndexCount() { return 36; }

  /**
   * @brief 释放资源
   */
  void cleanup();

 private:
  /**
   * @brief 初始化几何体缓冲区
   * @param context GL上下文
   * @return 如果成功返回true
   */
  bool initializeBuffers(std::shared_ptr<GltfOpenGLContext> context);

  /**
   * @brief 创建立方体索引数据
   * @return 索引数据
   */
  std::vector<uint16_t> createCubeIndices() const;

  /**
   * @brief 创建立方体顶点数据
   * @return 顶点数据
   */
  std::vector<float> createCubeVertices() const;

  /**
   * @brief 设置渲染状态
   */
  void setupRenderState();

  /**
   * @brief 恢复渲染状态
   */
  void restoreRenderState();

  /**
   * @brief 绑定几何体
   * @param positionAttributeLocation 位置属性位置
   */
  void bindGeometry(GLint positionAttributeLocation);

  /**
   * @brief 计算环境旋转矩阵
   * @param rotationDegrees 旋转角度（度）
   * @return 3x3旋转矩阵
   */
  glm::mat3 calculateEnvironmentRotationMatrix(float rotationDegrees) const;


  /**
   * @brief 检查OpenGL错误
   * @param operation 操作描述
   * @return 如果没有错误返回true
   */
  bool checkGLError(const std::string &operation) const;

 private:
  GLuint indexBuffer;         ///< 索引缓冲区对象
  GLuint vertexBuffer;        ///< 顶点缓冲区对象
  bool initialized;           ///< 初始化状态标志

  // 渲染状态备份
  struct RenderStateBackup {
    GLenum frontFace;           ///< 正面方向
    GLboolean cullFaceEnabled;  ///< 面剔除启用状态
    GLboolean blendEnabled;     ///< 混合启用状态
    GLboolean depthTestEnabled; ///< 深度测试启用状态
  };

  mutable RenderStateBackup stateBackup; ///< 渲染状态备份
};

} // namespace digitalhumans


#endif //LIGHTDIGITALHUMAN_ENVIRONMENTRENDERER_H
