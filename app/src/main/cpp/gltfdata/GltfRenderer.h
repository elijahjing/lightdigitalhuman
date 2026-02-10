//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFRENDERER_H
#define LIGHTDIGITALHUMAN_GLTFRENDERER_H


#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <array>
#include <GLES3/gl3.h>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "GltfState.h"
#include "GltfScene.h"
#include "GltfCamera.h"
#include "GltfMaterial.h"

namespace digitalhumans {


class GltfPrimitive;

class EnvironmentRenderer;

class ShaderCache;

class GltfLight;


/**
 * @brief 可绘制对象结构体
 */
struct Drawable {
  std::shared_ptr<GltfNode> node;           ///< 节点对象
  std::shared_ptr<GltfPrimitive> primitive; ///< 图元对象
  int primitiveIndex;                       ///< 图元索引
  float depth;                             ///< 深度值（用于排序）

  Drawable() : primitiveIndex(-1), depth(0.0f) {}

  Drawable(std::shared_ptr<GltfNode> n,
           std::shared_ptr<GltfPrimitive> p,
           int idx)
      : node(n), primitive(p), primitiveIndex(idx), depth(0.0f) {}
};


/**
 * @brief 实例化绘制数据
 */
struct InstanceData {
  std::vector<Drawable> drawables;                    ///< 可绘制对象列表
  std::vector<glm::mat4> instanceTransforms;         ///< 实例变换矩阵
  std::string groupId;                               ///< 分组ID

  InstanceData() = default;

  explicit InstanceData(const std::string &id) : groupId(id) {}
};

/**
 * @brief 渲染通道配置
 */
struct RenderPassConfiguration {
  bool linearOutput;              ///< 是否使用线性输出
  bool enableBlending;            ///< 是否启用混合
  bool enableDepthTest;           ///< 是否启用深度测试
  bool enableCulling;             ///< 是否启用面剔除
  GLenum cullFace;               ///< 剔除面类型
  GLenum frontFace;              ///< 正面定义

  RenderPassConfiguration()
      : linearOutput(false), enableBlending(false), enableDepthTest(true),
        enableCulling(true),
        cullFace(GL_BACK), frontFace(GL_CCW) {}
};

/**
 * @brief glTF渲染器主类
 *
 * 负责渲染整个glTF场景，包括环境贴图、不透明物体、透明物体等
 */
class GltfRenderer {


 public:

  /**
   * @brief 构造函数
   * @param context OpenGL上下文
   */
  explicit GltfRenderer();

  /**
   * @brief 析构函数
   */
  virtual ~GltfRenderer();

  /**
   * @brief 禁用拷贝构造
   */
  GltfRenderer(const GltfRenderer &) = delete;

  /**
   * @brief 禁用拷贝赋值
   */
  GltfRenderer &operator=(const GltfRenderer &) = delete;

  /**
   * @brief 移动构造函数
   */
  GltfRenderer(GltfRenderer &&other) noexcept;

  /**
   * @brief 移动赋值操作符
   */
  GltfRenderer &operator=(GltfRenderer &&other) noexcept;

  /**
   * @brief 初始化渲染器
   * @param state 应用状态
   * @return 如果初始化成功返回true
   */
  bool init(std::shared_ptr<GltfState> state);

  /**
   * @brief 调整渲染器大小
   * @param width 宽度
   * @param height 高度
   */
  void resize(int width, int height);

  /**
   * @brief 清除帧缓冲区
   * @param clearColor 清除颜色 (RGBA)
   */
  void clearFrame(const glm::vec4 &clearColor);

  /**
   * @brief 准备场景渲染
   * @param state 渲染状态
   * @param scene 要渲染的场景
   */
  void prepareScene(std::shared_ptr<GltfState> state,
                    std::shared_ptr<GltfScene> scene);

  /**
   * @brief 绘制整个场景
   * @param state 渲染状态
   * @param scene 要渲染的场景
   */
  void
  drawScene(std::shared_ptr<GltfState> state, std::shared_ptr<GltfScene> scene);

  /**
   * @brief 绘制单个图元
   * @param state 渲染状态
   * @param config 渲染通道配置
   * @param primitive 图元对象
   * @param node 节点对象
   * @param viewProjectionMatrix 视图投影矩阵
   * @param transmissionSampleTexture 透射采样纹理
   * @param instanceOffset 实例偏移量
   */
  void drawPrimitive(std::shared_ptr<GltfState> state,
                     const RenderPassConfiguration &config,
                     std::shared_ptr<GltfPrimitive> primitive,
                     std::shared_ptr<GltfNode> node,
                     const glm::mat4 &viewProjectionMatrix,
                     GLuint transmissionSampleTexture = 0,
                     const std::vector<glm::mat4> *instanceOffset = nullptr);

  /**
   * @brief 销毁渲染器资源
   */
  void destroy();

  /**
   * @brief 检查渲染器是否已初始化
   * @return 如果已初始化返回true
   */
  bool isInitialized() const { return initialized; }

  // === Getter方法 ===

  /**
   * @brief 获取当前宽度
   * @return 宽度
   */
  int getCurrentWidth() const { return currentWidth; }

  /**
   * @brief 获取当前高度
   * @return 高度
   */
  int getCurrentHeight() const { return currentHeight; }

  /**
   * @brief 获取WebGL上下文
   * @return WebGL上下文
   */
  std::shared_ptr<GltfOpenGLContext> getWebGL() const { return openGlContext; }

  /**
   * @brief 获取着色器缓存
   * @return 着色器缓存
   */
  std::shared_ptr<ShaderCache> getShaderCache() const { return shaderCache; }

  /**
   * @brief 获取环境渲染器
   * @return 环境渲染器
   */
  std::shared_ptr<EnvironmentRenderer>
  getEnvironmentRenderer() const { return environmentRenderer; }

  /**
   * @brief 获取当前相机位置
   * @return 相机位置
   */
  const glm::vec3 &
  getCurrentCameraPosition() const { return currentCameraPosition; }

  /**
   * @brief 获取视图矩阵
   * @return 视图矩阵
   */
  const glm::mat4 &getViewMatrix() const { return viewMatrix; }

  /**
   * @brief 获取投影矩阵
   * @return 投影矩阵
   */
  const glm::mat4 &getProjectionMatrix() const { return projMatrix; }

  /**
   * @brief 获取视图投影矩阵
   * @return 视图投影矩阵
   */
  const glm::mat4 &
  getViewProjectionMatrix() const { return viewProjectionMatrix; }

  // === 调试和统计 ===

  /**
   * @brief 获取绘制调用统计
   * @return 绘制调用次数
   */
  size_t getDrawCallCount() const { return drawCallCount; }

  /**
   * @brief 获取渲染的图元数量
   * @return 图元数量
   */
  size_t getRenderedPrimitives() const { return renderedPrimitives; }

  /**
   * @brief 重置渲染统计
   */
  void resetStatistics();

  /**
   * @brief 设置多重采样级别
   * @param samples 采样级别
   */
  void setSamples(int samples);

  /**
   * @brief 获取当前多重采样级别
   * @return 采样级别
   */
  int getSamples() const { return samples; }

  /**
   * @brief 设置帧缓冲区大小
   * @param width 宽度
   * @param height 高度
   */
  void setFramebufferSize(int width, int height);

  /**
   * @brief 启用/禁用详细日志
   * @param enabled 是否启用
   */
  void setVerboseLogging(bool enabled) { verboseLogging = enabled; }

  /**
   * @brief 获取渲染器调试信息
   * @return 调试信息字符串
   */
  std::string getDebugInfo() const;

 private:
  // === 初始化方法 ===

  /**
   * @brief 初始化着色器源代码
   */
  void initializeShaderSources();

  /**
   * @brief 创建帧缓冲区
   * @return 如果成功返回true
   */
  bool createFramebuffers();

  /**
   * @brief 初始化默认光源
   */
  void initializeDefaultLights();

  /**
   * @brief 设置OpenGL状态
   */
  void setupOpenGLState();

  // === 场景准备方法 ===

  /**
   * @brief 收集所有节点
   * @param state 渲染状态
   * @param scene 场景对象
   * @return 节点列表
   */
  std::vector<std::shared_ptr<GltfNode>>
  gatherNodes(std::shared_ptr<GltfState> state,
              std::shared_ptr<GltfScene> scene);

  /**
   * @brief 收集可绘制对象
   * @param state 渲染状态
   * @param nodes 节点列表
   * @return 可绘制对象列表
   */
  std::vector<Drawable> collectDrawables(std::shared_ptr<GltfState> state,
                                         const std::vector<std::shared_ptr<
                                             GltfNode>> &nodes);

  /**
   * @brief 分组可绘制对象（用于实例化渲染）
   * @param drawables 可绘制对象列表
   * @return 分组后的实例数据
   */
  std::unordered_map<std::string, InstanceData>
  groupDrawables(const std::vector<Drawable> &drawables);

  /**
   * @brief 过滤不透明可绘制对象
   * @param drawables 可绘制对象列表
   * @return 不透明对象列表
   */
  std::vector<Drawable>
  filterOpaqueDrawables(const std::vector<Drawable> &drawables,
                        std::shared_ptr<GltfState> ptr);

  /**
   * @brief 过滤透明可绘制对象
   * @param drawables 可绘制对象列表
   * @return 透明对象列表
   */
  std::vector<Drawable>
  filterTransparentDrawables(const std::vector<Drawable> &drawables,
                             std::shared_ptr<GltfState> ptr);

  /**
   * @brief 过滤透射可绘制对象
   * @param drawables 可绘制对象列表
   * @return 透射对象列表
   */
  std::vector<Drawable>
  filterTransmissionDrawables(const std::vector<Drawable> &drawables,
                              std::shared_ptr<GltfState> ptr);

  // === 相机和矩阵计算 ===

  /**
   * @brief 计算相机矩阵
   * @param state 渲染状态
   * @param camera 相机对象
   */
  void calculateCameraMatrices(std::shared_ptr<GltfState> state,
                               std::shared_ptr<GltfCamera> camera);

  /**
   * @brief 计算视口参数
   * @param camera 相机对象
   * @param aspectOffsetX X偏移（输出）
   * @param aspectOffsetY Y偏移（输出）
   * @param aspectWidth 宽度（输出）
   * @param aspectHeight 高度（输出）
   */
  void calculateViewportParameters(std::shared_ptr<GltfCamera> camera,
                                   float &aspectOffsetX, float &aspectOffsetY,
                                   float &aspectWidth, float &aspectHeight);

  // === 光照处理 ===

  /**
   * @brief 获取可见光源
   * @param state 渲染状态
   * @param nodes 节点列表
   * @return 可见光源列表
   */
  std::vector<std::pair<std::shared_ptr<GltfNode>, std::shared_ptr<GltfLight>>>
  getVisibleLights(std::shared_ptr<GltfState> state,
                   const std::vector<int> &nodes);

  /**
   * @brief 应用光源到着色器
   */
  void applyLights();

  void updateSkins(std::shared_ptr<GltfState> state);


  /**
   * @brief 更新皮肤动画
   * @param state 渲染状态
   * @param node 节点对象
   */
  void
  updateSkin(std::shared_ptr<GltfState> state, std::shared_ptr<GltfNode> node);

  // === 着色器定义生成 ===

  /**
   * @brief 添加顶点着色器参数定义
   * @param vertDefines 顶点着色器定义列表
   * @param parameters 渲染参数
   * @param state 渲染状态
   * @param node 节点对象
   * @param primitive 图元对象
   */
  void pushVertParameterDefines(std::vector<std::string> &vertDefines,
                                const RenderingParameters &parameters,
                                std::shared_ptr<GltfState> state,
                                std::shared_ptr<GltfNode> node,
                                std::shared_ptr<GltfPrimitive> primitive);


  /**
   * @brief 添加片段着色器参数定义
   * @param fragDefines 片段着色器定义列表
   * @param state 渲染状态
   */
  void pushFragParameterDefines(std::vector<std::string> &fragDefines,
                                std::shared_ptr<GltfState> state);

  /**
   * @brief 更新动画uniform变量
   * @param state 渲染状态
   * @param node 节点对象
   * @param primitive 图元对象
   */
  void updateAnimationUniforms(std::shared_ptr<GltfState> state,
                               std::shared_ptr<GltfNode> node,
                               std::shared_ptr<GltfPrimitive> primitive);

  // === 纹理和环境贴图 ===

  /**
   * @brief 应用环境贴图
   * @param state 渲染状态
   * @param texSlotOffset 纹理槽偏移量
   * @return 使用的纹理槽数量
   */
  int applyEnvironmentMap(std::shared_ptr<GltfState> state, int texSlotOffset);

  // === 渲染辅助方法 ===

  /**
   * @brief 设置渲染状态
   * @param material 材质对象
   * @param node 节点对象
   */
  void setupRenderState(std::shared_ptr<GltfMaterial> material,
                        std::shared_ptr<GltfNode> node);

  /**
   * @brief 绑定顶点属性
   * @param state 渲染状态
   * @param primitive 图元对象
   * @param instanceOffset 实例偏移量
   * @return 顶点数量
   */
  int bindVertexAttributes(std::shared_ptr<GltfState> state,
                           std::shared_ptr<GltfPrimitive> primitive,
                           const std::vector<glm::mat4> *instanceOffset);

  /**
   * @brief 解除顶点属性绑定
   * @param primitive 图元对象
   * @param instanceOffset 实例偏移量
   */
  void unbindVertexAttributes(std::shared_ptr<GltfPrimitive> primitive,
                              const std::vector<glm::mat4> *instanceOffset);

  /**
   * @brief 更新材质uniform变量
   * @param state 渲染状态
   * @param material 材质对象
   * @param textureSlotOffset 纹理槽起始偏移量
   * @return 使用的纹理槽数量
   */
  int updateMaterialUniforms(std::shared_ptr<GltfState> state,
                             std::shared_ptr<GltfMaterial> material,
                             int textureSlotOffset,
                             std::shared_ptr<GltfPrimitive> ptr,
                             std::shared_ptr<GltfNode> sharedPtr);

  /**
   * @brief 执行绘制调用
   * @param primitive 图元对象
   * @param vertexCount 顶点数量
   * @param instanceOffset 实例偏移量
   */
  void
  executeDrawCall(std::shared_ptr<GltfPrimitive> primitive, int vertexCount,
                  const std::vector<glm::mat4> *instanceOffset,
                  std::shared_ptr<GltfState> ptr);

  // === 帧缓冲区管理 ===

  /**
   * @brief 销毁帧缓冲区
   */
  void destroyFramebuffers();

  /**
   * @brief 重新创建帧缓冲区
   * @param width 宽度
   * @param height 高度
   * @return 如果成功返回true
   */
  bool recreateFramebuffers(int width, int height);

  /**
   * @brief 绑定不透明帧缓冲区
   */
  void bindOpaqueFramebuffer();

  /**
   * @brief 绑定默认帧缓冲区
   */
  void bindDefaultFramebuffer();

  /**
   * @brief 执行帧缓冲区blit操作
   */
  void blitFramebuffers();

  // === 实例化渲染支持 ===

  /**
   * @brief 准备实例化数据
   * @param instanceData 实例数据
   */
  void prepareInstanceData(const InstanceData &instanceData);

  /**
   * @brief 绑定实例缓冲区
   * @param instanceTransforms 实例变换矩阵
   */
  void bindInstanceBuffer(const std::vector<glm::mat4> &instanceTransforms);

  /**
   * @brief 解除实例缓冲区绑定
   */
  void unbindInstanceBuffer();

  // === 排序和深度测试 ===

  /**
   * @brief 按深度排序可绘制对象
   * @param drawables 可绘制对象列表
   * @param camera 相机对象
   * @return 排序后的对象列表
   */
  std::vector<Drawable> sortDrawablesByDepth(std::vector<Drawable> &drawables,
                                             std::shared_ptr<GltfState> state);
  /**
   * @brief 计算对象到相机的距离
   * @param drawable 可绘制对象
   * @return z距离值
   */
  float calculateDistanceToCamera(const Drawable &drawable);

  // === 错误处理和调试 ===

  /**
   * @brief 检查OpenGL错误
   * @param operation 操作描述
   * @return 如果没有错误返回true
   */
  bool checkGLError(const std::string &operation) const;

  /**
   * @brief 验证帧缓冲区完整性
   * @param framebuffer 帧缓冲区ID
   * @return 如果完整返回true
   */
  bool validateFramebuffer(GLuint framebuffer) const;

  /**
   * @brief 记录详细日志
   * @param message 消息内容
   */
  void logVerbose(const std::string &message) const;

  // === 着色器源代码常量 ===

  /**
   * @brief 获取PBR片段着色器源代码
   * @return 着色器源代码
   */
  static std::string getPbrFragmentShaderSource();

  /**
   * @brief 获取图元顶点着色器源代码
   * @return 着色器源代码
   */
  static std::string getPrimitiveVertexShaderSource();

  /**
   * @brief 获取立方体贴图顶点着色器源代码
   * @return 着色器源代码
   */
  static std::string getCubemapVertexShaderSource();

  /**
   * @brief 获取立方体贴图片段着色器源代码
   * @return 着色器源代码
   */
  static std::string getCubemapFragmentShaderSource();

  /**
   * @brief 获取镜面光泽度片段着色器源代码
   * @return 着色器源代码
   */
  static std::string getSpecularGlossinessFragmentShaderSource();

  // === 包含文件着色器源代码 ===

  /**
   * @brief 获取BRDF着色器源代码
   * @return 着色器源代码
   */
  static std::string getBrdfShaderSource();

  /**
   * @brief 获取虹彩效果着色器源代码
   * @return 着色器源代码
   */
  static std::string getIridescenceShaderSource();

  /**
   * @brief 获取材质信息着色器源代码
   * @return 着色器源代码
   */
  static std::string getMaterialInfoShaderSource();

  /**
   * @brief 获取IBL着色器源代码
   * @return 着色器源代码
   */
  static std::string getIblShaderSource();

  /**
   * @brief 获取点光源着色器源代码
   * @return 着色器源代码
   */
  static std::string getPunctualShaderSource();

  /**
   * @brief 获取纹理处理着色器源代码
   * @return 着色器源代码
   */
  static std::string getTexturesShaderSource();

  /**
   * @brief 获取色调映射着色器源代码
   * @return 着色器源代码
   */
  static std::string getTonemappingShaderSource();

  /**
   * @brief 获取通用函数着色器源代码
   * @return 着色器源代码
   */
  static std::string getFunctionsShaderSource();

  /**
   * @brief 获取动画着色器源代码
   * @return 着色器源代码
   */
  static std::string getAnimationShaderSource();


  std::vector<std::vector<glm::mat4>> prepareInstanceTransforms();

  void renderTransmissionBackground(std::shared_ptr<GltfState> state,
                                    const std::vector<std::vector<glm::mat4>> &instanceTransforms);

  void renderToCanvas(std::shared_ptr<GltfState> state,
                      float aspectOffsetX, float aspectOffsetY,
                      float aspectWidth, float aspectHeight,
                      const std::vector<std::vector<glm::mat4>> &instanceTransforms);

  std::shared_ptr<GltfCamera>
  getCurrentCamera(std::shared_ptr<GltfState> state);

  std::shared_ptr<GltfMaterial>
  getMaterial(int materialIndex, std::shared_ptr<GltfState> ptr);


  std::shared_ptr<GltfMaterial>
  getMaterialWithVariant(std::shared_ptr<GltfState> state,
                         std::shared_ptr<GltfPrimitive> primitive);

  std::pair<size_t, size_t>
  selectShaderPermutation(std::shared_ptr<GltfState> state,
                          const RenderPassConfiguration &config,
                          std::shared_ptr<GltfPrimitive> primitive,
                          std::shared_ptr<GltfNode> node,
                          std::shared_ptr<GltfMaterial> material,
                          const std::vector<glm::mat4> *instanceOffset);

  void
  bindTransmissionSampleTexture(GLuint transmissionTexture, int textureSlot,
                                std::shared_ptr<GltfNode> node);

  void updateCommonUniforms(std::shared_ptr<GltfState> state,
                            std::shared_ptr<GltfNode> node,
                            const glm::mat4 &viewProjectionMatrix);

  void updateExtensionUniforms(std::shared_ptr<GltfMaterial> material);

  int bindJointTexture(std::shared_ptr<GltfState> state,
                       int textureSlot,
                       std::shared_ptr<GltfNode> ptr);

  void addDebugOutputDefines(std::vector<std::string> &fragDefines,
                             DebugOutput debugOutput);

 private:
  // === 核心组件 ===
  std::shared_ptr<GltfShader> shader;                    ///< 当前着色器
  std::shared_ptr<GltfOpenGLContext>
      openGlContext;                      ///< WebGL上下文
  std::shared_ptr<ShaderCache> shaderCache;              ///< 着色器缓存
  std::shared_ptr<EnvironmentRenderer> environmentRenderer; ///< 环境渲染器

  // === 渲染状态 ===
  bool initialized;                                       ///< 是否已初始化
  int currentWidth;                                       ///< 当前宽度
  int currentHeight;                                      ///< 当前高度
  int samples;                                           ///< 多重采样级别
  bool verboseLogging;                                   ///< 详细日志标志

  // === 帧缓冲区对象 ===
  GLuint opaqueRenderTexture;                            ///< 不透明渲染纹理
  GLuint opaqueFramebuffer;                              ///< 不透明帧缓冲区
  GLuint opaqueFramebufferMSAA;                          ///< 多重采样帧缓冲区
  GLuint opaqueDepthTexture;                             ///< 不透明深度纹理
  GLuint colorRenderBuffer;                              ///< 颜色渲染缓冲区
  GLuint depthRenderBuffer;                              ///< 深度渲染缓冲区
  int opaqueFramebufferWidth;                            ///< 帧缓冲区宽度
  int opaqueFramebufferHeight;                           ///< 帧缓冲区高度

  // === 实例化渲染 ===
  GLuint instanceBuffer;                                 ///< 实例缓冲区
  int maxVertAttributes;                                 ///< 最大顶点属性数量

  // === 相机和变换矩阵 ===
  glm::mat4 viewMatrix;                                  ///< 视图矩阵
  glm::mat4 projMatrix;                                  ///< 投影矩阵
  glm::mat4 viewProjectionMatrix;                        ///< 视图投影矩阵
  glm::vec3 currentCameraPosition;                       ///< 当前相机位置

  // === 光源 ===
  std::vector<std::pair<std::shared_ptr<GltfNode>,
                        std::shared_ptr<GltfLight>>> visibleLights; ///< 可见光源列表
  std::shared_ptr<GltfLight> lightKey;                   ///< 主光源
  std::shared_ptr<GltfLight> lightFill;                  ///< 补充光源
  std::shared_ptr<GltfLight> lightBack;                  ///< 补充光源

  // === 场景渲染数据 ===
  std::vector<std::shared_ptr<GltfNode>> nodes;          ///< 节点列表
  std::unordered_map<std::string, InstanceData>
      opaqueDrawables;     ///< 不透明可绘制对象
  std::vector<Drawable> transparentDrawables;            ///< 透明可绘制对象
  std::vector<Drawable> transmissionDrawables;           ///< 透射可绘制对象
  std::shared_ptr<GltfScene> preparedScene;              ///< 已准备的场景

  // === 统计信息 ===
  mutable size_t drawCallCount;                          ///< 绘制调用次数
  mutable size_t renderedPrimitives;                     ///< 渲染图元数量
  mutable size_t shaderSwitches;                         ///< 着色器切换次数
  mutable size_t textureBinds;                           ///< 纹理绑定次数

 public:


  /**
   * @brief 获取着色器切换次数
   * @return 切换次数
   */
  size_t getShaderSwitches() const { return shaderSwitches; }

  /**
   * @brief 获取纹理绑定次数
   * @return 绑定次数
   */
  size_t getTextureBinds() const { return textureBinds; }

  /**
   * @brief 获取帧缓冲区信息
   * @return 帧缓冲区信息结构体
   */
  struct FramebufferInfo {
    int width;
    int height;
    int samples;
    GLuint colorTexture;
    GLuint depthTexture;
    GLuint framebuffer;
    GLuint msaaFramebuffer;

    FramebufferInfo() : width(0), height(0), samples(0),
                        colorTexture(0), depthTexture(0),
                        framebuffer(0), msaaFramebuffer(0) {}
  };

  /**
   * @brief 获取帧缓冲区信息
   * @return 帧缓冲区信息
   */
  FramebufferInfo getFramebufferInfo() const;

  /**
   * @brief 强制重新编译所有着色器
   */
  void recompileShaders();

  /**
   * @brief 预热着色器缓存
   * @param state 渲染状态
   */
  void warmupShaderCache(std::shared_ptr<GltfState> state);


  void setupCenteredCamera(std::shared_ptr<Gltf> gltf);

};

} // namespace digitalhumans

#endif //LIGHTDIGITALHUMAN_GLTFRENDERER_H
