//
// Created by vincentsyan on 2025/9/15.
//
#ifndef IBL_SAMPLER_H
#define IBL_SAMPLER_H

#include <GLES3/gl3.h>
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace digitalhumans {

// 前向声明
class ShaderCache;

class GltfOpenGLContext;

struct HDRImage;

struct TextureData {
  GLenum internalFormat;
  GLenum format;
  GLenum type;
  void *data;
  size_t dataSize;
};

class IBLSampler {
 public:
  explicit IBLSampler(const std::shared_ptr<GltfOpenGLContext> &gl_webgl);

  ~IBLSampler();

  // 初始化
  bool init(const HDRImage &panoramaImage);

  // 执行所有预处理
  void filterAll();

  // 销毁资源
  void destroy();

  // 获取生成的纹理
  GLuint getLambertianTextureID() const { return lambertianTextureID_; }

  GLuint getGGXTextureID() const { return ggxTextureID_; }

  GLuint getSheenTextureID() const { return sheenTextureID_; }

  GLuint getGGXLutTextureID() const { return ggxLutTextureID_; }

  GLuint getCharlieLutTextureID() const { return charlieLutTextureID_; }

  GLuint getCubemapTextureID() const { return cubemapTextureID_; }

  float getScaleValue() const { return scaleValue_; }

 private:
  // 核心处理方法
  void panoramaToCubeMap();

  void cubeMapToLambertian();

  void cubeMapToGGX();

  void cubeMapToSheen();

  void sampleGGXLut();

  void sampleCharlieLut();

  // 辅助方法
  TextureData prepareTextureData(const HDRImage &image);

  GLuint loadTextureHDR(const HDRImage &image);

  GLuint createCubemapTexture(bool withMipmaps);

  GLuint createLutTexture();

  void applyFilter(int distribution, float roughness, int targetMipLevel,
                   GLuint targetTexture, int sampleCount, float lodBias = 0.0f);

  void
  sampleLut(int distribution, GLuint targetTexture, int currentTextureSize);

  // 格式检测和设置
  void checkSupportedFormats();

  GLint getInternalFormat() const;

  GLenum getTextureTargetType() const;

 public:
  int getMipmapLevels() const;

 private:
  std::shared_ptr<GltfOpenGLContext> gl_;

  std::shared_ptr<ShaderCache> shaderCache_;

  // 配置参数
  int textureSize_;
  int ggxSampleCount_;
  int lambertianSampleCount_;
  int sheenSampleCount_;
  float lodBias_;
  int lowestMipLevel_;
  int lutResolution_;
  int mipmapLevels_;

  float scaleValue_;

  // 纹理资源
  GLuint inputTextureID_;
  GLuint cubemapTextureID_;
  GLuint lambertianTextureID_;
  GLuint ggxTextureID_;
  GLuint sheenTextureID_;
  GLuint ggxLutTextureID_;
  GLuint charlieLutTextureID_;

  GLuint framebuffer_;

  // 支持的格式
  std::vector<std::string> supportedFormats_;
  std::string preferredFormat_;
};

// HDR图像数据结构
struct HDRImage {
  int width;
  int height;
  std::vector<float> dataFloat;  // RGB浮点数据

  HDRImage() : width(0), height(0) {}

  HDRImage(int w, int h, const float *data)
      : width(w), height(h), dataFloat(data, data + w * h * 3) {}
};
}

#endif // IBL_SAMPLER_H