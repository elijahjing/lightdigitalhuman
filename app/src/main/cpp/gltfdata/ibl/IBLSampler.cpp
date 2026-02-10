//
// Created by vincentsyan on 2025/9/16.
//

#include "IBLSampler.h"

namespace digitalhunman {
// IBLSampler.cpp
#include "IBLSampler.h"
#include "../ShaderCache.h"
#include <cmath>
#include <algorithm>
#include <android/log.h>

#define LOG_TAG "IBLSampler"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static const char *fullscreenVertexShader = R"(
#version 300 es
precision highp float;

out vec2 v_texCoord;

void main() {
    // 全屏三角形
    vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );

    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
    v_texCoord = positions[gl_VertexID] * 0.5 + 0.5;
}
)";

static const char *panoramaToCubemapFragmentShader = R"(
#version 300 es
precision highp float;

uniform sampler2D u_panorama;
uniform int u_currentFace;

in vec2 v_texCoord;
out vec4 FragColor;

vec3 uvToDirection(vec2 uv, int face) {
    uv = uv * 2.0 - 1.0; // [0,1] -> [-1,1]

    if (face == 0) return normalize(vec3(1.0, -uv.y, -uv.x));  // +X
    if (face == 1) return normalize(vec3(-1.0, -uv.y, uv.x));  // -X
    if (face == 2) return normalize(vec3(uv.x, 1.0, uv.y));    // +Y
    if (face == 3) return normalize(vec3(uv.x, -1.0, -uv.y));  // -Y
    if (face == 4) return normalize(vec3(uv.x, -uv.y, 1.0));   // +Z
    if (face == 5) return normalize(vec3(-uv.x, -uv.y, -1.0)); // -Z

    return vec3(0.0);
}

vec2 directionToUV(vec3 dir) {
    float phi = atan(dir.z, dir.x);
    float theta = acos(dir.y);
    return vec2(phi / (2.0 * 3.14159265) + 0.5, theta / 3.14159265);
}

void main() {
    vec3 direction = uvToDirection(v_texCoord, u_currentFace);
    vec2 panoramaUV = directionToUV(direction);
    FragColor = texture(u_panorama, panoramaUV);
}
)";

static const char *iblFilteringFragmentShader = R"(
#version 300 es
precision highp float;

uniform samplerCube u_cubemapTexture;
uniform float u_roughness;
uniform int u_sampleCount;
uniform float u_width;
uniform float u_lodBias;
uniform int u_distribution;
uniform int u_currentFace;
uniform int u_isGeneratingLUT;
uniform int u_floatTexture;
uniform float u_intensityScale;

in vec2 v_texCoord;
out vec4 FragColor;

const float PI = 3.14159265359;

// 随机数生成
float radicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley2d(int i, int N) {
    return vec2(float(i) / float(N), radicalInverse_VdC(uint(i)));
}

// 重要性采样
vec3 importanceSampleGGX(vec2 Xi, float roughness, vec3 N) {
    float a = roughness * roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

    vec3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;

    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    return tangent * H.x + bitangent * H.y + N * H.z;
}

vec3 uvToDirection(vec2 uv, int face) {
    uv = uv * 2.0 - 1.0;

    if (face == 0) return normalize(vec3(1.0, -uv.y, -uv.x));
    if (face == 1) return normalize(vec3(-1.0, -uv.y, uv.x));
    if (face == 2) return normalize(vec3(uv.x, 1.0, uv.y));
    if (face == 3) return normalize(vec3(uv.x, -1.0, -uv.y));
    if (face == 4) return normalize(vec3(uv.x, -uv.y, 1.0));
    if (face == 5) return normalize(vec3(-uv.x, -uv.y, -1.0));

    return vec3(0.0);
}

void main() {
    if (u_isGeneratingLUT == 1) {
        // BRDF LUT生成逻辑
        float NdotV = v_texCoord.x;
        float roughness = v_texCoord.y;

        vec3 V = vec3(sqrt(1.0 - NdotV*NdotV), 0.0, NdotV);
        vec3 N = vec3(0.0, 0.0, 1.0);

        float A = 0.0;
        float B = 0.0;

        for(int i = 0; i < u_sampleCount; i++) {
            vec2 Xi = hammersley2d(i, u_sampleCount);
            vec3 H = importanceSampleGGX(Xi, roughness, N);
            vec3 L = normalize(2.0 * dot(V, H) * H - V);

            float NdotL = max(L.z, 0.0);
            float NdotH = max(H.z, 0.0);
            float VdotH = max(dot(V, H), 0.0);

            if(NdotL > 0.0) {
                float G = min(1.0, min(2.0 * NdotH * NdotV / VdotH, 2.0 * NdotH * NdotL / VdotH));
                float G_Vis = G * VdotH / (NdotH * NdotV);
                float Fc = pow(1.0 - VdotH, 5.0);
                A += (1.0 - Fc) * G_Vis;
                B += Fc * G_Vis;
            }
        }

        FragColor = vec4(A / float(u_sampleCount), B / float(u_sampleCount), 0.0, 1.0);
        return;
    }

    // 环境贴图滤波
    vec3 N = uvToDirection(v_texCoord, u_currentFace);
    vec3 V = N;

    vec3 color = vec3(0.0);
    float totalWeight = 0.0;

    for(int i = 0; i < u_sampleCount; i++) {
        vec2 Xi = hammersley2d(i, u_sampleCount);

        vec3 H, L;
        if (u_distribution == 0) { // Lambertian
            // 余弦加权半球采样
            float phi = 2.0 * PI * Xi.x;
            float cosTheta = sqrt(Xi.y);
            float sinTheta = sqrt(1.0 - Xi.y);

            H = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

            vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
            vec3 tangent = normalize(cross(up, N));
            vec3 bitangent = cross(N, tangent);
            L = tangent * H.x + bitangent * H.y + N * H.z;
        } else { // GGX or Sheen
            H = importanceSampleGGX(Xi, u_roughness, N);
            L = normalize(2.0 * dot(V, H) * H - V);
        }

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0) {
            float mipLevel = 0.0;
            if (u_distribution != 0) {
                // 计算合适的mip级别
                float D = (u_roughness * u_roughness) /
                         (PI * pow(pow(dot(N, H), 2.0) * (u_roughness * u_roughness - 1.0) + 1.0, 2.0));
                float pdf = D * dot(N, H) / (4.0 * dot(V, H));
                float solidAngle = 1.0 / (float(u_sampleCount) * pdf);
                mipLevel = 0.5 * log2(solidAngle * u_width * u_width / (4.0 * PI)) + u_lodBias;
            }

            color += textureLod(u_cubemapTexture, L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    if (totalWeight > 0.0) {
        color /= totalWeight;
    }

    color *= u_intensityScale;
    FragColor = vec4(color, 1.0);
}
)";

IBLSampler::IBLSampler(int contextWidth, int contextHeight) {

}

IBLSampler::~IBLSampler() {
  destroy();
}

bool IBLSampler::initialize() {
  if (!checkExtensionSupport()) {
    LOGE("Required extensions not supported");
    return false;
  }

  // 2. 创建着色器缓存
  shaderCache_ = std::make_unique<ShaderCache>();
  if (!createShaders()) {
    LOGE(" Failed to create shaders");
    return false;
  }
  glGenFramebuffers(1, &framebuffer_);
  if (framebuffer_ == 0) {
    LOGE("Failed to create framebuffer");
    return false;
  }
  mipmapLevels_ =
      static_cast<int>(std::floor(std::log2(config_.textureSize))) + 1
          - config_.lowestMipLevel;
  return true;
}

bool IBLSampler::checkExtensionSupport() {

  supportedFormats_.push_back(TextureFormat::BYTE);
  if (glGetString(GL_EXTENSIONS)) {
    std::string
        extensions(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));

    if (extensions.find("GL_EXT_color_buffer_float") != std::string::npos &&
        extensions.find("GL_OES_texture_float_linear") != std::string::npos) {
      supportedFormats_.push_back(TextureFormat::FLOAT);
    }

    if (extensions.find("GL_EXT_color_buffer_half_float") != std::string::npos
        ||
            extensions.find("GL_EXT_color_buffer_float") != std::string::npos) {
      supportedFormats_.push_back(TextureFormat::HALF_FLOAT);
    }
  }

  return true;
}

bool IBLSampler::createShaders() {
  if (!shaderCache_->addShader("fullscreen.vert", fullscreenVertexShader)) {
    return false;
  }

  if (!shaderCache_->addShader("panorama_to_cubemap.frag",
                               panoramaToCubemapFragmentShader)) {
    return false;
  }

  if (!shaderCache_->addShader("ibl_filtering.frag",
                               iblFilteringFragmentShader)) {
    return false;
  }
  return true;
}

struct IBLSampler::TextureData
IBLSampler::prepareTextureData(const HDRImage &image) {
  TextureData textureData;
  textureData.width = image.getWidth();
  textureData.height = image.getHeight();

  const float *sourceData = image.getData();
  const int numPixels = textureData.width * textureData.height;
  scaleValue_ = 1.0f;

  bool useFloat = std::find(supportedFormats_.begin(), supportedFormats_.end(),
                            TextureFormat::FLOAT) != supportedFormats_.end();
  bool useHalfFloat =
      std::find(supportedFormats_.begin(), supportedFormats_.end(),
                TextureFormat::HALF_FLOAT) != supportedFormats_.end();

  if (preferredFormat_ == TextureFormat::FLOAT && useFloat) {
    textureData.internalFormat = GL_RGBA32F;
    textureData.format = GL_RGBA;
    textureData.type = GL_FLOAT;

    textureData.floatData.resize(numPixels * 4);
    for (int i = 0; i < numPixels; i++) {
      textureData.floatData[i * 4 + 0] = sourceData[i * 3 + 0];
      textureData.floatData[i * 4 + 1] = sourceData[i * 3 + 1];
      textureData.floatData[i * 4 + 2] = sourceData[i * 3 + 2];
      textureData.floatData[i * 4 + 3] = 1.0f;
    }
  } else if (useHalfFloat) {
    textureData.internalFormat = GL_RGBA16F;
    textureData.format = GL_RGBA;
    textureData.type = GL_HALF_FLOAT;

    textureData.floatData.resize(numPixels * 4);
    for (int i = 0; i < numPixels; i++) {
      textureData.floatData[i * 4 + 0] = sourceData[i * 3 + 0];
      textureData.floatData[i * 4 + 1] = sourceData[i * 3 + 1];
      textureData.floatData[i * 4 + 2] = sourceData[i * 3 + 2];
      textureData.floatData[i * 4 + 3] = 1.0f;
    }
  } else {
    textureData.internalFormat = GL_RGBA8;
    textureData.format = GL_RGBA;
    textureData.type = GL_UNSIGNED_BYTE;

    float maxValue = 0.0f;
    float clampedSum = 0.0f;
    float diffSum = 0.0f;

    for (int i = 0; i < numPixels; i++) {
      float maxComponent = std::max({sourceData[i * 3 + 0],
                                     sourceData[i * 3 + 1],
                                     sourceData[i * 3 + 2]});
      if (maxComponent > 1.0f) {
        diffSum += maxComponent - 1.0f;
      }
      clampedSum += std::min(maxComponent, 1.0f);
      maxValue = std::max(maxComponent, maxValue);
    }

    if (clampedSum > 1.0f) {
      scaleValue_ = (clampedSum + diffSum) / clampedSum;
    }
    textureData.byteData.resize(numPixels * 4);
    for (int i = 0; i < numPixels; i++) {
      textureData.byteData[i * 4 + 0] =
          static_cast<uint8_t>(std::min(sourceData[i * 3 + 0] * 255.0f,
                                        255.0f));
      textureData.byteData[i * 4 + 1] =
          static_cast<uint8_t>(std::min(sourceData[i * 3 + 1] * 255.0f,
                                        255.0f));
      textureData.byteData[i * 4 + 2] =
          static_cast<uint8_t>(std::min(sourceData[i * 3 + 2] * 255.0f,
                                        255.0f));
      textureData.byteData[i * 4 + 3] = 255;
    }
  }

  return textureData;
}

GLuint IBLSampler::loadTextureHDR(const HDRImage &image) {
  TextureData textureData = prepareTextureData(image);
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  const void *data = textureData.floatData.empty() ?
                     static_cast<const void *>(textureData.byteData.data()) :
                     static_cast<const void *>(textureData.floatData.data());

  glTexImage2D(GL_TEXTURE_2D, 0, textureData.internalFormat,
               textureData.width, textureData.height, 0,
               textureData.format, textureData.type, data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    glDeleteTextures(1, &textureID);
    return 0;
  }
  return textureID;
}

GLuint IBLSampler::createCubemapTexture(bool withMipmaps) {
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  GLenum internalFormat = getInternalFormat();
  GLenum type = getTextureType();

  for (int i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat,
                 config_.textureSize, config_.textureSize, 0,
                 GL_RGBA, type, nullptr);
  }

  if (withMipmaps) {
    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  return textureID;
}

GLuint IBLSampler::createLutTexture() {
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexImage2D(GL_TEXTURE_2D, 0, getInternalFormat(),
               config_.lutResolution, config_.lutResolution, 0,
               GL_RGBA, getTextureType(), nullptr);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  return textureID;
}

bool IBLSampler::processHDRImage(const HDRImage &panoramaImage) {
  inputTextureID_ = loadTextureHDR(panoramaImage);
  if (inputTextureID_ == 0) {
    LOGE("Failed to load HDR texture");
    return false;
  }
  cubemapTextureID_ = createCubemapTexture(true);
  lambertianTextureID_ = createCubemapTexture(false);
  ggxTextureID_ = createCubemapTexture(true);
  sheenTextureID_ = createCubemapTexture(true);

  panoramaToCubeMap();
  cubeMapToLambertian();
  cubeMapToGGX();
  cubeMapToSheen();
  sampleGGXLut();
  sampleCharlieLut();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return true;
}

void IBLSampler::panoramaToCubeMap() {
  GLuint program =
      shaderCache_->getProgram("fullscreen.vert", "panorama_to_cubemap.frag");
  glUseProgram(program);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, inputTextureID_);
  glUniform1i(glGetUniformLocation(program, "u_panorama"), 0);

  for (int face = 0; face < 6; face++) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                           cubemapTextureID_, 0);

    glViewport(0, 0, config_.textureSize, config_.textureSize);
    glClear(GL_COLOR_BUFFER_BIT);
    glUniform1i(glGetUniformLocation(program, "u_currentFace"), face);
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID_);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void
IBLSampler::applyFilter(int distribution, float roughness, int targetMipLevel,
                        GLuint targetTexture, int sampleCount, float lodBias) {
  GLuint program =
      shaderCache_->getProgram("fullscreen.vert", "ibl_filtering.frag");
  glUseProgram(program);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID_);
  glUniform1i(glGetUniformLocation(program, "u_cubemapTexture"), 0);

  glUniform1f(glGetUniformLocation(program, "u_roughness"), roughness);
  glUniform1i(glGetUniformLocation(program, "u_sampleCount"), sampleCount);
  glUniform1f(glGetUniformLocation(program, "u_width"),
              static_cast<float>(config_.textureSize));
  glUniform1f(glGetUniformLocation(program, "u_lodBias"), lodBias);
  glUniform1i(glGetUniformLocation(program, "u_distribution"), distribution);
  glUniform1i(glGetUniformLocation(program, "u_isGeneratingLUT"), 0);
  glUniform1i(glGetUniformLocation(program, "u_floatTexture"),
              (preferredFormat_ != TextureFormat::BYTE) ? 1 : 0);
  glUniform1f(glGetUniformLocation(program, "u_intensityScale"), scaleValue_);

  int currentTextureSize = config_.textureSize >> targetMipLevel;

  for (int face = 0; face < 6; face++) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                           targetTexture, targetMipLevel);

    glViewport(0, 0, currentTextureSize, currentTextureSize);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1i(glGetUniformLocation(program, "u_currentFace"), face);

    glDrawArrays(GL_TRIANGLES, 0, 3);
  }
}

void IBLSampler::cubeMapToLambertian() {
  applyFilter(0, 0.0f, 0, lambertianTextureID_, config_.lambertianSampleCount);
}

void IBLSampler::cubeMapToGGX() {
  glBindTexture(GL_TEXTURE_CUBE_MAP, ggxTextureID_);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  for (int mipLevel = 0; mipLevel <= mipmapLevels_; mipLevel++) {
    float roughness =
        static_cast<float>(mipLevel) / static_cast<float>(mipmapLevels_ - 1);
    applyFilter(1, roughness, mipLevel, ggxTextureID_, config_.ggxSampleCount);
  }
}

void IBLSampler::cubeMapToSheen() {
  glBindTexture(GL_TEXTURE_CUBE_MAP, sheenTextureID_);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  for (int mipLevel = 0; mipLevel <= mipmapLevels_; mipLevel++) {
    float roughness =
        static_cast<float>(mipLevel) / static_cast<float>(mipmapLevels_ - 1);
    applyFilter(2,
                roughness,
                mipLevel,
                sheenTextureID_,
                config_.sheenSampleCount);
  }
}

void
IBLSampler::sampleLut(int distribution, GLuint targetTexture, int textureSize) {
  GLuint program =
      shaderCache_->getProgram("fullscreen.vert", "ibl_filtering.frag");
  glUseProgram(program);

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, targetTexture, 0);

  glViewport(0, 0, textureSize, textureSize);
  glClear(GL_COLOR_BUFFER_BIT);

  // 设置uniforms
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID_);
  glUniform1i(glGetUniformLocation(program, "u_cubemapTexture"), 0);

  glUniform1f(glGetUniformLocation(program, "u_roughness"), 0.0f);
  glUniform1i(glGetUniformLocation(program, "u_sampleCount"), 512);
  glUniform1f(glGetUniformLocation(program, "u_width"), 0.0f);
  glUniform1f(glGetUniformLocation(program, "u_lodBias"), 0.0f);
  glUniform1i(glGetUniformLocation(program, "u_distribution"), distribution);
  glUniform1i(glGetUniformLocation(program, "u_currentFace"), 0);
  glUniform1i(glGetUniformLocation(program, "u_isGeneratingLUT"), 1);

  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void IBLSampler::sampleGGXLut() {
  ggxLutTextureID_ = createLutTexture();
  sampleLut(1, ggxLutTextureID_, config_.lutResolution);
}

void IBLSampler::sampleCharlieLut() {
  charlieLutTextureID_ = createLutTexture();
  sampleLut(2, charlieLutTextureID_, config_.lutResolution);
}

GLenum IBLSampler::getInternalFormat() const {
  // 根据支持的格式返回合适的内部格式
  if (std::find(supportedFormats_.begin(), supportedFormats_.end(),
                preferredFormat_) != supportedFormats_.end()) {
    switch (preferredFormat_) {
      case TextureFormat::FLOAT:
        return GL_RGBA32F;
      case TextureFormat::HALF_FLOAT:
        return GL_RGBA16F;
      case TextureFormat::BYTE:
        return GL_RGBA8;
    }
  }

  // 降级选择
  if (std::find(supportedFormats_.begin(), supportedFormats_.end(),
                TextureFormat::FLOAT) != supportedFormats_.end()) {
    return GL_RGBA32F;
  }
  if (std::find(supportedFormats_.begin(), supportedFormats_.end(),
                TextureFormat::HALF_FLOAT) != supportedFormats_.end()) {
    return GL_RGBA16F;
  }

  return GL_RGBA8; // 最后的降级选择
}

GLenum IBLSampler::getTextureType() const {
  // 与getInternalFormat()对应
  if (std::find(supportedFormats_.begin(), supportedFormats_.end(),
                preferredFormat_) != supportedFormats_.end()) {
    switch (preferredFormat_) {
      case TextureFormat::FLOAT:
        return GL_FLOAT;
      case TextureFormat::HALF_FLOAT:
        return GL_HALF_FLOAT;
      case TextureFormat::BYTE:
        return GL_UNSIGNED_BYTE;
    }
  }

  if (std::find(supportedFormats_.begin(), supportedFormats_.end(),
                TextureFormat::FLOAT) != supportedFormats_.end()) {
    return GL_FLOAT;
  }
  if (std::find(supportedFormats_.begin(), supportedFormats_.end(),
                TextureFormat::HALF_FLOAT) != supportedFormats_.end()) {
    return GL_HALF_FLOAT;
  }

  return GL_UNSIGNED_BYTE;
}

void IBLSampler::destroy() {
  // 删除所有纹理
  GLuint textures[] = {
      inputTextureID_, cubemapTextureID_, lambertianTextureID_,
      ggxTextureID_, sheenTextureID_, ggxLutTextureID_, charlieLutTextureID_
  };

  for (GLuint texture: textures) {
    if (texture != 0) {
      glDeleteTextures(1, &texture);
    }
  }

  if (framebuffer_ != 0) {
    glDeleteFramebuffers(1, &framebuffer_);
    framebuffer_ = 0;
  }

  // 重置ID
  inputTextureID_ = cubemapTextureID_ = lambertianTextureID_ = 0;
  ggxTextureID_ = sheenTextureID_ = ggxLutTextureID_ = charlieLutTextureID_ = 0;

  shaderCache_.reset();
}

// HDRImage实现
HDRImage::HDRImage(int width, int height, const float *data)
    : width_(width), height_(height) {
  int numPixels = width * height;
  data_.resize(numPixels * 3);
  std::memcpy(data_.data(), data, numPixels * 3 * sizeof(float));
}

HDRImage::~HDRImage() = default;
} // digitalhunman