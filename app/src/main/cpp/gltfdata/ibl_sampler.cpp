//
// Created by vincentsyan on 2025/9/15.
//

#include "ibl_sampler.h"
#include "ibl_sampler.h"
#include "ShaderCache.h"
#include "GltfOpenGLContext.h"
#include "GltfShader.h"
#include "converter/ShaderManager.h"
#include "../utils/LogUtils.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <memory>

namespace digitalhumans {

IBLSampler::IBLSampler(const std::shared_ptr<GltfOpenGLContext> &gl_)
    : gl_(gl_), textureSize_(256), ggxSampleCount_(1024),
      lambertianSampleCount_(2048),
      sheenSampleCount_(64), lodBias_(0.0f), lowestMipLevel_(4),
      lutResolution_(1024), mipmapLevels_(0),
      scaleValue_(1.0f), inputTextureID_(0), cubemapTextureID_(0),
      lambertianTextureID_(0), ggxTextureID_(0),
      sheenTextureID_(0), ggxLutTextureID_(0), charlieLutTextureID_(0),
      framebuffer_(0),
      preferredFormat_("HALF_FLOAT") {
  supportedFormats_.push_back("BYTE");

  // 初始化shader缓存
  ShaderCache::SourceMap sources;
  sources["fullscreen.vert"] =
      ShaderManager::getInstance().getShaderFiles().fullscreen;
  sources["panorama_to_cubemap.frag"] =
      ShaderManager::getInstance().getShaderFiles().panorama_to_cubemap;
  sources["ibl_filtering.frag"] =
      ShaderManager::getInstance().getShaderFiles().ibl_filtering;
  sources["debug.frag"] = ShaderManager::getInstance().getShaderFiles().debug;

  shaderCache_ = std::make_shared<ShaderCache>(sources, gl_);
  if (!shaderCache_) {
    throw std::runtime_error("Failed to create shader cache");
  }
}


IBLSampler::~IBLSampler() {
  destroy();
}

TextureData IBLSampler::prepareTextureData(const HDRImage &image) {
  TextureData texture;
  texture.internalFormat = GL_RGB32F;
  texture.format = GL_RGB;
  texture.type = GL_FLOAT;
  texture.data = nullptr;
  texture.dataSize = 0;

  // 重置缩放值
  scaleValue_ = 1.0f;

  const size_t numPixels = image.dataFloat.size() / 3;

  // 如果不支持高精度浮点格式
  if (std::find(supportedFormats_.begin(), supportedFormats_.end(), "FLOAT")
      == supportedFormats_.end() &&
      std::find(supportedFormats_.begin(),
                supportedFormats_.end(),
                "HALF_FLOAT") == supportedFormats_.end()) {

    texture.internalFormat = getInternalFormat();
    texture.format = GL_RGBA;
    texture.type = GL_UNSIGNED_BYTE;

    // 计算动态范围和缩放因子
    float maxValue = 0.0f;
    float clampedSum = 0.0f;
    float diffSum = 0.0f;

    for (size_t i = 0; i < numPixels; ++i) {
      size_t src = i * 3;
      float maxComponent = std::max({image.dataFloat[src],
                                     image.dataFloat[src + 1],
                                     image.dataFloat[src + 2]});

      if (maxComponent > 1.0f) {
        diffSum += maxComponent - 1.0f;
      }
      clampedSum += std::min(maxComponent, 1.0f);
      maxValue = std::max(maxComponent, maxValue);
    }

    float scaleFactor = 1.0f;
    if (clampedSum > 1.0f) {
      scaleFactor = (clampedSum + diffSum) / clampedSum;
    }


    // 转换为8位RGBA
    auto *byteData = new uint8_t[numPixels * 4];
    for (size_t i = 0; i < numPixels; ++i) {
      size_t src = i * 3;
      size_t dst = i * 4;

      byteData[dst + 0] =
          static_cast<uint8_t>(std::min(image.dataFloat[src + 0] * 255.0f,
                                        255.0f));
      byteData[dst + 1] =
          static_cast<uint8_t>(std::min(image.dataFloat[src + 1] * 255.0f,
                                        255.0f));
      byteData[dst + 2] =
          static_cast<uint8_t>(std::min(image.dataFloat[src + 2] * 255.0f,
                                        255.0f));
      byteData[dst + 3] = 255; // Alpha
    }

    texture.data = byteData;
    texture.dataSize = numPixels * 4;
    scaleValue_ = scaleFactor;
    return texture;
  }

  // 转换为RGBA32F格式
  auto *floatData = new float[numPixels * 4];
  float maxValue = 0.0f;

  for (size_t i = 0; i < numPixels; ++i) {
    size_t src = i * 3;
    size_t dst = i * 4;

    floatData[dst + 0] = image.dataFloat[src + 0];
    floatData[dst + 1] = image.dataFloat[src + 1];
    floatData[dst + 2] = image.dataFloat[src + 2];
    floatData[dst + 3] = 1.0f; // Alpha

    float maxComponent = std::max({image.dataFloat[src],
                                   image.dataFloat[src + 1],
                                   image.dataFloat[src + 2]});
    maxValue = std::max(maxComponent, maxValue);
  }

  // 检查是否需要FLOAT格式（超出HALF_FLOAT范围）
  if (maxValue > 65504.0f) {
    if (std::find(supportedFormats_.begin(), supportedFormats_.end(), "FLOAT")
        != supportedFormats_.end()) {
      // 移除HALF_FLOAT支持
      auto it = std::find(supportedFormats_.begin(),
                          supportedFormats_.end(),
                          "HALF_FLOAT");
      if (it != supportedFormats_.end()) {
        supportedFormats_.erase(it);
      }
    } else {
      std::cerr
          << "Warning: Supported texture formats do not support HDR value range"
          << std::endl;
      std::cerr
          << "Warning: Environment light intensity cannot be displayed correctly on this device"
          << std::endl;

      // 截断到HALF_FLOAT范围并计算补偿
      const float maxRange = 65504.0f;
      float clampedSum = 0.0f;
      float diffSum = 0.0f;

      for (size_t i = 0; i < numPixels; ++i) {
        size_t src = i * 3;
        size_t dst = i * 4;

        floatData[dst + 0] = std::min(image.dataFloat[src + 0], maxRange);
        floatData[dst + 1] = std::min(image.dataFloat[src + 1], maxRange);
        floatData[dst + 2] = std::min(image.dataFloat[src + 2], maxRange);

        float maxComponent = std::max({image.dataFloat[src],
                                       image.dataFloat[src + 1],
                                       image.dataFloat[src + 2]});

        if (maxComponent > maxRange) {
          diffSum += maxComponent - maxRange;
        }
        clampedSum += std::min(maxComponent, maxRange);
      }

      if (clampedSum > 1.0f) {
        scaleValue_ = (clampedSum + diffSum) / clampedSum;
      }
    }
  }

  // 设置最终的纹理格式
  if (std::find(supportedFormats_.begin(),
                supportedFormats_.end(),
                preferredFormat_) !=
      supportedFormats_.end()) {
    if (preferredFormat_ == "HALF_FLOAT") {
      texture.internalFormat = getInternalFormat();
      texture.format = GL_RGBA;
      texture.type = GL_FLOAT;
    }
  } else if (
      std::find(supportedFormats_.begin(), supportedFormats_.end(), "FLOAT")
          != supportedFormats_.end()) {
    texture.internalFormat = GL_RGBA32F;
    texture.format = GL_RGBA;
    texture.type = GL_FLOAT;
  }

  texture.data = floatData;
  texture.dataSize = numPixels * 4 * sizeof(float);
  return texture;
}

GLuint IBLSampler::loadTextureHDR(const HDRImage &image) {
  TextureData texture = prepareTextureData(image);

  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexImage2D(GL_TEXTURE_2D, 0, texture.internalFormat,
               image.width, image.height, 0,
               texture.format, texture.type, texture.data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // 清理临时数据
  delete[] static_cast<uint8_t *>(texture.data);

  return textureID;
}

bool isExtensionSupported(const std::string &extension) {
  GLint numExtensions;
  glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

  for (GLint i = 0; i < numExtensions; i++) {
    const char
        *ext = reinterpret_cast<const char *>(glGetStringi(GL_EXTENSIONS, i));
    if (ext && extension == std::string(ext)) {
      return true;
    }
  }
  return false;
}

void IBLSampler::checkSupportedFormats() {
  if (isExtensionSupported("GL_EXT_color_buffer_float") &&
      isExtensionSupported("GL_OES_texture_float_linear")) {
    supportedFormats_.push_back("FLOAT");
  }

  if (isExtensionSupported("GL_EXT_color_buffer_float") ||
      isExtensionSupported("GL_EXT_color_buffer_half_float")) {
    supportedFormats_.push_back("HALF_FLOAT");
  }
}

GLint IBLSampler::getInternalFormat() const {
  if (std::find(supportedFormats_.begin(),
                supportedFormats_.end(),
                preferredFormat_) !=
      supportedFormats_.end()) {
    if (preferredFormat_ == "FLOAT") return GL_RGBA32F;
    if (preferredFormat_ == "HALF_FLOAT") return GL_RGBA16F;
    if (preferredFormat_ == "BYTE") return GL_RGBA8;
  }

  if (std::find(supportedFormats_.begin(), supportedFormats_.end(), "FLOAT")
      != supportedFormats_.end()) {
    return GL_RGBA32F;
  }
  if (std::find(supportedFormats_.begin(),
                supportedFormats_.end(),
                "HALF_FLOAT") != supportedFormats_.end()) {
    return GL_RGBA16F;
  }

  return GL_RGBA8; // Fallback
}

GLenum IBLSampler::getTextureTargetType() const {
  if (std::find(supportedFormats_.begin(),
                supportedFormats_.end(),
                preferredFormat_) !=
      supportedFormats_.end()) {
    if (preferredFormat_ == "FLOAT") return GL_FLOAT;
    if (preferredFormat_ == "HALF_FLOAT") return GL_HALF_FLOAT;
    if (preferredFormat_ == "BYTE") return GL_UNSIGNED_BYTE;
  }

  if (std::find(supportedFormats_.begin(), supportedFormats_.end(), "FLOAT")
      != supportedFormats_.end()) {
    return GL_FLOAT;
  }
  if (std::find(supportedFormats_.begin(),
                supportedFormats_.end(),
                "HALF_FLOAT") != supportedFormats_.end()) {
    return GL_HALF_FLOAT;
  }

  return GL_UNSIGNED_BYTE; // Fallback
}

GLuint IBLSampler::createCubemapTexture(bool withMipmaps) {
  GLuint targetTexture;
  glGenTextures(1, &targetTexture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, targetTexture);

  auto internalFormat = getInternalFormat();
  auto textureTargetType = getTextureTargetType();


  // 为立方体贴图的6个面创建存储空间
  for (int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                 internalFormat, textureSize_, textureSize_, 0,
                 GL_RGBA, textureTargetType, nullptr);

    // 检查每个面是否创建成功
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
      LOGE("ERROR: glTexImage2D failed for face %d with error 0x%04X",
           i,
           error);

      // 尝试降级到最兼容的格式
      LOGE("Trying fallback format GL_RGBA + GL_UNSIGNED_BYTE");
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                   GL_RGBA, textureSize_, textureSize_, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

      error = glGetError();
      if (error != GL_NO_ERROR) {
        LOGE("ERROR: Fallback format also failed: 0x%04X", error);
        return 0;
      } else {
        LOGE("Fallback format succeeded for face %d", i);
      }
    } else {
      LOGE("Face %d created successfully", i);
    }
  }

  if (withMipmaps) {
    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
      LOGE("ERROR: glGenerateMipmap failed: 0x%04X", error);
    } else {
      LOGE("Mipmaps generated successfully");
    }
  } else {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    LOGE("Using linear filtering without mipmaps");
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return targetTexture;
}
GLuint IBLSampler::createLutTexture() {
  GLuint targetTexture;
  glGenTextures(1, &targetTexture);
  glBindTexture(GL_TEXTURE_2D, targetTexture);

  glTexImage2D(GL_TEXTURE_2D, 0, getInternalFormat(),
               lutResolution_, lutResolution_, 0,
               GL_RGBA, getTextureTargetType(), nullptr);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  return targetTexture;
}

bool IBLSampler::init(const HDRImage &panoramaImage) {
  // 检查支持的格式
  checkSupportedFormats();

  // 加载输入纹理
  inputTextureID_ = loadTextureHDR(panoramaImage);
  if (inputTextureID_ == 0) {
    return false;
  }

  // 创建立方体贴图
  cubemapTextureID_ = createCubemapTexture(true);

  // 创建帧缓冲区
  glGenFramebuffers(1, &framebuffer_);

  // 创建各种环境贴图
  lambertianTextureID_ = createCubemapTexture(false);
  ggxTextureID_ = createCubemapTexture(true);
  sheenTextureID_ = createCubemapTexture(true);

  // 生成mipmap
  glBindTexture(GL_TEXTURE_CUBE_MAP, ggxTextureID_);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  glBindTexture(GL_TEXTURE_CUBE_MAP, sheenTextureID_);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  // 计算mipmap级别
  mipmapLevels_ = static_cast<int>(std::floor(std::log2(textureSize_))) + 1
      - lowestMipLevel_;

  return true;
}

void IBLSampler::filterAll() {
  panoramaToCubeMap();
  cubeMapToLambertian();
  cubeMapToGGX();
  cubeMapToSheen();
  sampleGGXLut();
  sampleCharlieLut();

  // 恢复默认帧缓冲区
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void IBLSampler::panoramaToCubeMap() {
  for (int i = 0; i < 6; ++i) {
    // 设置渲染目标
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                           cubemapTextureID_, 0);

    glViewport(0, 0, textureSize_, textureSize_);
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 使用shader
    auto vertexShader = shaderCache_->selectShader("fullscreen.vert", {});
    auto fragmentShader =
        shaderCache_->selectShader("panorama_to_cubemap.frag", {});
    auto shader = shaderCache_->getShaderProgram(vertexShader, fragmentShader);
    glUseProgram(shader->getProgram());

    // 绑定输入纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTextureID_);

    GLint location = glGetUniformLocation(shader->getProgram(), "u_panorama");
    glUniform1i(location, 0); // 纹理单元0

    // 设置当前面
    GLint faceLocation =
        glGetUniformLocation(shader->getProgram(), "u_currentFace");
    glUniform1i(faceLocation, i);

    // 渲染全屏三角形
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }

  // 生成mipmap
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID_);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void
IBLSampler::applyFilter(int distribution, float roughness, int targetMipLevel,
                        GLuint targetTexture, int sampleCount, float lodBias) {
  int currentTextureSize = textureSize_ >> targetMipLevel;

  for (int i = 0; i < 6; ++i) {
    // 设置渲染目标
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                           targetTexture, targetMipLevel);

    glViewport(0, 0, currentTextureSize, currentTextureSize);
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 使用IBL过滤shader
    auto vertexShader = shaderCache_->selectShader("fullscreen.vert", {});
    auto fragmentShader = shaderCache_->selectShader("ibl_filtering.frag", {});

    auto shader = shaderCache_->getShaderProgram(vertexShader, fragmentShader);
    auto program = shader->getProgram();

    glUseProgram(program);

    // 绑定立方体贴图
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID_);

    GLint location = glGetUniformLocation(program, "u_cubemapTexture");
    glUniform1i(location, 0);

    // 设置uniform参数
    glUniform1f(glGetUniformLocation(program, "u_roughness"), roughness);
    glUniform1i(glGetUniformLocation(program, "u_sampleCount"), sampleCount);
    glUniform1f(glGetUniformLocation(program, "u_width"),
                static_cast<float>(textureSize_));
    glUniform1f(glGetUniformLocation(program, "u_lodBias"), lodBias);
    glUniform1i(glGetUniformLocation(program, "u_distribution"), distribution);
    glUniform1i(glGetUniformLocation(program, "u_currentFace"), i);
    glUniform1i(glGetUniformLocation(program, "u_isGeneratingLUT"), 0);

    // 设置纹理格式标志
    bool isFloatTexture =
        (std::find(supportedFormats_.begin(), supportedFormats_.end(), "BYTE")
            ==
                supportedFormats_.end());
    glUniform1i(glGetUniformLocation(program, "u_floatTexture"),
                isFloatTexture ? 1 : 0);

    glUniform1f(glGetUniformLocation(program, "u_intensityScale"), scaleValue_);

    // 渲染全屏三角形
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }
}

void IBLSampler::cubeMapToLambertian() {
  applyFilter(0, 0.0f, 0, lambertianTextureID_, lambertianSampleCount_);
}

void IBLSampler::cubeMapToGGX() {
  for (int currentMipLevel = 0; currentMipLevel <= mipmapLevels_;
       ++currentMipLevel) {
    float roughness = static_cast<float>(currentMipLevel) / (mipmapLevels_ - 1);
    applyFilter(1, roughness, currentMipLevel, ggxTextureID_, ggxSampleCount_);
  }
}

void IBLSampler::cubeMapToSheen() {
  for (int currentMipLevel = 0; currentMipLevel <= mipmapLevels_;
       ++currentMipLevel) {
    float roughness = static_cast<float>(currentMipLevel) / (mipmapLevels_ - 1);
    applyFilter(2,
                roughness,
                currentMipLevel,
                sheenTextureID_,
                sheenSampleCount_);
  }
}

void IBLSampler::sampleLut(int distribution,
                           GLuint targetTexture,
                           int currentTextureSize) {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, targetTexture, 0);

  glViewport(0, 0, currentTextureSize, currentTextureSize);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 清成黑色
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // 使用IBL过滤shader
  auto vertexShader = shaderCache_->selectShader("fullscreen.vert", {});
  auto fragmentShader = shaderCache_->selectShader("ibl_filtering.frag", {});
  auto shader = shaderCache_->getShaderProgram(vertexShader, fragmentShader);
  auto program = shader->getProgram();
  glUseProgram(program);

  // 绑定立方体贴图
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID_);

  GLint location = glGetUniformLocation(program, "u_cubemapTexture");
  glUniform1i(location, 0);

  // 设置LUT生成参数
  glUniform1f(glGetUniformLocation(program, "u_roughness"), 0.0f);
  glUniform1i(glGetUniformLocation(program, "u_sampleCount"), 512);
  glUniform1f(glGetUniformLocation(program, "u_width"), 0.0f);
  glUniform1f(glGetUniformLocation(program, "u_lodBias"), 0.0f);
  glUniform1i(glGetUniformLocation(program, "u_distribution"), distribution);
  glUniform1i(glGetUniformLocation(program, "u_currentFace"), 0);
  glUniform1i(glGetUniformLocation(program, "u_isGeneratingLUT"), 1);

  // 渲染全屏三角形
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void IBLSampler::sampleGGXLut() {
  ggxLutTextureID_ = createLutTexture();
  sampleLut(1, ggxLutTextureID_, lutResolution_);

  // 立即测试LUT内容
  glBindTexture(GL_TEXTURE_2D, ggxLutTextureID_);
  float testPixel[4];
  // 移动端调试：读取帧缓冲内容
  glReadPixels(lutResolution_ / 2,
               lutResolution_ / 2,
               1,
               1,
               GL_RGBA,
               GL_FLOAT,
               testPixel);

}

void IBLSampler::sampleCharlieLut() {
  charlieLutTextureID_ = createLutTexture();
  sampleLut(2, charlieLutTextureID_, lutResolution_);
}

void IBLSampler::destroy() {
//        if (shaderCache_) {
//            shaderCache_->destroy();
//            shaderCache_.reset();
//        }
//
//        // 删除所有纹理
//        if (inputTextureID_ != 0) {
//            glDeleteTextures(1, &inputTextureID_);
//            inputTextureID_ = 0;
//        }
//        if (cubemapTextureID_ != 0) {
//            glDeleteTextures(1, &cubemapTextureID_);
//            cubemapTextureID_ = 0;
//        }
//        if (lambertianTextureID_ != 0) {
//            glDeleteTextures(1, &lambertianTextureID_);
//            lambertianTextureID_ = 0;
//        }
//        if (ggxTextureID_ != 0) {
//            glDeleteTextures(1, &ggxTextureID_);
//            ggxTextureID_ = 0;
//        }
//        if (sheenTextureID_ != 0) {
//            glDeleteTextures(1, &sheenTextureID_);
//            sheenTextureID_ = 0;
//        }
//        if (ggxLutTextureID_ != 0) {
//            glDeleteTextures(1, &ggxLutTextureID_);
//            ggxLutTextureID_ = 0;
//        }
//        if (charlieLutTextureID_ != 0) {
//            glDeleteTextures(1, &charlieLutTextureID_);
//            charlieLutTextureID_ = 0;
//        }
//
//        // 删除帧缓冲区
//        if (framebuffer_ != 0) {
//            glDeleteFramebuffers(1, &framebuffer_);
//            framebuffer_ = 0;
//        }
}

int IBLSampler::getMipmapLevels() const {
  return mipmapLevels_;
}
}