//
// Created by vincentsyan on 2025/8/18.
//

#include "EnvironmentRenderer.h"

#include <android/log.h>
#include <algorithm>

#include "../utils/LogUtils.h"
#include "GltfState.h"
#include "ShaderCache.h"
#include "GltfShader.h"

#include "GltfOpenGLContext.h"


namespace digitalhumans {

EnvironmentRenderer::EnvironmentRenderer(std::shared_ptr<GltfOpenGLContext> context)
    : indexBuffer(0), vertexBuffer(0), initialized(false), stateBackup{} {
  if (context) {
    initialized = initializeBuffers(context);
    if (!initialized) {
      LOGE("Failed to initialize EnvironmentRenderer");
    }
  } else {
    LOGE("Invalid  context provided to EnvironmentRenderer");
  }
}

EnvironmentRenderer::~EnvironmentRenderer() {
  cleanup();
}

EnvironmentRenderer::EnvironmentRenderer(EnvironmentRenderer &&other) noexcept
    : indexBuffer(other.indexBuffer), vertexBuffer(other.vertexBuffer),
      initialized(other.initialized), stateBackup(other.stateBackup) {
  other.indexBuffer = 0;
  other.vertexBuffer = 0;
  other.initialized = false;
}

EnvironmentRenderer &
EnvironmentRenderer::operator=(EnvironmentRenderer &&other) noexcept {
  if (this != &other) {
    cleanup();

    indexBuffer = other.indexBuffer;
    vertexBuffer = other.vertexBuffer;
    initialized = other.initialized;
    stateBackup = other.stateBackup;

    other.indexBuffer = 0;
    other.vertexBuffer = 0;
    other.initialized = false;

  }
  return *this;
}

void
EnvironmentRenderer::drawEnvironmentMap(std::shared_ptr<GltfOpenGLContext> context,
                                        const glm::mat4 &viewProjectionMatrix,
                                        std::shared_ptr<GltfState> state,
                                        std::shared_ptr<ShaderCache> shaderCache,
                                        const std::vector<std::string> &fragDefines) {
  if (!initialized || !context || !state || !shaderCache) {
    LOGW("EnvironmentRenderer not properly initialized or invalid parameters");
    return;
  }
  auto environment = state->getEnvironment();
  if (!environment || !state->getRenderingParameters().renderEnvironmentMap) {
    return;
  }

  try {
    auto vertShader = shaderCache->selectShader("cubemap.vert", {});
    auto fragShader = shaderCache->selectShader("cubemap.frag", fragDefines);
    auto shaderProgram = shaderCache->getShaderProgram(vertShader, fragShader);

    if (!shaderProgram || !shaderProgram->isValid()) {
      LOGE("Failed to get valid shader program for environment rendering");
      return;
    }
    shaderProgram->use();
    auto specularEnvMap = environment->getSpecularEnvMap();
    if (specularEnvMap) {
      context->setTexture(shaderProgram->getUniformLocation("u_GGXEnvSampler"),
                          state->getGltf(), environment->specularEnvMap, 0);
    }
    shaderProgram->setUniform("u_MipCount",
                              static_cast<float>(environment->getMipCount()));
    float blurValue =
        state->getRenderingParameters().blurEnvironmentMap ? 0.6f : 0.0f;
    shaderProgram->setUniform("u_EnvBlurNormalized", blurValue);
    float envIntensity = state->getRenderingParameters().iblIntensity *
        environment->getIBLIntensityScale();
    shaderProgram->setUniform("u_EnvIntensity", envIntensity);
    shaderProgram->setUniform("u_ViewProjectionMatrix", viewProjectionMatrix);
    shaderProgram->setUniform("u_Exposure",
                              state->getRenderingParameters().exposure);
    glm::mat3 rotMatrix = calculateEnvironmentRotationMatrix(
        state->getRenderingParameters().environmentRotation);
    shaderProgram->setUniform("u_EnvRotation", rotMatrix);
    setupRenderState();
    GLint positionAttributeLocation =
        shaderProgram->getAttributeLocation("a_position");
    if (positionAttributeLocation >= 0) {
      bindGeometry(positionAttributeLocation);
      glDrawElements(GL_TRIANGLES, getIndexCount(), GL_UNSIGNED_SHORT, 0);
    } else {
      LOGW("Could not find position attribute in environment shader");
    }
    restoreRenderState();
  } catch (const std::exception &e) {
    LOGE("Exception in drawEnvironmentMap: %s", e.what());
    restoreRenderState();
  }
}

void EnvironmentRenderer::cleanup() {
  if (initialized) {
    if (indexBuffer != 0) {
      glDeleteBuffers(1, &indexBuffer);
      indexBuffer = 0;
    }

    if (vertexBuffer != 0) {
      glDeleteBuffers(1, &vertexBuffer);
      vertexBuffer = 0;
    }

    initialized = false;
    checkGLError("cleanup");
  }
}

bool
EnvironmentRenderer::initializeBuffers(std::shared_ptr<GltfOpenGLContext> context) {
  if (!context) {
    LOGE("Invalid WebGL context");
    return false;
  }

  try {
    glGenBuffers(1, &indexBuffer);
    if (indexBuffer == 0) {
      LOGE("Failed to generate index buffer");
      return false;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    auto indices = createCubeIndices();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(uint16_t),
                 indices.data(),
                 GL_STATIC_DRAW);

    if (!checkGLError("create index buffer")) {
      glDeleteBuffers(1, &indexBuffer);
      indexBuffer = 0;
      return false;
    }
    glGenBuffers(1, &vertexBuffer);
    if (vertexBuffer == 0) {
      LOGE("Failed to generate vertex buffer");
      glDeleteBuffers(1, &indexBuffer);
      indexBuffer = 0;
      return false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    auto vertices = createCubeVertices();
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(float),
                 vertices.data(),
                 GL_STATIC_DRAW);

    if (!checkGLError("create vertex buffer")) {
      glDeleteBuffers(1, &indexBuffer);
      glDeleteBuffers(1, &vertexBuffer);
      indexBuffer = 0;
      vertexBuffer = 0;
      return false;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;

  } catch (const std::exception &e) {
    LOGE("Exception during buffer initialization: %s", e.what());
    cleanup();
    return false;
  }
}

std::vector<uint16_t> EnvironmentRenderer::createCubeIndices() const {
  return {
      1, 2, 0,    // 面1
      2, 3, 0,
      6, 2, 1,    // 面2
      1, 5, 6,
      6, 5, 4,    // 面3
      4, 7, 6,
      6, 3, 2,    // 面4
      7, 3, 6,
      3, 7, 0,    // 面5
      7, 4, 0,
      5, 1, 0,    // 面6
      4, 5, 0
  };
}

std::vector<float> EnvironmentRenderer::createCubeVertices() const {
  return {
      -1.0f, -1.0f, -1.0f,    // 顶点0
      1.0f, -1.0f, -1.0f,    // 顶点1
      1.0f, 1.0f, -1.0f,    // 顶点2
      -1.0f, 1.0f, -1.0f,    // 顶点3
      -1.0f, -1.0f, 1.0f,    // 顶点4
      1.0f, -1.0f, 1.0f,    // 顶点5
      1.0f, 1.0f, 1.0f,    // 顶点6
      -1.0f, 1.0f, 1.0f     // 顶点7
  };
}

void EnvironmentRenderer::setupRenderState() {
  glGetIntegerv(GL_FRONT_FACE,
                reinterpret_cast<GLint *>(&stateBackup.frontFace));
  stateBackup.cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
  stateBackup.blendEnabled = glIsEnabled(GL_BLEND);
  stateBackup.depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  checkGLError("setup render state");
}

void EnvironmentRenderer::restoreRenderState() {
  // 恢复之前的渲染状态
  glFrontFace(stateBackup.frontFace);

  if (stateBackup.cullFaceEnabled) {
    glEnable(GL_CULL_FACE);
  } else {
    glDisable(GL_CULL_FACE);
  }

  if (stateBackup.blendEnabled) {
    glEnable(GL_BLEND);
  } else {
    glDisable(GL_BLEND);
  }

  if (stateBackup.depthTestEnabled) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }

  checkGLError("restore render state");
}

void EnvironmentRenderer::bindGeometry(GLint positionAttributeLocation) {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glVertexAttribPointer(positionAttributeLocation,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        0,
                        nullptr);

  glEnableVertexAttribArray(positionAttributeLocation);

  checkGLError("bind geometry");
}

glm::mat3
EnvironmentRenderer::calculateEnvironmentRotationMatrix(float rotationDegrees) const {
  glm::mat4 rotMatrix4 = glm::mat4(1.0f);
  rotMatrix4 = glm::rotate(rotMatrix4,
                           glm::radians(rotationDegrees),
                           glm::vec3(0.0f, 1.0f, 0.0f)); // 绕Y轴旋转

  return glm::mat3(rotMatrix4);
}


bool EnvironmentRenderer::checkGLError(const std::string &operation) const {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    const char *errorString = nullptr;

    switch (error) {
      case GL_INVALID_ENUM:
        errorString = "GL_INVALID_ENUM";
        break;
      case GL_INVALID_VALUE:
        errorString = "GL_INVALID_VALUE";
        break;
      case GL_INVALID_OPERATION:
        errorString = "GL_INVALID_OPERATION";
        break;
      case GL_OUT_OF_MEMORY:
        errorString = "GL_OUT_OF_MEMORY";
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;
      default:
        errorString = "UNKNOWN_ERROR";
        break;
    }

    LOGE("OpenGL error during %s: %s (0x%x)",
         operation.c_str(),
         errorString,
         error);
    return false;
  }
  return true;
}

} // namespace digitalhumans