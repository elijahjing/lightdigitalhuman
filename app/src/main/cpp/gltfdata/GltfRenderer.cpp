//
// Created by vincentsyan on 2025/8/18.
//
#include "GltfRenderer.h"
#include "../utils/utils.h"
#include <android/log.h>
#include <algorithm>
#include <sstream>
#include <cmath>

#include "../utils/LogUtils.h"
#include "UniformTypes.h"  // 或者包含定义 UniformValue 的正确头文件
#include "ShaderCache.h"  // 或者包含定义 UniformValue 的正确头文件
#include "GltfLight.h"  // 或者包含定义 UniformValue 的正确头文件
#include "EnvironmentRenderer.h"  // 或者包含定义 UniformValue 的正确头文件

#include "GltfPrimitive.h"
#include "GltfShader.h"

#include "GltfOpenGLContext.h"
#include "Gltf.h"
#include "GltfMesh.h"
#include "GltfSkin.h"
#include "GltfBufferView.h"
#include "GltfBuffer.h"

#include "converter/ShaderManager.h"
#include "GltfUtils.h"

namespace digitalhumans {

GltfRenderer::GltfRenderer()
    : shader(nullptr), openGlContext(nullptr), shaderCache(nullptr),
      environmentRenderer(nullptr), initialized(false),
      currentWidth(0), currentHeight(0), samples(4), verboseLogging(false),
      opaqueRenderTexture(0),
      opaqueFramebuffer(0), opaqueFramebufferMSAA(0), opaqueDepthTexture(0),
      colorRenderBuffer(0),
      depthRenderBuffer(0), opaqueFramebufferWidth(1024),
      opaqueFramebufferHeight(1024), instanceBuffer(0),
      maxVertAttributes(0), viewMatrix(1.0f), projMatrix(1.0f),
      viewProjectionMatrix(1.0f),
      currentCameraPosition(0.0f), visibleLights(), lightKey(nullptr),
      lightFill(nullptr), nodes(),
      opaqueDrawables(), transparentDrawables(), transmissionDrawables(),
      preparedScene(nullptr),
      drawCallCount(0), renderedPrimitives(0), shaderSwitches(0),
      textureBinds(0) {
  try {
    openGlContext = std::make_shared<GltfOpenGLContext>();
    if (!openGlContext) {
      LOGE("Failed to create WebGL context");
      return;
    }

    LOGI("GltfRenderer created with context");
  } catch (const std::exception &e) {
    LOGE("Exception in GltfRenderer constructor: %s", e.what());
  }
}

GltfRenderer::~GltfRenderer() {
  destroy();
}

GltfRenderer::GltfRenderer(GltfRenderer &&other) noexcept
    : shader(std::move(other.shader)), openGlContext(std::move(other.openGlContext)),
      shaderCache(std::move(other.shaderCache)),
      environmentRenderer(std::move(other.environmentRenderer)),
      initialized(other.initialized),
      currentWidth(other.currentWidth), currentHeight(other.currentHeight),
      samples(other.samples),
      verboseLogging(other.verboseLogging),
      opaqueRenderTexture(other.opaqueRenderTexture),
      opaqueFramebuffer(other.opaqueFramebuffer),
      opaqueFramebufferMSAA(other.opaqueFramebufferMSAA),
      opaqueDepthTexture(other.opaqueDepthTexture),
      colorRenderBuffer(other.colorRenderBuffer),
      depthRenderBuffer(other.depthRenderBuffer),
      opaqueFramebufferWidth(other.opaqueFramebufferWidth),
      opaqueFramebufferHeight(other.opaqueFramebufferHeight),
      instanceBuffer(other.instanceBuffer),
      maxVertAttributes(other.maxVertAttributes), viewMatrix(other.viewMatrix),
      projMatrix(other.projMatrix),
      viewProjectionMatrix(other.viewProjectionMatrix),
      currentCameraPosition(other.currentCameraPosition),
      visibleLights(std::move(other.visibleLights)),
      lightKey(std::move(other.lightKey)),
      lightFill(std::move(other.lightFill)), nodes(std::move(other.nodes)),
      opaqueDrawables(std::move(other.opaqueDrawables)),
      transparentDrawables(std::move(other.transparentDrawables)),
      transmissionDrawables(std::move(other.transmissionDrawables)),
      preparedScene(std::move(other.preparedScene)),
      drawCallCount(other.drawCallCount),
      renderedPrimitives(other.renderedPrimitives),
      shaderSwitches(other.shaderSwitches),
      textureBinds(other.textureBinds) {
  // 重置源对象
  other.initialized = false;
  other.currentWidth = 0;
  other.currentHeight = 0;
  other.samples = 4;
  other.verboseLogging = false;
  other.opaqueRenderTexture = 0;
  other.opaqueFramebuffer = 0;
  other.opaqueFramebufferMSAA = 0;
  other.opaqueDepthTexture = 0;
  other.colorRenderBuffer = 0;
  other.depthRenderBuffer = 0;
  other.instanceBuffer = 0;
  other.maxVertAttributes = 0;
  other.drawCallCount = 0;
  other.renderedPrimitives = 0;
  other.shaderSwitches = 0;
  other.textureBinds = 0;
}

GltfRenderer &GltfRenderer::operator=(GltfRenderer &&other) noexcept {
  if (this != &other) {
    destroy();

    shader = std::move(other.shader);
    openGlContext = std::move(other.openGlContext);
    shaderCache = std::move(other.shaderCache);
    environmentRenderer = std::move(other.environmentRenderer);
    initialized = other.initialized;
    currentWidth = other.currentWidth;
    currentHeight = other.currentHeight;
    samples = other.samples;
    verboseLogging = other.verboseLogging;
    opaqueRenderTexture = other.opaqueRenderTexture;
    opaqueFramebuffer = other.opaqueFramebuffer;
    opaqueFramebufferMSAA = other.opaqueFramebufferMSAA;
    opaqueDepthTexture = other.opaqueDepthTexture;
    colorRenderBuffer = other.colorRenderBuffer;
    depthRenderBuffer = other.depthRenderBuffer;
    opaqueFramebufferWidth = other.opaqueFramebufferWidth;
    opaqueFramebufferHeight = other.opaqueFramebufferHeight;
    instanceBuffer = other.instanceBuffer;
    maxVertAttributes = other.maxVertAttributes;
    viewMatrix = other.viewMatrix;
    projMatrix = other.projMatrix;
    viewProjectionMatrix = other.viewProjectionMatrix;
    currentCameraPosition = other.currentCameraPosition;
    visibleLights = std::move(other.visibleLights);
    lightKey = std::move(other.lightKey);
    lightFill = std::move(other.lightFill);
    nodes = std::move(other.nodes);
    opaqueDrawables = std::move(other.opaqueDrawables);
    transparentDrawables = std::move(other.transparentDrawables);
    transmissionDrawables = std::move(other.transmissionDrawables);
    preparedScene = std::move(other.preparedScene);
    drawCallCount = other.drawCallCount;
    renderedPrimitives = other.renderedPrimitives;
    shaderSwitches = other.shaderSwitches;
    textureBinds = other.textureBinds;

    // 重置源对象
    other.initialized = false;
    other.currentWidth = 0;
    other.currentHeight = 0;
    other.samples = 4;
    other.verboseLogging = false;
    other.opaqueRenderTexture = 0;
    other.opaqueFramebuffer = 0;
    other.opaqueFramebufferMSAA = 0;
    other.opaqueDepthTexture = 0;
    other.colorRenderBuffer = 0;
    other.depthRenderBuffer = 0;
    other.instanceBuffer = 0;
    other.maxVertAttributes = 0;
    other.drawCallCount = 0;
    other.renderedPrimitives = 0;
    other.shaderSwitches = 0;
    other.textureBinds = 0;
  }
  return *this;
}


bool GltfRenderer::init(std::shared_ptr<GltfState> state) {
  if (initialized) {
    return true;
  }
  if (!openGlContext) {
    LOGE("Invalid WebGL context during initialization");
    return false;
  }

  try {
    // 初始化着色器源代码
    initializeShaderSources();

    // 设置OpenGL状态
    setupOpenGLState();

    // 创建帧缓冲区
    if (!createFramebuffers()) {
      LOGE("Failed to create framebuffers");
      return false;
    }

    // 初始化默认光源
    initializeDefaultLights();

    // 创建环境渲染器
    environmentRenderer = std::make_shared<EnvironmentRenderer>(openGlContext);
    if (!environmentRenderer) {
      LOGE("Failed to create environment renderer");
      return false;
    }

    // 获取最大顶点属性数量
    GLint maxAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
    maxVertAttributes = static_cast<int>(maxAttribs);

    initialized = true;
    checkGLError("renderer initialization");
    return true;

  } catch (const std::exception &e) {
    LOGE("Exception during renderer initialization: %s", e.what());
    return false;
  }
}

void GltfRenderer::initializeShaderSources() {
  ShaderCache::SourceMap sources;

  // 主要着色器
  sources["primitive.vert"] = getPrimitiveVertexShaderSource();
  sources["pbr.frag"] = getPbrFragmentShaderSource();
  sources["cubemap.vert"] = getCubemapVertexShaderSource();
  sources["cubemap.frag"] = getCubemapFragmentShaderSource();
  sources["specular_glossiness.frag"] =
      getSpecularGlossinessFragmentShaderSource();
  // 包含文件
  sources["material_info.glsl"] = getMaterialInfoShaderSource();
  sources["brdf.glsl"] = getBrdfShaderSource();
  sources["iridescence.glsl"] = getIridescenceShaderSource();
  sources["ibl.glsl"] = getIblShaderSource();
  sources["punctual.glsl"] = getPunctualShaderSource();
  sources["tonemapping.glsl"] = getTonemappingShaderSource();
  sources["textures.glsl"] = getTexturesShaderSource();
  sources["functions.glsl"] = getFunctionsShaderSource();
  sources["animation.glsl"] = getAnimationShaderSource();

  // 创建着色器缓存
  shaderCache = std::make_shared<ShaderCache>(sources, openGlContext);
  if (!shaderCache) {
    throw std::runtime_error("Failed to create shader cache");
  }

  LOGI("Initialized shader cache with %zu sources", sources.size());
}

void GltfRenderer::setupOpenGLState() {
  if (!openGlContext) {
    LOGE("Invalid WebGL context");
    return;
  }
  // 启用深度测试
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  // 设置颜色掩码和深度清除值
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glClearDepthf(1.0f);

  // 加载WebGL扩展
  openGlContext->loadOpenGLExtensions();
  checkGLError("setup OpenGL state");
}

bool GltfRenderer::createFramebuffers() {
  if (!openGlContext) {
    LOGE("Invalid WebGL context");
    return false;
  }

  try {
    // 获取最大多重采样级别
    GLint maxSamples;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    if (samples > maxSamples) {
      samples = maxSamples;
      LOGW("Reduced MSAA samples to %d (max supported)", samples);
    }

    // 创建不透明渲染纹理
    glGenTextures(1, &opaqueRenderTexture);
    glBindTexture(GL_TEXTURE_2D, opaqueRenderTexture);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA8,
                 opaqueFramebufferWidth,
                 opaqueFramebufferHeight,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 创建深度纹理
    glGenTextures(1, &opaqueDepthTexture);
    glBindTexture(GL_TEXTURE_2D, opaqueDepthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_DEPTH_COMPONENT24,
                 opaqueFramebufferWidth,
                 opaqueFramebufferHeight,
                 0,
                 GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT,
                 nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 创建多重采样颜色渲染缓冲区
    glGenRenderbuffers(1, &colorRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                     samples,
                                     GL_RGBA8,
                                     opaqueFramebufferWidth,
                                     opaqueFramebufferHeight);

    // 创建多重采样深度渲染缓冲区
    glGenRenderbuffers(1, &depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                     samples,
                                     GL_DEPTH_COMPONENT24,
                                     opaqueFramebufferWidth,
                                     opaqueFramebufferHeight);

    // 创建多重采样帧缓冲区
    glGenFramebuffers(1, &opaqueFramebufferMSAA);
    glBindFramebuffer(GL_FRAMEBUFFER, opaqueFramebufferMSAA);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER,
                              colorRenderBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER,
                              depthRenderBuffer);

    // 验证多重采样帧缓冲区
    if (!validateFramebuffer(opaqueFramebufferMSAA)) {
      LOGE("MSAA framebuffer is not complete");
      return false;
    }

    // 创建普通帧缓冲区（用于resolve）
    glGenFramebuffers(1, &opaqueFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, opaqueFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           opaqueRenderTexture,
                           0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D,
                           opaqueDepthTexture,
                           0);

    // 验证帧缓冲区
    if (!validateFramebuffer(opaqueFramebuffer)) {
      LOGE("Opaque framebuffer is not complete");
      return false;
    }

    glViewport(0, 0, opaqueFramebufferWidth, opaqueFramebufferHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    checkGLError("create framebuffers");
    return true;

  } catch (const std::exception &e) {
    LOGE("Exception creating framebuffers: %s", e.what());
    destroyFramebuffers();
    return false;
  }
}

void GltfRenderer::initializeDefaultLights() {
//        // 创建主光源
//        // 创建主光源
//        lightKey = std::make_shared<GltfLight>();
//        lightKey->setType(LightType::DIRECTIONAL);
//        lightKey->setIntensity(0.5f);
//        lightKey->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
//
//        // 主光源方向 - 在原有基础上向上偏移
//        glm::quat quatKey = glm::quat(0.8535534, -0.3535534, -0.353553385, -0.146446586);
//        glm::vec3 keyDirection = glm::rotate(quatKey, glm::vec3(0.0f, 1.0f, -1.0f));
//        // 向上偏移调整
//    //    keyDirection.y += -5.0f;  // Y轴向上偏移
//       // keyDirection = glm::normalize(keyDirection);  // 重新归一化
//        lightKey->setDirection(keyDirection);
//
//        // 创建补光源
//        lightFill = std::make_shared<GltfLight>();
//        lightFill->setType(LightType::DIRECTIONAL);
//        lightFill->setIntensity(1.5f);
//        lightFill->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
//
//        // 补光源方向 - 在原有基础上向上偏移
//        glm::quat quatKeye = glm::quat(-0.353553444, -0.8535534, 0.146446645, -0.353553325);
//        glm::vec3 fillDirection = glm::rotate(quatKeye, glm::vec3(0.0f, 1.0f, -1.0f));
//       /* glm::vec3 fillDirection = glm::normalize(glm::vec3(0.0f, 0.5f, -1.0f));  // 也主要从前方
//        fillDirection = glm::normalize(fillDirection);  // 重新归一化*/
//        lightFill->setDirection(fillDirection);
///*
//        // ✨ 创建背部光源 (Back Light / Rim Light)
//        lightBack = std::make_shared<GltfLight>();
//        lightBack->setType(LightType::DIRECTIONAL);
//        lightBack->setIntensity(0.5f);  // 较低强度，用于轮廓照明
//        lightBack->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
//
//// 背部光源方向 - 从后上方照射
//        glm::vec3 backDirection = glm::normalize(glm::vec3(0.0f, -0.3f, 1.0f));  // 从后方向前，稍微向下
//        lightBack->setDirection(backDirection);*/
//
//        LOGI("Initialized default lights with upward adjustment");

  this->lightKey = std::make_unique<GltfLight>();
  this->lightFill = std::make_unique<GltfLight>();

  // 设置补光源强度
  lightFill->setIntensity(0.5f);
  lightKey->setIntensity(1.0f);

  // 定义四元数
  glm::quat quatKey(0.8535534f, -0.3535534f, -0.353553385f, -0.146446586f);
  glm::quat quatFill(-0.353553444f, -0.8535534f, 0.146446645f, -0.353553325f);

  // 初始化方向向量
  this->lightKey->setDirection(glm::vec3(0.0f));
  this->lightFill->setDirection(glm::vec3(0.0f));

  // 使用四元数变换方向向量 [0, 0, -1]
  glm::vec3 defaultDirection(1.0f, 1.0f, -1.0f);
  this->lightKey->setDirection(quatKey * defaultDirection);
  this->lightFill->setDirection(quatFill * defaultDirection);

}


void GltfRenderer::resize(int width, int height) {
  if (currentWidth != width || currentHeight != height) {
    currentWidth = width;
    currentHeight = height;

    if (openGlContext) {
      glViewport(0, 0, width, height);
      logVerbose("Resized viewport to " + std::to_string(width) + "x"
                     + std::to_string(height));
    }
  }
}

void GltfRenderer::clearFrame(const glm::vec4 &clearColor) {
  if (!openGlContext) {
    LOGE("Invalid WebGL context");
    return;
  }

  // 清除默认帧缓冲区
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // 清除不透明帧缓冲区
  glBindFramebuffer(GL_FRAMEBUFFER, opaqueFramebuffer);
  glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // 清除MSAA帧缓冲区
  glBindFramebuffer(GL_FRAMEBUFFER, opaqueFramebufferMSAA);
  glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GltfRenderer::prepareScene(std::shared_ptr<GltfState> state,
                                std::shared_ptr<GltfScene> scene) {
  if (!state || !scene) {
    LOGE("Invalid state or scene");
    return;
  }

  try {
    // 收集场景节点
    nodes = gatherNodes(state, scene);
    // 收集所有可绘制对象
    std::vector<Drawable> allDrawables = collectDrawables(state, nodes);
    // 过滤不透明对象
    std::vector<Drawable>
        opaqueList = filterOpaqueDrawables(allDrawables, state);

    // 分组不透明对象（用于实例化渲染）
    opaqueDrawables = groupDrawables(opaqueList);
    // 过滤透明对象
    transparentDrawables = filterTransparentDrawables(allDrawables, state);
    // 过滤透射对象
    transmissionDrawables = filterTransmissionDrawables(allDrawables, state);
    preparedScene = scene;
  } catch (const std::exception &e) {
    LOGE("Exception preparing scene: %s", e.what());
  }
}


void GltfRenderer::drawScene(std::shared_ptr<GltfState> state,
                             std::shared_ptr<GltfScene> scene) {
  if (!initialized || !openGlContext) {
    LOGE("Renderer not initialized or invalid context");
    return;
  }

  if (!state || !scene) {
    LOGE("Invalid state or scene");
    return;
  }

  try {
    // 准备场景（如果需要）
    if (preparedScene != scene) {
      prepareScene(state, scene);
    }


    // 获取当前相机
    std::shared_ptr<GltfCamera> currentCamera = getCurrentCamera(state);
    if (!currentCamera) {
      LOGE("No camera found");
      return;
    }
    // 计算视口参数
    float aspectOffsetX, aspectOffsetY, aspectWidth, aspectHeight;
    calculateViewportParameters(currentCamera, aspectOffsetX, aspectOffsetY,
                                aspectWidth, aspectHeight);

    // 计算相机矩阵
    calculateCameraMatrices(state, currentCamera);
    // 获取可见光源
    std::vector<int> sceneNodes = scene->getNodes();
    visibleLights = getVisibleLights(state, sceneNodes);

    // 如果没有光源且需要默认光源
    if (visibleLights.empty() && !state->getRenderingParameters().useIBL &&
        state->getRenderingParameters().useDirectionalLightsWithDisabledIBL) {
      visibleLights.emplace_back(nullptr, lightKey);
      visibleLights.emplace_back(nullptr, lightFill);
    }

    // 更新蒙皮动画
    updateSkins(state);
    // 准备实例变换矩阵
    std::vector<std::vector<glm::mat4>>
        instanceTransforms = prepareInstanceTransforms();
    for (auto &instanceTransform: instanceTransforms) {
      if (instanceTransform.empty()) {
        // 添加单位矩阵
        instanceTransform.emplace_back(1.0f);
      }
    }

    // 渲染透射背景（如果有透射对象）
    if (!transmissionDrawables.empty()) {
      renderTransmissionBackground(state, instanceTransforms);
    }

    // 渲染到画布
    renderToCanvas(state,
                   aspectOffsetX,
                   aspectOffsetY,
                   aspectWidth,
                   aspectHeight,
                   instanceTransforms);

  } catch (const std::exception &e) {
    LOGE("Exception drawing scene: %s", e.what());
  }
}

std::shared_ptr<GltfCamera>
GltfRenderer::getCurrentCamera(std::shared_ptr<GltfState> state) {
  std::optional<int> cameraIndex = state->getCameraNodeIndex();

  if (!cameraIndex.has_value() || cameraIndex.value() == -1) {
    // 使用用户相机
    auto userCamera = state->getUserCamera();
    if (userCamera == nullptr) {
      return nullptr;
    }
    if (userCamera->getType() == GltfCamera::Type::PERSPECTIVE) {
      userCamera->getPerspective().aspectRatio =
          static_cast<float>(currentWidth) / static_cast<float>(currentHeight);
    }
    static bool first = true;
    return userCamera;
  } else {
    // 使用场景中的相机
    auto gltf = state->getGltf();
    for (int i = 0; i < gltf->nodes.size(); i++) {
      auto node = gltf->nodes[i];

      if (node->getCamera() == state->getCameraNodeIndex()) {
        int cameraIndex = node->getCamera().value();
        if (cameraIndex < gltf->cameras.size()) {
          auto camera = gltf->cameras[cameraIndex];
          camera->setNode(gltf, cameraIndex);
          return camera;
        }
      }
    }
    return nullptr;
  }
}


void
GltfRenderer::calculateCameraMatrices(std::shared_ptr<GltfState> state,
                                      std::shared_ptr<GltfCamera> currentCamera) {
  float aspectRatio = static_cast<float>(currentWidth) / currentHeight;
  projMatrix = currentCamera->getProjectionMatrix(aspectRatio);
  currentCameraPosition = currentCamera->getPosition(state->getGltf());
  viewMatrix = currentCamera->getViewMatrix(state->getGltf());
  //       currentCameraPosition = glm::vec3(-0.0025f, 0.0f, 4.54f);  // 稍微上方，距离更近
//
//        // 投影矩阵
//        projMatrix = glm::perspective(glm::radians(90.0f), aspectRatio, 0.1f, 1000.0f);

  // 视图矩阵
//        viewMatrix = glm::lookAt(
//                currentCameraPosition,              // 相机位置
//                glm::vec3(0.0f, 0.8f, 0.0f),      // 看向胸部位置（人体中心稍上）
//                glm::vec3(0.0f, 1.0f, 0.0f)       // 上方向
//        );

//       currentCameraPosition = glm::vec3(0.0f, 1.0f, 3.0f);  // 稍微上方，距离更近
//
//        // 投影矩阵
  //    projMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
//
//         //视图矩阵
//        viewMatrix = glm::lookAt(
//                currentCameraPosition,              // 相机位置
//                glm::vec3(0.0f, 1.0f, 0.0f),     // 目标点也下移（看向模型中心）
//                glm::vec3(0.0f, 1.0f, 0.0f)       // 上方向
//        );


  // setupCenteredCamera(state->getGltf());
  //setupCenteredCameraEnhanced(state->getGltf());
  viewProjectionMatrix = projMatrix * viewMatrix;
}

void
GltfRenderer::calculateViewportParameters(std::shared_ptr<GltfCamera> camera,
                                          float &aspectOffsetX,
                                          float &aspectOffsetY,
                                          float &aspectWidth,
                                          float &aspectHeight) {
  aspectWidth = static_cast<float>(currentWidth);
  aspectHeight = static_cast<float>(currentHeight);
  aspectOffsetX = 0.0f;
  aspectOffsetY = 0.0f;

  const float currentAspectRatio = aspectWidth / aspectHeight;

  if (camera->getType() == GltfCamera::Type::PERSPECTIVE) {
    auto perspectiveParams = camera->getPerspective();
    if (perspectiveParams.aspectRatio > 0.0f) {
      if (perspectiveParams.aspectRatio > currentAspectRatio) {
        aspectHeight = aspectWidth / perspectiveParams.aspectRatio.value();
      } else {
        aspectWidth = aspectHeight * perspectiveParams.aspectRatio.value();
      }
    }
  } else if (camera->getType() == GltfCamera::Type::ORTHOGRAPHIC) {
    auto orthoParams = camera->getOrthographic();
    const float orthoAspect = orthoParams.getXmag() / orthoParams.getYmag();
    if (orthoAspect > currentAspectRatio) {
      aspectHeight = aspectWidth / orthoAspect;
    } else {
      aspectWidth = aspectHeight * orthoAspect;
    }
  }

  if (aspectHeight < currentHeight) {
    aspectOffsetY = (currentHeight - aspectHeight) * 0.5f;
  }
  if (aspectWidth < currentWidth) {
    aspectOffsetX = (currentWidth - aspectWidth) * 0.5f;
  }
}

std::vector<std::shared_ptr<GltfNode>>
GltfRenderer::gatherNodes(std::shared_ptr<GltfState> state,
                          std::shared_ptr<GltfScene> scene) {
  return scene->gatherNodes(state->getGltf());
}

std::vector<Drawable>
GltfRenderer::collectDrawables(std::shared_ptr<GltfState> state,
                               const std::vector<std::shared_ptr<GltfNode>> &nodes) {
  std::vector<Drawable> drawables;

  if (!state || !state->getGltf()) {
    LOGE("Invalid state or glTF object");
    return drawables;
  }

  auto gltf = state->getGltf();

  for (auto node: nodes) {
    if (!node || node->getMesh() == -1) {
      continue;
    }

    if (node->getMesh() >= gltf->meshes.size()) {
      LOGW("Invalid mesh index %d for node", node->getMesh().value());
      continue;
    }

    auto mesh = gltf->meshes[node->getMesh().value()];
    if (!mesh) {
      continue;
    }

    for (int primitiveIndex = 0; primitiveIndex < mesh->getPrimitiveCount();
         ++primitiveIndex) {
      auto primitive = mesh->getPrimitives()[primitiveIndex];
      if (!primitive || !primitive->getMaterial().has_value()) {
        continue;
      }

      if (primitive->getMaterial() >= gltf->materials.size()) {
        LOGW("Invalid material index %d for primitive",
             primitive->getMaterial().value());
        continue;
      }

      Drawable drawable(node, primitive, primitiveIndex);
      drawables.push_back(drawable);
    }
  }
  return drawables;
}

std::unordered_map<std::string, InstanceData>
GltfRenderer::groupDrawables(const std::vector<Drawable> &drawables) {
  std::unordered_map<std::string, InstanceData> groups;
  int counter = 0;

  for (const auto &drawable: drawables) {
    if (!drawable.node || !drawable.primitive) {
      continue;
    }

    // 计算变换矩阵的行列式符号（用于确定绕序）
    float winding =
        glm::sign(glm::determinant(drawable.node->getWorldTransform()));

    // 生成基础分组ID
    std::string
        baseId = std::to_string(drawable.node->getMesh().value()) + "_" +
        std::to_string(static_cast<int>(winding)) + "_" +
        std::to_string(drawable.primitiveIndex);

    // 检查是否禁用实例化渲染的条件
    bool disableInstancing =
        drawable.node->getSkin() != -1 ||                           // 有蒙皮
            drawable.primitive->getTargets().size() > 0 ||              // 有变形目标
            drawable.primitive->getAttributes().size() + 4 > maxVertAttributes
            || // 超出顶点属性限制
                !drawable.node->getInstanceMatrices().empty();                       // 已有实例矩阵

    std::string groupId = baseId;
    if (disableInstancing) {
      if (!drawable.node->getInstanceMatrices().empty() &&
          drawable.primitive->getAttributes().size() + 4 > maxVertAttributes) {
        LOGW(
            "EXT_mesh_gpu_instancing disabled for mesh %d because GPU vertex attribute limit is reached",
            drawable.node->getMesh().value());
      }
      groupId += "_" + std::to_string(counter++);
    }

    // 添加到对应分组
    auto &group = groups[groupId];
    if (group.groupId.empty()) {
      group.groupId = groupId;
    }
    group.drawables.push_back(drawable);
  }
  return groups;
}

std::vector<Drawable>
GltfRenderer::filterOpaqueDrawables(const std::vector<Drawable> &drawables,
                                    std::shared_ptr<GltfState> ptr) {
  std::vector<Drawable> opaqueList;

  for (const auto &drawable: drawables) {
    if (!drawable.primitive || drawable.primitive->getMaterial() == -1) {
      continue;
    }

    auto material = getMaterial(drawable.primitive->getMaterial().value(), ptr);
    if (!material) {
      continue;
    }

    // 检查是否为不透明材质
    bool isOpaque = material->getAlphaMode() != AlphaMode::BLEND;

    // 检查是否没有透射扩展
    bool hasTransmission = material->hasTransmissionExtension();

    if (isOpaque && !hasTransmission) {
      opaqueList.push_back(drawable);
    }
  }

  return opaqueList;
}

std::vector<Drawable>
GltfRenderer::filterTransparentDrawables(const std::vector<Drawable> &drawables,
                                         std::shared_ptr<GltfState> ptr) {
  std::vector<Drawable> transparentList;

  for (const auto &drawable: drawables) {
    if (!drawable.primitive || drawable.primitive->getMaterial() == -1) {
      continue;
    }

    auto material = getMaterial(drawable.primitive->getMaterial().value(), ptr);
    if (!material) {
      continue;
    }

    // 检查是否为透明材质且没有透射
    bool isTransparent = material->getAlphaMode() == AlphaMode::BLEND;
    bool hasTransmission = material->hasTransmissionExtension();

    if (isTransparent && !hasTransmission) {
      transparentList.push_back(drawable);
    }
  }

  return transparentList;
}

std::vector<Drawable>
GltfRenderer::filterTransmissionDrawables(const std::vector<Drawable> &drawables,
                                          std::shared_ptr<GltfState> ptr) {
  std::vector<Drawable> transmissionList;

  for (const auto &drawable: drawables) {
    if (!drawable.primitive || drawable.primitive->getMaterial() == -1) {
      continue;
    }

    auto material = getMaterial(drawable.primitive->getMaterial().value(), ptr);
    if (!material) {
      continue;
    }
    // 检查是否有透射扩展
    if (material->hasTransmissionExtension()) {
      transmissionList.push_back(drawable);
    }
  }

  return transmissionList;
}

std::vector<std::pair<std::shared_ptr<GltfNode>, std::shared_ptr<GltfLight>>>
GltfRenderer::getVisibleLights(std::shared_ptr<GltfState> state,
                               const std::vector<int> &nodeIndices) {
  std::vector<std::pair<std::shared_ptr<GltfNode>, std::shared_ptr<GltfLight>>>
      nodeLights;

  if (!state || !state->getGltf()) {
    return nodeLights;
  }

  auto gltf = state->getGltf();

  for (int nodeIndex: nodeIndices) {
    if (nodeIndex < 0 || nodeIndex >= gltf->nodes.size()) {
      continue;
    }

    auto node = gltf->nodes[nodeIndex];
    if (!node) {
      continue;
    }

    // 递归处理子节点
    if (!node->getChildren().empty()) {
      auto childLights = getVisibleLights(state, node->getChildren());
      nodeLights.insert(nodeLights.end(),
                        childLights.begin(),
                        childLights.end());
    }

    // 检查节点是否有光源
    auto lightIndex = node->getLight();
    if (!lightIndex.has_value() || lightIndex == -1) {
      continue;
    }

    if (lightIndex >= gltf->lights.size()) {
      LOGW("Invalid light index %d for node %d", lightIndex, nodeIndex);
      continue;
    }

    auto light = gltf->lights[lightIndex.value()];
    if (light) {
      nodeLights.emplace_back(node, light);
    }
  }
  return nodeLights;
}

void GltfRenderer::updateSkins(std::shared_ptr<GltfState> state) {
  if (!state->getRenderingParameters().skinning) {
    return;
  }

  auto gltf = state->getGltf();
  if (!gltf || gltf->skins.empty()) {
    return;
  }

  for (auto node: nodes) {
    if (node && node->getMesh() != -1 && node->getSkin() != -1) {
      updateSkin(state, node);
    }
  }
}

void GltfRenderer::updateSkin(std::shared_ptr<GltfState> state,
                              std::shared_ptr<GltfNode> node) {
  if (!state->getRenderingParameters().skinning || !state->getGltf()) {
    return;
  }

  auto gltf = state->getGltf();
  if (node->getSkin() >= gltf->skins.size()) {
    LOGW("Invalid skin index %d", node->getSkin().value());
    return;
  }

  auto skin = gltf->skins[node->getSkin().value()];
  if (skin && openGlContext) {
    skin->computeJoints(gltf, openGlContext);
  }
}

std::vector<std::vector<glm::mat4>> GltfRenderer::prepareInstanceTransforms() {
  std::vector<std::vector<glm::mat4>> instanceTransforms;

  for (const auto &[groupId, instanceData]: opaqueDrawables) {
    std::vector<glm::mat4> transforms;

    if (instanceData.drawables.size() > 1) {
      // 多个实例，收集所有变换矩阵
      for (const auto &drawable: instanceData.drawables) {
        if (drawable.node) {
          transforms.push_back(drawable.node->getWorldTransform());
        }
      }
    } else if (!instanceData.drawables.empty() &&
        instanceData.drawables[0].node &&
        !instanceData.drawables[0].node->getInstanceMatrices().empty()) {
      // EXT_mesh_gpu_instancing扩展
      if (instanceData.drawables[0].primitive->getAttributes().size() + 4
          <= maxVertAttributes) {
        transforms =
            instanceData.drawables[0].node->getInstanceWorldTransforms();
      }
    } else {
      if (!instanceData.drawables.empty() && instanceData.drawables[0].node) {
        transforms.push_back(instanceData.drawables[0].node->getWorldTransform());
      } else {
        transforms.push_back(glm::mat4(1.0f));
      }
    }


    instanceTransforms.push_back(transforms);
  }

  return instanceTransforms;
}

void
GltfRenderer::renderTransmissionBackground(std::shared_ptr<GltfState> state,
                                           const std::vector<std::vector<glm::mat4>> &instanceTransforms) {
  // 绑定MSAA帧缓冲区
  glBindFramebuffer(GL_FRAMEBUFFER, opaqueFramebufferMSAA);
  glViewport(0, 0, opaqueFramebufferWidth, opaqueFramebufferHeight);

  // 渲染环境贴图作为背景
  std::vector<std::string> envDefines = {"LINEAR_OUTPUT 1"};
  environmentRenderer->drawEnvironmentMap(openGlContext,
                                          viewProjectionMatrix,
                                          state,
                                          shaderCache,
                                          envDefines);

  // 渲染不透明对象
  int drawableCounter = 0;
  for (const auto &[groupId, instanceData]: opaqueDrawables) {
    if (!instanceData.drawables.empty()) {
      const auto &drawable = instanceData.drawables[0];
      RenderPassConfiguration config;
      config.linearOutput = true;

      const std::vector<glm::mat4> *instanceOffset = nullptr;
      if (drawableCounter < instanceTransforms.size() &&
          !instanceTransforms[drawableCounter].empty()) {
        instanceOffset = &instanceTransforms[drawableCounter];
      }

      drawPrimitive(state, config, drawable.primitive, drawable.node,
                    viewProjectionMatrix, 0, instanceOffset);
    }
    drawableCounter++;
  }

  // 渲染透明对象
  std::vector<Drawable>
      sortedTransparent = sortDrawablesByDepth(transparentDrawables, state);
  for (const auto &drawable: sortedTransparent) {
    if (drawable.depth <= 0) {
      RenderPassConfiguration config;
      config.linearOutput = true;
      drawPrimitive(state,
                    config,
                    drawable.primitive,
                    drawable.node,
                    viewProjectionMatrix);
    }
  }

  // Blit MSAA帧缓冲区到普通纹理
  blitFramebuffers();

  // 生成mipmap
  glBindTexture(GL_TEXTURE_2D, opaqueRenderTexture);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
}


void GltfRenderer::renderToCanvas(std::shared_ptr<GltfState> state,
                                  float aspectOffsetX, float aspectOffsetY,
                                  float aspectWidth, float aspectHeight,
                                  const std::vector<std::vector<glm::mat4>> &instanceTransforms) {
  // 绑定默认帧缓冲区
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(static_cast<GLint>(aspectOffsetX),
             static_cast<GLint>(aspectOffsetY),
             static_cast<GLsizei>(aspectWidth),
             static_cast<GLsizei>(aspectHeight));

//        // 渲染环境贴图
  std::vector<std::string> fragDefines;
  pushFragParameterDefines(fragDefines, state);
  if (environmentRenderer) {
    environmentRenderer->drawEnvironmentMap(openGlContext,
                                            viewProjectionMatrix,
                                            state,
                                            shaderCache,
                                            fragDefines);

  } else {
    fragDefines.push_back("USE_NORMAL_LIGHT 1");
  }


  // 渲染不透明对象
  int drawableCounter = 0;
  for (const auto &[groupId, instanceData]: opaqueDrawables) {
    if (!instanceData.drawables.empty()) {
      const auto &drawable = instanceData.drawables[0];
      RenderPassConfiguration config;
      config.linearOutput = false;

      const std::vector<glm::mat4> *instanceOffset = nullptr;
      if (drawableCounter < instanceTransforms.size() &&
          !instanceTransforms[drawableCounter].empty()) {
        instanceOffset = &instanceTransforms[drawableCounter];
      }

      drawPrimitive(state, config, drawable.primitive, drawable.node,
                    viewProjectionMatrix, 0, instanceOffset);
    }
    drawableCounter++;
  }

//        // 渲染透射对象
  auto camera = getCurrentCamera(state);
  std::vector<Drawable>
      sortedTransmission = sortDrawablesByDepth(transmissionDrawables, state);
  for (const auto &drawable: sortedTransmission) {
    if (drawable.depth <= 0) {
      RenderPassConfiguration config;
      config.linearOutput = false;
      drawPrimitive(state, config, drawable.primitive, drawable.node,
                    viewProjectionMatrix, opaqueRenderTexture);
    }
  }
  // 渲染透明对象
  std::vector<Drawable>
      sortedTransparent = sortDrawablesByDepth(transparentDrawables, state);
  for (const auto &drawable: sortedTransparent) {
    if (drawable.depth <= 0) {
      RenderPassConfiguration config;
      config.linearOutput = false;
      drawPrimitive(state,
                    config,
                    drawable.primitive,
                    drawable.node,
                    viewProjectionMatrix);
    }
  }
}


void GltfRenderer::drawPrimitive(std::shared_ptr<GltfState> state,
                                 const RenderPassConfiguration &config,
                                 std::shared_ptr<GltfPrimitive> primitive,
                                 std::shared_ptr<GltfNode> node,
                                 const glm::mat4 &viewProjectionMatrix,
                                 GLuint transmissionSampleTexture,
                                 const std::vector<glm::mat4> *instanceOffset) {
  if (!state || !primitive || !node) {
    LOGE("Invalid parameters for drawPrimitive");
    return;
  }
  // 修正变换测试
  if (primitive->skip) {
    logVerbose("Skipping primitive");
    return;
  }
  try {
    // 获取材质（考虑变体）
    auto material = getMaterialWithVariant(state, primitive);
    if (!material) {
      LOGW("No valid material found for primitive");
      return;
    }

    // 选择着色器排列组合
    auto [vertexHash, fragmentHash] =
        selectShaderPermutation(state,
                                config,
                                primitive,
                                node,
                                material,
                                instanceOffset);
    if (vertexHash == 0 || fragmentHash == 0) {
      LOGW("Failed to select shader permutation");
      return;
    }

    // 获取着色器程序
    shader = shaderCache->getShaderProgram(vertexHash, fragmentHash);
    if (!shader) {
      LOGW("Failed to get shader program");
      return;
    }

    // 使用着色器程序
    glUseProgram(shader->getProgram());
    shaderSwitches++;
    GLint
        tt = glGetUniformLocation(shader->getProgram(), "u_OcclusionStrength");
    LOGE("Failed to glGetUniformLocation=== shader program, tt: %d, shader: %d",
         tt,
         shader->getProgram());

    // 应用光照
    if (state->getRenderingParameters().usePunctual) {
      applyLights();
    }
    // 更新通用uniform变量
    updateCommonUniforms(state, node, viewProjectionMatrix);
//
//            // 更新动画uniform变量
    updateAnimationUniforms(state, node, primitive);
//
//            // 设置渲染状态
    setupRenderState(material, node);

    // 绑定顶点属性
    int vertexCount = bindVertexAttributes(state, primitive, instanceOffset);
    if (vertexCount <= 0) {
      LOGW("No valid vertex data");
      return;
    }

    // 更新材质uniform变量和纹理
    int textureSlotOffset =
        updateMaterialUniforms(state, material, 0, primitive, node);

    // 应用环境贴图
    textureSlotOffset = applyEnvironmentMap(state, textureSlotOffset);

    // 处理透射采样纹理
    if (transmissionSampleTexture != 0 && state->getEnvironment() &&
        state->getRenderingParameters().enabledExtensions.KHR_materials_transmission) {

      bindTransmissionSampleTexture(transmissionSampleTexture,
                                    textureSlotOffset,
                                    node);
      textureSlotOffset++;
    }

    // 执行绘制调用
    executeDrawCall(primitive, vertexCount, instanceOffset, state);

//            // 解除顶点属性绑定
    unbindVertexAttributes(primitive, instanceOffset);
    glDepthMask(GL_TRUE);  // 关键：透明对象不写入深度缓冲

    renderedPrimitives++;
    drawCallCount++;

  } catch (const std::exception &e) {
    LOGE("Exception in drawPrimitive: %s", e.what());
  }
}

std::shared_ptr<GltfMaterial>
GltfRenderer::getMaterialWithVariant(std::shared_ptr<GltfState> state,
                                     std::shared_ptr<GltfPrimitive> primitive) {
  if (!state || !primitive || primitive->getMaterial() == -1) {
    return nullptr;
  }

  auto gltf = state->getGltf();
  if (!gltf || primitive->getMaterial() >= gltf->materials.size()) {
    return nullptr;
  }

  // 检查是否有材质变体
  if (!primitive->getMappings().empty() && state->getVariant() != "default") {
//            const auto &variantNames = gltf->getVariantNames();
//            auto it = std::find(variantNames.begin(), variantNames.end(), state->getVariant());
//
//            if (it != variantNames.end()) {
//                int variantIndex = static_cast<int>(std::distance(variantNames.begin(), it));
//
//                // 查找匹配的材质映射
//                for (const auto &mapping: primitive->getMappings()) {
//                    auto variantIt = std::find(mapping.variants.begin(), mapping.variants.end(), variantIndex);
//                    if (variantIt != mapping.variants.end()) {
//                        if (mapping.material < gltf->materials.size()) {
//                            return gltf->materials[mapping.material];
//                        }
//                    }
//                }
//            }
  }

  return gltf->materials[primitive->getMaterial().value()];
}

std::pair<size_t, size_t>
GltfRenderer::selectShaderPermutation(std::shared_ptr<GltfState> state,
                                      const RenderPassConfiguration &config,
                                      std::shared_ptr<GltfPrimitive> primitive,
                                      std::shared_ptr<GltfNode> node,
                                      std::shared_ptr<GltfMaterial> material,
                                      const std::vector<glm::mat4> *instanceOffset) {
  // 生成顶点着色器定义
  std::vector<std::string> vertDefines = primitive->getDefines();
  pushVertParameterDefines(vertDefines,
                           state->getRenderingParameters(),
                           state,
                           node,
                           primitive);
  if (instanceOffset) {
    vertDefines.push_back("USE_INSTANCING 1");
  }

  // 处理纹理变换
  if (!material->getTextureTransforms().empty()) {
    for (const auto &transform: material->getTextureTransforms()) {
      if (transform.key == "Normal") {
        vertDefines.push_back("HAS_VERT_NORMAL_UV_TRANSFORM 1");
        break;
      }
    }
  }

  // 生成片段着色器定义
  std::vector<std::string>
      fragDefines = material->getDefines(state->getRenderingParameters());
  fragDefines.insert(fragDefines.end(), vertDefines.begin(), vertDefines.end());

  if (config.linearOutput) {
    fragDefines.push_back("LINEAR_OUTPUT 1");
  }

  // 处理非三角形图元
  if (primitive->getMode() < 4) { // POINTS, LINES, LINE_LOOP, LINE_STRIP
    fragDefines.push_back("NOT_TRIANGLE 1");
    if (primitive->hasNormals() && !primitive->hasTangents()) {
      // 移除法线贴图相关定义
      fragDefines.erase(std::remove_if(fragDefines.begin(), fragDefines.end(),
                                       [](const std::string &def) {
                                         return def == "HAS_NORMAL_MAP 1" ||
                                             def
                                                 == "HAS_CLEARCOAT_NORMAL_MAP 1";
                                       }), fragDefines.end());
    }
  }

  pushFragParameterDefines(fragDefines, state);

  // 选择片段着色器类型
  std::string fragmentShaderType = (material->getType() == MaterialType::SG) ?
                                   "specular_glossiness.frag" : "pbr.frag";

  // 编译着色器
  size_t
      fragmentHash = shaderCache->selectShader(fragmentShaderType, fragDefines);
  size_t vertexHash = shaderCache->selectShader("primitive.vert", vertDefines);

  return {vertexHash, fragmentHash};
}

void GltfRenderer::updateCommonUniforms(std::shared_ptr<GltfState> state,
                                        std::shared_ptr<GltfNode> node,
                                        const glm::mat4 &viewProjectionMatrix) {
  if (!shader) return;
  // 更新变换矩阵
  shader->updateUniform("u_ViewProjectionMatrix", viewProjectionMatrix);
  shader->updateUniform("u_ModelMatrix", node->getWorldTransform());
  shader->updateUniform("u_NormalMatrix", node->getNormalMatrix(), false);

  // 更新相机和曝光
  shader->updateUniform("u_Exposure",
                        state->getRenderingParameters().exposure,
                        false);
  shader->updateUniform("u_Camera", currentCameraPosition, false);
  logVerbose("Updated common uniforms");
}

void GltfRenderer::setupRenderState(std::shared_ptr<GltfMaterial> material,
                                    std::shared_ptr<GltfNode> node) {
  if (!material || !node) return;

  // 设置正面方向（根据变换矩阵的行列式）
  if (glm::determinant(node->getWorldTransform()) < 0.0f) {
    glFrontFace(GL_CW);
  } else {
    glFrontFace(GL_CCW);
  }


  // 设置面剔除
  if (material->isDoubleSided()) {
    glDisable(GL_CULL_FACE);
  } else {
    glEnable(GL_CULL_FACE);
  }

  // 设置混合模式
  if (material->getAlphaMode() == AlphaMode::BLEND) {
    glDepthMask(GL_FALSE);  // 关键：透明对象不写入深度缓冲

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA,
                        GL_ONE_MINUS_SRC_ALPHA,
                        GL_ONE,
                        GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  } else {
    glDisable(GL_BLEND);

  }

  checkGLError("setup render state");
}

int GltfRenderer::bindVertexAttributes(std::shared_ptr<GltfState> state,
                                       std::shared_ptr<GltfPrimitive> primitive,
                                       const std::vector<glm::mat4> *instanceOffset) {
  if (!shader || !state || !primitive) {
    return 0;
  }

  auto gltf = state->getGltf();
  if (!gltf) {
    return 0;
  }

  // 绑定索引缓冲区
  if (primitive->getIndices() != -1) {
    if (!openGlContext->setIndices(gltf, primitive->getIndices().value())) {
      LOGW("Failed to bind index buffer");
      return 0;
    }
  }

  int vertexCount = 0;

  // 绑定顶点属性
  for (const auto &attribute: primitive->getGLAttributes()) {
    if (attribute.accessor >= gltf->accessors.size()) {
      LOGW("Invalid accessor index %d", attribute.accessor);
      continue;
    }

    auto accessor = gltf->accessors[attribute.accessor];
    vertexCount = accessor->getCount().value();

    GLint location = shader->getAttributeLocation(attribute.name);
    if (location == -1) {
      logVerbose("Attribute " + attribute.name + " not found in shader");
      continue;
    }

    if (!openGlContext->enableAttribute(gltf, location, accessor)) {
      LOGW("Failed to enable attribute %s", attribute.name.c_str());
      return 0;
    }
  }

  // 处理实例化属性
  if (instanceOffset && !instanceOffset->empty()) {
    bindInstanceBuffer(*instanceOffset);
  }

  return vertexCount;
}

void
GltfRenderer::bindInstanceBuffer(const std::vector<glm::mat4> &instanceTransforms) {
  if (!shader || instanceTransforms.empty()) {
    return;
  }

  GLint location = shader->getAttributeLocation("a_instance_model_matrix");
  if (location == -1) {
    LOGW("Instance matrix attribute not found");
    return;
  }

  // 创建实例缓冲区（如果需要）
  if (instanceBuffer == 0) {
    glGenBuffers(1, &instanceBuffer);
  }

  // 启用实例矩阵的4个vec4属性
  glEnableVertexAttribArray(location);
  glEnableVertexAttribArray(location + 1);
  glEnableVertexAttribArray(location + 2);
  glEnableVertexAttribArray(location + 3);

  // 上传实例数据
  glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
  glBufferData(GL_ARRAY_BUFFER,
               instanceTransforms.size() * sizeof(glm::mat4),
               instanceTransforms.data(),
               GL_DYNAMIC_DRAW);

  // 设置实例属性指针
  const int stride = sizeof(glm::mat4);
  glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (void *) 0);
  glVertexAttribPointer(location + 1,
                        4,
                        GL_FLOAT,
                        GL_FALSE,
                        stride,
                        (void *) (sizeof(glm::vec4)));
  glVertexAttribPointer(location + 2,
                        4,
                        GL_FLOAT,
                        GL_FALSE,
                        stride,
                        (void *) (2 * sizeof(glm::vec4)));
  glVertexAttribPointer(location + 3,
                        4,
                        GL_FLOAT,
                        GL_FALSE,
                        stride,
                        (void *) (3 * sizeof(glm::vec4)));

  // 设置实例分割器
  glVertexAttribDivisor(location, 1);
  glVertexAttribDivisor(location + 1, 1);
  glVertexAttribDivisor(location + 2, 1);
  glVertexAttribDivisor(location + 3, 1);

  logVerbose(
      "Bound instance buffer with " + std::to_string(instanceTransforms.size())
          + " instances");
}

void
GltfRenderer::unbindVertexAttributes(std::shared_ptr<GltfPrimitive> primitive,
                                     const std::vector<glm::mat4> *instanceOffset) {
  if (!shader || !primitive) {
    return;
  }

  // 禁用顶点属性
  for (const auto &attribute: primitive->getGLAttributes()) {
    GLint location = shader->getAttributeLocation(attribute.name);
    if (location != -1) {
      glDisableVertexAttribArray(location);
    }
  }

  // 禁用实例属性
  if (instanceOffset && !instanceOffset->empty()) {
    unbindInstanceBuffer();
  }
}

void GltfRenderer::unbindInstanceBuffer() {
  if (!shader) {
    return;
  }

  GLint location = shader->getAttributeLocation("a_instance_model_matrix");
  if (location != -1) {
    glVertexAttribDivisor(location, 0);
    glVertexAttribDivisor(location + 1, 0);
    glVertexAttribDivisor(location + 2, 0);
    glVertexAttribDivisor(location + 3, 0);

    glDisableVertexAttribArray(location);
    glDisableVertexAttribArray(location + 1);
    glDisableVertexAttribArray(location + 2);
    glDisableVertexAttribArray(location + 3);
  }
}

int GltfRenderer::updateMaterialUniforms(std::shared_ptr<GltfState> state,
                                         std::shared_ptr<GltfMaterial> material,
                                         int textureSlotOffset,
                                         std::shared_ptr<GltfPrimitive> ptr,
                                         std::shared_ptr<GltfNode> sharedPtr) {
  if (!shader || !material || !state) {
    return textureSlotOffset;
  }

  auto gltf = state->getGltf();
  if (!gltf) {
    return textureSlotOffset;
  }

  // 更新材质纹理变换
  material->updateTextureTransforms(shader);

  // 基础材质属性
  shader->updateUniform("u_EmissiveFactor", material->getEmissiveFactor());
  shader->updateUniform("u_AlphaCutoff", material->getAlphaCutoff());

  // 法线贴图属性
  auto normalTexture = material->getNormalTexture();
  if (normalTexture) {
    shader->updateUniform("u_NormalScale", normalTexture->getScale());
    shader->updateUniform("u_NormalUVSet", normalTexture->getTexCoord());
  }

  // 遮挡贴图属性
  auto occlusionTexture = material->getOcclusionTexture();
  if (occlusionTexture) {
    shader->updateUniform("u_OcclusionStrength",
                          occlusionTexture->getStrength());
    shader->updateUniform("u_OcclusionUVSet", occlusionTexture->getTexCoord());
  }

  // 自发光贴图属性
  auto emissiveTexture = material->getEmissiveTexture();
  if (emissiveTexture) {
    shader->updateUniform("u_EmissiveUVSet", emissiveTexture->getTexCoord());
  }

  // PBR金属粗糙度属性
  auto pbrMR = material->getPbrMetallicRoughness();
  if (pbrMR) {
    shader->updateUniform("u_BaseColorFactor", pbrMR->getBaseColorFactor());
    shader->updateUniform("u_MetallicFactor", pbrMR->getMetallicFactor());
    shader->updateUniform("u_RoughnessFactor", pbrMR->getRoughnessFactor());

    if (pbrMR->getBaseColorTexture()) {
      shader->updateUniform("u_BaseColorUVSet",
                            pbrMR->getBaseColorTexture()->getTexCoord());
    }
    if (pbrMR->getMetallicRoughnessTexture()) {
      shader->updateUniform("u_MetallicRoughnessUVSet",
                            pbrMR->getMetallicRoughnessTexture()->getTexCoord());
    }
  }

  // 更新扩展属性
  updateExtensionUniforms(material);

  // 绑定纹理
  int currentTextureSlot = textureSlotOffset;
  const auto &textures = material->getTextures();
  for (const auto &textureInfo: textures) {
    GLint location = shader->getUniformLocation(textureInfo->getSamplerName());
    if (location != -1) {
      if (openGlContext->setTexture(location, gltf, textureInfo, currentTextureSlot)) {
        currentTextureSlot++;
        textureBinds++;
      }
    }
  }

  // 绑定变形目标纹理
  auto morphTargetTexture = ptr->getMorphTargetTextureInfo();
  if (morphTargetTexture) {
    GLint location =
        shader->getUniformLocation(morphTargetTexture->getSamplerName());
    if (location != -1) {
      openGlContext->setTexture(location, gltf, morphTargetTexture, currentTextureSlot);
      currentTextureSlot++;
    }
  }

  // 绑定关节纹理
  currentTextureSlot = bindJointTexture(state, currentTextureSlot, sharedPtr);

  return currentTextureSlot;
}

void
GltfRenderer::updateExtensionUniforms(std::shared_ptr<GltfMaterial> material) {
  if (!shader || !material) {
    return;
  }

  // 各向异性扩展
//        auto anisotropy = material->hasAnisotropyExtension();
//        if (anisotropy) {
//            shader->updateUniform("u_AnisotropyUVSet", anisotropy->anisotropyTexture ?
//                                                       anisotropy->anisotropyTexture->texCoord : -1);
//
//            float factor = anisotropy->anisotropyStrength;
//            float rotation = anisotropy->anisotropyRotation;
//            glm::vec3 anisotropyVec(std::cos(rotation), std::sin(rotation), factor);
//            shader->updateUniform("u_Anisotropy", anisotropyVec);
//        }
//
//        // 清漆扩展
//        auto clearcoat = material->hasClearcoatExtension();
//        if (clearcoat) {
//            shader->updateUniform("u_ClearcoatFactor", clearcoat->clearcoatFactor);
//            shader->updateUniform("u_ClearcoatRoughnessFactor", clearcoat->clearcoatRoughnessFactor);
//            shader->updateUniform("u_ClearcoatUVSet", clearcoat->clearcoatTexture ?
//                                                      clearcoat->clearcoatTexture->texCoord : -1);
//            shader->updateUniform("u_ClearcoatRoughnessUVSet", clearcoat->clearcoatRoughnessTexture ?
//                                                               clearcoat->clearcoatRoughnessTexture->texCoord : -1);
//            shader->updateUniform("u_ClearcoatNormalUVSet", clearcoat->clearcoatNormalTexture ?
//                                                            clearcoat->clearcoatNormalTexture->texCoord : -1);
//            shader->updateUniform("u_ClearcoatNormalScale", clearcoat->clearcoatNormalTexture ?
//                                                            clearcoat->clearcoatNormalTexture->scale : 1.0f);
//        }
//
//        // 色散扩展
//        auto dispersion = material->hasDispersionExtension();
//        if (dispersion) {
//            shader->updateUniform("u_Dispersion", dispersion->dispersion);
//        }
//
//        // 自发光强度扩展
//        auto emissiveStrength = material->hasEmissiveStrengthExtension();
//        if (emissiveStrength) {
//            shader->updateUniform("u_EmissiveStrength", emissiveStrength->emissiveStrength);
//        }
//
//        // 折射率扩展
//        auto ior = material->hasIorExtension();
//        if (ior) {
//            shader->updateUniform("u_Ior", ior->ior);
//        }
//
//        // 虹彩扩展
//        auto iridescence = material->hasIridescenceExtension();
//        if (iridescence) {
//            shader->updateUniform("u_IridescenceFactor", iridescence->iridescenceFactor);
//            shader->updateUniform("u_IridescenceIor", iridescence->iridescenceIor);
//            shader->updateUniform("u_IridescenceThicknessMaximum", iridescence->iridescenceThicknessMaximum);
//            shader->updateUniform("u_IridescenceThicknessMinimum", iridescence->iridescenceThicknessMinimum);
//            shader->updateUniform("u_IridescenceUVSet", iridescence->iridescenceTexture ?
//                                                        iridescence->iridescenceTexture->texCoord : -1);
//            shader->updateUniform("u_IridescenceThicknessUVSet", iridescence->iridescenceThicknessTexture ?
//                                                                 iridescence->iridescenceThicknessTexture->texCoord
//                                                                                                          : -1);
//        }
//
//        // 光泽扩展
//        auto sheen = material->hasSheenExtension();
//        if (sheen) {
//            shader->updateUniform("u_SheenRoughnessFactor", sheen->sheenRoughnessFactor);
//            shader->updateUniform("u_SheenColorFactor", sheen->sheenColorFactor);
//            shader->updateUniform("u_SheenRoughnessUVSet", sheen->sheenRoughnessTexture ?
//                                                           sheen->sheenRoughnessTexture->texCoord : -1);
//            shader->updateUniform("u_SheenColorUVSet", sheen->sheenColorTexture ?
//                                                       sheen->sheenColorTexture->texCoord : -1);
//        }
//
//        // 镜面反射扩展
//        auto specular = material->hasSpecularExtension();
//        if (specular) {
//            shader->updateUniform("u_KHR_materials_specular_specularColorFactor", specular->specularColorFactor);
//            shader->updateUniform("u_KHR_materials_specular_specularFactor", specular->specularFactor);
//            shader->updateUniform("u_SpecularUVSet", specular->specularTexture ?
//                                                     specular->specularTexture->texCoord : -1);
//            shader->updateUniform("u_SpecularColorUVSet", specular->specularColorTexture ?
//                                                          specular->specularColorTexture->texCoord : -1);
//        }
//
//        // 透射扩展
//        auto transmission = material->hasTransmissionExtension();
//        if (transmission) {
//            shader->updateUniform("u_TransmissionFactor", transmission->transmissionFactor);
//            shader->updateUniform("u_TransmissionUVSet", transmission->transmissionTexture ?
//                                                         transmission->transmissionTexture->texCoord : -1);
//        }
//
//        // 体积扩展
//        auto volume = material->hasVolumeExtension();
//        if (volume) {
//            shader->updateUniform("u_AttenuationColor", volume->attenuationColor);
//            shader->updateUniform("u_AttenuationDistance", volume->attenuationDistance);
//            shader->updateUniform("u_ThicknessFactor", volume->thicknessFactor);
//            shader->updateUniform("u_ThicknessUVSet", volume->thicknessTexture ?
//                                                      volume->thicknessTexture->texCoord : -1);
//        }
//
//        // 漫反射透射扩展
//        auto diffuseTransmission = material->hasDiffuseTransmissionExtension();
//        if (diffuseTransmission) {
//            shader->updateUniform("u_DiffuseTransmissionFactor", diffuseTransmission->diffuseTransmissionFactor);
//            shader->updateUniform("u_DiffuseTransmissionColorFactor",
//                                  diffuseTransmission->diffuseTransmissionColorFactor);
//            shader->updateUniform("u_DiffuseTransmissionUVSet", diffuseTransmission->diffuseTransmissionTexture ?
//                                                                diffuseTransmission->diffuseTransmissionTexture->texCoord
//                                                                                                                : -1);
//            shader->updateUniform("u_DiffuseTransmissionColorUVSet",
//                                  diffuseTransmission->diffuseTransmissionColorTexture ?
//                                  diffuseTransmission->diffuseTransmissionColorTexture->texCoord : -1);
//        }
//
//        // 镜面光泽度扩展
  auto specularGlossiness = material->hasSpecularExtension();
  /*     if (specularGlossiness) {
           shader->updateUniform("u_DiffuseFactor", specularGlossiness->diffuseFactor);
           shader->updateUniform("u_SpecularFactor", specularGlossiness->specularFactor);
           shader->updateUniform("u_GlossinessFactor", specularGlossiness->glossinessFactor);
           shader->updateUniform("u_SpecularGlossinessUVSet", specularGlossiness->specularGlossinessTexture ?
                                                              specularGlossiness->specularGlossinessTexture->texCoord
                                                                                                            : -1);
           shader->updateUniform("u_DiffuseUVSet", specularGlossiness->diffuseTexture ?
                                                   specularGlossiness->diffuseTexture->texCoord : -1);
       }*/
}

int
GltfRenderer::bindJointTexture(std::shared_ptr<GltfState> state,
                               int textureSlot,
                               std::shared_ptr<GltfNode> ptr) {
  if (!state->getRenderingParameters().skinning || !shader) {
    return textureSlot;
  }

  // 这里需要根据当前渲染的节点来获取皮肤信息
  // 简化处理，实际应该传递节点信息
  auto gltf = state->getGltf();
  if (!gltf || gltf->skins.empty()) {
    return textureSlot;
  }

  // 检查活动的uniform
  GLint uniformCount;
  glGetProgramiv(shader->getProgram(), GL_ACTIVE_UNIFORMS, &uniformCount);
  auto skin = gltf->skins[ptr->getSkin().value()];
  if (skin && skin->getJointTextureInfo()) {
    auto name = skin->getJointTextureInfo()->getSamplerName();
    GLint location = shader->getUniformLocation(name);
    if (location != -1) {
      openGlContext->setTexture(location,
                                gltf,
                                skin->getJointTextureInfo(),
                                textureSlot);
      return textureSlot + 1;
    }
  }

  return textureSlot;
}

void GltfRenderer::bindTransmissionSampleTexture(GLuint transmissionTexture,
                                                 int textureSlot,
                                                 std::shared_ptr<GltfNode> node) {
  if (!shader || transmissionTexture == 0) {
    return;
  }

  // 绑定透射采样纹理
  glActiveTexture(GL_TEXTURE0 + textureSlot);
  glBindTexture(GL_TEXTURE_2D, opaqueRenderTexture);

  GLint
      location = shader->getUniformLocation("u_TransmissionFramebufferSampler");
  if (location != -1) {
    glUniform1i(location, textureSlot);
  }

  // 设置帧缓冲区大小
  location = shader->getUniformLocation("u_TransmissionFramebufferSize");
  if (location != -1) {
    glUniform2i(location, opaqueFramebufferWidth, opaqueFramebufferHeight);
  }

  // 设置额外的变换矩阵（用于透射计算）
  shader->updateUniform("u_ModelMatrix", node->getWorldTransform());
  shader->updateUniform("u_ViewMatrix", viewMatrix);
  shader->updateUniform("u_ProjectionMatrix", projMatrix);

  textureBinds++;
}


void GltfRenderer::executeDrawCall(std::shared_ptr<GltfPrimitive> primitive,
                                   int vertexCount,
                                   const std::vector<glm::mat4> *instanceOffset,
                                   std::shared_ptr<GltfState> ptr) {
  if (!primitive || vertexCount <= 0) {
    return;
  }

  try {
    bool drawIndexed = primitive->getIndices() != -1;
    bool isInstanced = instanceOffset && !instanceOffset->empty();
    if (drawIndexed) {
      // 索引绘制
      auto gltf = ptr->getGltf(); // 需要获取当前glTF对象
      if (!gltf || primitive->getIndices() >= gltf->accessors.size()) {
        LOGW("Invalid indices accessor");
        return;
      }

      auto
          indexAccessor = gltf->getAccessors()[primitive->getIndices().value()];
      if (!indexAccessor) {
        LOGW("Invalid index accessor");
        return;
      }

      if (isInstanced) {
        glDrawElementsInstanced(primitive->getMode(),
                                indexAccessor->getCount().value(),
                                indexAccessor->getComponentType().value(),
                                0,
                                static_cast<GLsizei>(instanceOffset->size()));
        logVerbose("Drew " + std::to_string(instanceOffset->size())
                       + " indexed instances");
      } else {

        glDrawElements(primitive->getMode(),
                       indexAccessor->getCount().value(),
                       indexAccessor->getComponentType().value(),
                       0);
      }
    } else {
      // 数组绘制
      if (isInstanced) {
        glDrawArraysInstanced(primitive->getMode(),
                              0,
                              vertexCount,
                              static_cast<GLsizei>(instanceOffset->size()));
      } else {
        glDrawArrays(primitive->getMode(), 0, vertexCount);
      }
    }

    checkGLError("execute draw call");

  } catch (const std::exception &e) {
    LOGE("Exception in executeDrawCall: %s", e.what());
  }
}

std::vector<Drawable>
GltfRenderer::sortDrawablesByDepth(std::vector<Drawable> &drawables,
                                   std::shared_ptr<GltfState> state) {
  // 计算每个可绘制对象的深度
  for (auto &drawable: drawables) {
    drawable.depth = calculateDistanceToCamera(drawable);
  }
  // 按深度排序（远到近，用于透明物体）
  std::sort(drawables.begin(), drawables.end(),
            [](const Drawable &a, const Drawable &b) {
              return a.depth < b.depth;
            });
  return drawables;
}

float GltfRenderer::calculateDistanceToCamera(const Drawable &drawable) {
  if (!drawable.node || !drawable.primitive) {
    return std::numeric_limits<float>::max(); // 无效对象排到最后
  }
  // 获取局部重心
  glm::vec3 centroid = drawable.primitive->getCentroid();
  // 变换到世界坐标
  glm::mat4 worldTransform = drawable.node->getWorldTransform();
  glm::vec3 worldPos = glm::vec3(worldTransform * glm::vec4(centroid, 1.0f));
  // 关键：变换到视图空间并获取深度值
  glm::vec4 viewPos = viewMatrix * glm::vec4(worldPos, 1.0f);
  // 返回视图空间的Z深度
  return viewPos.z;
}

void
GltfRenderer::pushVertParameterDefines(std::vector<std::string> &vertDefines,
                                       const RenderingParameters &parameters,
                                       std::shared_ptr<GltfState> state,
                                       std::shared_ptr<GltfNode> node,
                                       std::shared_ptr<GltfPrimitive> primitive) {
  if (!node || !primitive || !state) {
    return;
  }

  auto gltf = state->getGltf();
  if (!gltf) {
    return;
  }

  // 蒙皮
  if (parameters.skinning && node->getSkin() != -1 &&
      primitive->hasWeights() && primitive->hasJoints()) {
    vertDefines.push_back("USE_SKINNING 1");
  }

  // 变形目标
  if (parameters.morphing && node->getMesh() != -1
      && !primitive->getTargets().empty()) {
    auto weights = node->getWeights(gltf);
    if (!weights.empty()) {
      vertDefines.push_back("USE_MORPHING 1");
      vertDefines.push_back("WEIGHT_COUNT " + std::to_string(weights.size()));
    }
  }
}

void
GltfRenderer::pushFragParameterDefines(std::vector<std::string> &fragDefines,
                                       std::shared_ptr<GltfState> state) {
  if (!state) {
    return;
  }

  const auto &params = state->getRenderingParameters();

  // 点光源
  if (params.usePunctual) {
    fragDefines.push_back("USE_PUNCTUAL 1");
    fragDefines.push_back(
        "LIGHT_COUNT " + std::to_string(visibleLights.size()));
  }

  // IBL
  if (params.useIBL && state->getEnvironment()) {
    fragDefines.push_back("USE_IBL 1");
  }

  // 色调映射
  switch (params.toneMap) {
    case ToneMap::KHR_PBR_NEUTRAL:
      fragDefines.push_back("TONEMAP_KHR_PBR_NEUTRAL 1");
      break;
    case ToneMap::ACES_NARKOWICZ:
      fragDefines.push_back("TONEMAP_ACES_NARKOWICZ 1");
      break;
    case ToneMap::ACES_HILL:
      fragDefines.push_back("TONEMAP_ACES_HILL 1");
      break;
    case ToneMap::ACES_HILL_EXPOSURE_BOOST:
      fragDefines.push_back("TONEMAP_ACES_HILL_EXPOSURE_BOOST 1");
      break;
    case ToneMap::NONE:
    default:
      break;
  }
  addDebugOutputDefines(fragDefines, params.debugOutput);

}

void GltfRenderer::addDebugOutputDefines(std::vector<std::string> &fragDefines,
                                         DebugOutput debugOutput) {
  struct DebugMapping {
    DebugOutput output;
    std::string shaderDefine;
  };

  static const std::vector<DebugMapping> debugMappings = {
      {DebugOutput::NONE, "DEBUG_NONE"},
      {DebugOutput::WORLDSPACENORMAL, "DEBUG_NORMAL_SHADING"},
      {DebugOutput::NORMAL, "DEBUG_NORMAL_TEXTURE"},
      {DebugOutput::GEOMETRYNORMAL, "DEBUG_NORMAL_GEOMETRY"},
      {DebugOutput::TANGENT, "DEBUG_TANGENT"},
      {DebugOutput::BITANGENT, "DEBUG_BITANGENT"},
      {DebugOutput::ALPHA, "DEBUG_ALPHA"},
      {DebugOutput::UV_COORDS_0, "DEBUG_UV_0"},
      {DebugOutput::UV_COORDS_1, "DEBUG_UV_1"},
      {DebugOutput::OCCLUSION, "DEBUG_OCCLUSION"},
      {DebugOutput::EMISSIVE, "DEBUG_EMISSIVE"},
      {DebugOutput::BASECOLOR, "DEBUG_BASE_COLOR"},
      {DebugOutput::ROUGHNESS, "DEBUG_ROUGHNESS"},
      {DebugOutput::METALLIC, "DEBUG_METALLIC"},
      {DebugOutput::CLEARCOAT_FACTOR, "DEBUG_CLEARCOAT_FACTOR"},
      {DebugOutput::CLEARCOAT_ROUGHNESS, "DEBUG_CLEARCOAT_ROUGHNESS"},
      {DebugOutput::CLEARCOAT_NORMAL, "DEBUG_CLEARCOAT_NORMAL"},
      {DebugOutput::SHEEN_COLOR, "DEBUG_SHEEN_COLOR"},
      {DebugOutput::SHEEN_ROUGHNESS, "DEBUG_SHEEN_ROUGHNESS"},
      {DebugOutput::SPECULAR_FACTOR, "DEBUG_SPECULAR_FACTOR"},
      {DebugOutput::SPECULAR_COLOR, "DEBUG_SPECULAR_COLOR"},
      {DebugOutput::TRANSMISSION_FACTOR, "DEBUG_TRANSMISSION_FACTOR"},
      {DebugOutput::VOLUME_THICKNESS, "DEBUG_VOLUME_THICKNESS"},
      {DebugOutput::DIFFUSE_TRANSMISSION_FACTOR,
       "DEBUG_DIFFUSE_TRANSMISSION_FACTOR"},
      {DebugOutput::DIFFUSE_TRANSMISSION_COLOR_FACTOR,
       "DEBUG_DIFFUSE_TRANSMISSION_COLOR_FACTOR"},

      {DebugOutput::IRIDESCENCE_FACTOR, "DEBUG_IRIDESCENCE_FACTOR"},
      {DebugOutput::IRIDESCENCE_THICKNESS, "DEBUG_IRIDESCENCE_THICKNESS"},
      {DebugOutput::ANISOTROPIC_STRENGTH, "DEBUG_ANISOTROPIC_STRENGTH"},
      {DebugOutput::ANISOTROPIC_DIRECTION, "DEBUG_ANISOTROPIC_DIRECTION"},

  };

  // 添加所有调试定义及其编号
  int mappingCount = 0;
  bool mappingFound = false;

  for (const auto &mapping: debugMappings) {
    fragDefines.push_back(
        mapping.shaderDefine + " " + std::to_string(mappingCount++));

    if (debugOutput == mapping.output) {
      fragDefines.push_back("DEBUG " + mapping.shaderDefine);
      mappingFound = true;
    }
  }

  // 如果没有找到匹配的调试输出，使用默认值
  if (!mappingFound) {
    fragDefines.push_back("DEBUG DEBUG_NONE");
  }
}

void GltfRenderer::updateAnimationUniforms(std::shared_ptr<GltfState> state,
                                           std::shared_ptr<GltfNode> node,
                                           std::shared_ptr<GltfPrimitive> primitive) {
  if (!shader || !state || !node || !primitive) {
    return;
  }

  const auto &params = state->getRenderingParameters();
  auto gltf = state->getGltf();

  // 变形目标权重
  if (params.morphing && node->getMesh() != -1
      && !primitive->getTargets().empty()) {
    auto weights = node->getWeights(gltf);
    if (!weights.empty()) {
      // 直接使用 vector<double> 构造 UniformValue
      GLint morphWeightsLoc =
          glGetUniformLocation(shader->getProgram(), "u_morphWeights");
// 转换为 float 向量
      std::vector<float> floatWeights(weights.begin(), weights.end());

// 上传到 GPU
      glUniform1fv(morphWeightsLoc, floatWeights.size(), floatWeights.data());
      shader->updateUniform("u_morphWeights",
                            weights);  // 注意：使用 updateUniform 而不是 updateUniformArray

    }
  }
}

void GltfRenderer::applyLights() {
  if (!shader || visibleLights.empty()) {
    return;
  }

  // 转换光源为uniform格式
  std::vector<UniformLight> lightUniforms;
  lightUniforms.reserve(visibleLights.size());

  for (const auto &[node, light]: visibleLights) {
    if (light) {
      lightUniforms.push_back(light->toUniform(node));
    }
  }

  if (!lightUniforms.empty()) {
    auto lightPtr =
        std::make_shared<std::vector<UniformLight>>(std::move(lightUniforms));
//
//            glm::vec3 u_SkyColor = glm::vec3(0.85f, 0.8f, 0.75f);      // 温暖自然光
//            glm::vec3 u_GroundColor = glm::vec3(0.25f, 0.23f, 0.2f);   // 柔和反弹光
//
//            float intensity = 0.3f;
//            shader->updateUniform("u_GroundColor", u_GroundColor);
//            shader->updateUniform("u_AmbientIntensity", intensity);
//            shader->updateUniform("u_SkyColor", u_SkyColor);

    shader->updateUniform("u_Lights", lightPtr, true);
    logVerbose("Applied " + std::to_string(lightUniforms.size()) + " lights");
  }
}

int GltfRenderer::applyEnvironmentMap(std::shared_ptr<GltfState> state,
                                      int texSlotOffset) {
  if (!shader || !state) {
    return texSlotOffset;
  }

  auto environment = state->getEnvironment();
  if (!environment) {
    return texSlotOffset;
  }

  int currentSlot = texSlotOffset;
  auto gltf = state->getGltf();

  // 绑定环境贴图纹理
  openGlContext->setTexture(shader->getUniformLocation("u_LambertianEnvSampler"),
                            gltf, environment->diffuseEnvMap, currentSlot++);

  openGlContext->setTexture(shader->getUniformLocation("u_GGXEnvSampler"),
                            gltf, environment->specularEnvMap, currentSlot++);

  openGlContext->setTexture(shader->getUniformLocation("u_GGXLUT"),
                            gltf, environment->lut, currentSlot++);

  openGlContext->setTexture(shader->getUniformLocation("u_CharlieEnvSampler"),
                            gltf, environment->sheenEnvMap, currentSlot++);

// 在设置PBR材质的地方

  const int textureIndex = environment->lut->getIndex().value();
  if (textureIndex < 0
      || textureIndex >= static_cast<int>(gltf->getTextures().size())) {
    return false;
  }

  auto gltfTexture = gltf->getTextures()[textureIndex];
  if (!gltfTexture) {
    return false;
  }
// 激活纹理槽并绑定纹理


  openGlContext->setTexture(shader->getUniformLocation("u_CharlieLUT"),
                            gltf, environment->sheenLUT, currentSlot++);
  openGlContext->setTexture(shader->getUniformLocation("u_SheenELUT"),
                            gltf, environment->sheenELUT, currentSlot++);

  GLint tt = glGetUniformLocation(shader->getProgram(), "u_MipCount");
  glUniform1i(tt, environment->getMipCount());
  // 更新环境相关uniform
  shader->updateUniform("u_MipCount", environment->getMipCount());

  // 计算环境旋转矩阵
  glm::mat4 rotMatrix4 = glm::rotate(glm::mat4(1.0f),
                                     glm::radians(state->getRenderingParameters().environmentRotation),
                                     glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat3 rotMatrix3 = glm::mat3(rotMatrix4);
  shader->updateUniform("u_EnvRotation", rotMatrix3);

  // 计算环境强度
  float envIntensity = state->getRenderingParameters().iblIntensity
      * environment->getIBLIntensityScale();
  if (!state->getRenderingParameters().useIBL) {
    envIntensity = 0.0f;
  }
  shader->updateUniform("u_EnvIntensity", envIntensity);

  textureBinds += 6; // 绑定了6个纹理

  logVerbose("Applied environment map with intensity: "
                 + std::to_string(envIntensity));
  return currentSlot;
}

void GltfRenderer::blitFramebuffers() {
  if (opaqueFramebufferMSAA == 0 || opaqueFramebuffer == 0) {
    return;
  }

  // 从MSAA帧缓冲区blit到普通帧缓冲区
  glBindFramebuffer(GL_READ_FRAMEBUFFER, opaqueFramebufferMSAA);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, opaqueFramebuffer);

  glBlitFramebuffer(0, 0, opaqueFramebufferWidth, opaqueFramebufferHeight,
                    0, 0, opaqueFramebufferWidth, opaqueFramebufferHeight,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST);

  checkGLError("blit framebuffers");
}

bool GltfRenderer::validateFramebuffer(GLuint framebuffer) const {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    const char *errorStr = "Unknown error";
    switch (status) {
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        errorStr = "Incomplete attachment";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        errorStr = "Missing attachment";
        break;
      case GL_FRAMEBUFFER_UNSUPPORTED:
        errorStr = "Unsupported";
        break;
      default:
        break;
    }
    LOGE("Framebuffer validation failed: %s (0x%x)", errorStr, status);
    return false;
  }

  return true;
}

bool GltfRenderer::checkGLError(const std::string &operation) const {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    const char *errorStr = "Unknown error";
    switch (error) {
      case GL_INVALID_ENUM:
        errorStr = "Invalid enum";
        break;
      case GL_INVALID_VALUE:
        errorStr = "Invalid value";
        break;
      case GL_INVALID_OPERATION:
        errorStr = "Invalid operation";
        break;
      case GL_OUT_OF_MEMORY:
        errorStr = "Out of memory";
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        errorStr = "Invalid framebuffer operation";
        break;
      default:
        break;
    }
    LOGE("OpenGL error in %s: %s (0x%x)", operation.c_str(), errorStr, error);
    return false;
  }
  return true;
}

void GltfRenderer::logVerbose(const std::string &message) const {
  if (verboseLogging) {
    LOGV("%s", message.c_str());
  }
}

void GltfRenderer::destroy() {
  if (!initialized) {
    return;
  }

  try {
    // 销毁帧缓冲区
    destroyFramebuffers();

    // 销毁实例缓冲区
    if (instanceBuffer != 0) {
      glDeleteBuffers(1, &instanceBuffer);
      instanceBuffer = 0;
    }

    // 清理着色器缓存
    if (shaderCache) {
      shaderCache.reset();
    }

    // 清理环境渲染器
    if (environmentRenderer) {
      environmentRenderer.reset();
    }

    // 清理WebGL上下文
    if (openGlContext) {
      openGlContext.reset();
    }

    // 重置状态
    shader = nullptr;
    initialized = false;
    currentWidth = 0;
    currentHeight = 0;
    drawCallCount = 0;
    renderedPrimitives = 0;
    shaderSwitches = 0;
    textureBinds = 0;

    // 清理数据
    nodes.clear();
    opaqueDrawables.clear();
    transparentDrawables.clear();
    transmissionDrawables.clear();
    visibleLights.clear();

    LOGI("GltfRenderer destroyed");

  } catch (const std::exception &e) {
    LOGE("Exception in destroy: %s", e.what());
  }
}

void GltfRenderer::destroyFramebuffers() {
  if (opaqueRenderTexture != 0) {
    glDeleteTextures(1, &opaqueRenderTexture);
    opaqueRenderTexture = 0;
  }

  if (opaqueDepthTexture != 0) {
    glDeleteTextures(1, &opaqueDepthTexture);
    opaqueDepthTexture = 0;
  }

  if (colorRenderBuffer != 0) {
    glDeleteRenderbuffers(1, &colorRenderBuffer);
    colorRenderBuffer = 0;
  }

  if (depthRenderBuffer != 0) {
    glDeleteRenderbuffers(1, &depthRenderBuffer);
    depthRenderBuffer = 0;
  }

  if (opaqueFramebuffer != 0) {
    glDeleteFramebuffers(1, &opaqueFramebuffer);
    opaqueFramebuffer = 0;
  }

  if (opaqueFramebufferMSAA != 0) {
    glDeleteFramebuffers(1, &opaqueFramebufferMSAA);
    opaqueFramebufferMSAA = 0;
  }

  checkGLError("destroy framebuffers");
}

bool GltfRenderer::recreateFramebuffers(int width, int height) {
  if (width <= 0 || height <= 0) {
    LOGE("Invalid framebuffer dimensions: %dx%d", width, height);
    return false;
  }

  // 销毁现有帧缓冲区
  destroyFramebuffers();

  // 更新尺寸
  opaqueFramebufferWidth = width;
  opaqueFramebufferHeight = height;

  // 重新创建帧缓冲区
  return createFramebuffers();
}

void GltfRenderer::bindOpaqueFramebuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, opaqueFramebufferMSAA);
  glViewport(0, 0, opaqueFramebufferWidth, opaqueFramebufferHeight);
}

void GltfRenderer::bindDefaultFramebuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, currentWidth, currentHeight);
}

GltfRenderer::FramebufferInfo GltfRenderer::getFramebufferInfo() const {
  FramebufferInfo info;
  info.width = opaqueFramebufferWidth;
  info.height = opaqueFramebufferHeight;
  info.samples = samples;
  info.colorTexture = opaqueRenderTexture;
  info.depthTexture = opaqueDepthTexture;
  info.framebuffer = opaqueFramebuffer;
  info.msaaFramebuffer = opaqueFramebufferMSAA;
  return info;
}


void GltfRenderer::recompileShaders() {
  if (shaderCache) {
    shaderCache->clearCache();
    LOGI("Shader cache cleared - shaders will be recompiled on next use");
  }
}

void GltfRenderer::warmupShaderCache(std::shared_ptr<GltfState> state) {
  if (!state || !shaderCache) {
    return;
  }

  try {
    LOGI("Starting shader cache warmup...");

    // 预编译常用的着色器排列组合
    std::vector<std::string> commonVertDefines = {
        "NORMAL 1",
        "TEXCOORD_0 1",
        "USE_SKINNING 1",
        "USE_MORPHING 1",
        "USE_INSTANCING 1"
    };

    std::vector<std::string> commonFragDefines = {
        "BASECOLOR_MAP 1",
        "NORMAL_MAP 1",
        "METALLICROUGHNESS_MAP 1",
        "OCCLUSION_MAP 1",
        "EMISSIVE_MAP 1",
        "USE_IBL 1",
        "USE_PUNCTUAL 1",
        "LIGHT_COUNT 2"
    };

    // 生成常用组合
    std::vector<std::vector<std::string>> vertCombinations;
    std::vector<std::vector<std::string>> fragCombinations;

    // 基础组合
    vertCombinations.push_back({});
    fragCombinations.push_back({});

    // 添加单个定义的组合
    for (const auto &define: commonVertDefines) {
      vertCombinations.push_back({define});
    }

    for (const auto &define: commonFragDefines) {
      fragCombinations.push_back({define});
    }

    // 预编译着色器
    size_t compiledCount = 0;
    for (const auto &vertDefines: vertCombinations) {
      size_t
          vertHash = shaderCache->selectShader("primitive.vert", vertDefines);
      if (vertHash != 0) {
        for (const auto &fragDefines: fragCombinations) {
          size_t fragHash = shaderCache->selectShader("pbr.frag", fragDefines);
          if (fragHash != 0) {
            auto program = shaderCache->getShaderProgram(vertHash, fragHash);
            if (program) {
              compiledCount++;
            }
          }
        }
      }
    }

    LOGI("Shader cache warmup completed: %zu programs compiled", compiledCount);

  } catch (const std::exception &e) {
    LOGE("Exception during shader warmup: %s", e.what());
  }
}

std::shared_ptr<GltfMaterial>
GltfRenderer::getMaterial(int materialIndex, std::shared_ptr<GltfState> ptr) {
  // 这个方法需要访问当前的glTF对象
  // 实际实现中需要传递state参数或者保存glTF引用
  LOGW("getMaterial not fully implemented - needs glTF context");
  return ptr->getGltf()->getMaterials()[materialIndex];
}

// 着色器源代码方法（这些通常是静态的字符串常量）
std::string GltfRenderer::getPbrFragmentShaderSource() {
  return ShaderManager::getInstance().getShaderFiles().pbr;
}

std::string GltfRenderer::getPrimitiveVertexShaderSource() {
  return ShaderManager::getInstance().getShaderFiles().primitive;
}

std::string GltfRenderer::getCubemapVertexShaderSource() {
  return ShaderManager::getInstance().getShaderFiles().cubemap_vert;
}

std::string GltfRenderer::getCubemapFragmentShaderSource() {
  return ShaderManager::getInstance().getShaderFiles().cubemap;

}

std::string GltfRenderer::getSpecularGlossinessFragmentShaderSource() {
  return ShaderManager::getInstance().getShaderFiles().specular_glossiness;
}

// 包含文件着色器源代码
std::string GltfRenderer::getBrdfShaderSource() {
  return ShaderManager::getInstance().getShaderFiles().brdf;

}

std::string GltfRenderer::getIridescenceShaderSource() {
  return ShaderManager::getInstance().getShaderFiles().iridescence;
}

std::string GltfRenderer::getMaterialInfoShaderSource() {
  return ShaderManager::getInstance().getShaderFiles().material_info;
}

std::string GltfRenderer::getIblShaderSource() {
  return ShaderManager::getInstance().getShaderFiles().ibl;
}

std::string GltfRenderer::getPunctualShaderSource() {
  return ShaderManager::getInstance().getShaderFiles().punctual;
}

std::string GltfRenderer::getTexturesShaderSource() {
  return ShaderManager::getInstance().getShaderFiles().textures;
}

std::string GltfRenderer::getTonemappingShaderSource() {
  return ShaderManager::getInstance().getShaderFiles().tonemapping;
}

std::string GltfRenderer::getFunctionsShaderSource() {
  return ShaderManager::getInstance().getShaderFiles().functions;
}

std::string GltfRenderer::getAnimationShaderSource() {
  return ShaderManager::getInstance().getShaderFiles().animation;
}

} // namespace digitalhumans