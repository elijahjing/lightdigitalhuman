//
// Created by vincentsyan on 2025/9/16.
//

#ifndef LIGHTDIGITALHUMAN_IBLSAMPLER_H
#define LIGHTDIGITALHUMAN_IBLSAMPLER_H
// IBLSampler.h
#pragma once

#include <GLES3/gl3.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace digitalhunman {

class ShaderCache;
class HDRImage;

class IBLSampler {
 public:
  explicit IBLSampler(int contextWidth = 512, int contextHeight = 512);
  ~IBLSampler();

  bool initialize();
  void destroy();
  bool processHDRImage(const HDRImage &panoramaImage);
  GLuint getLambertianTexture() const { return lambertianTextureID_; }
  GLuint getGGXTexture() const { return ggxTextureID_; }
  GLuint getSheenTexture() const { return sheenTextureID_; }
  GLuint getGGXLutTexture() const { return ggxLutTextureID_; }
  GLuint getCharlieLutTexture() const { return charlieLutTextureID_; }

  int getMipLevels() const { return mipmapLevels_; }
  float getScaleValue() const { return scaleValue_; }

 private:
  // 配置参数
  struct Config {
    int textureSize = 256;
    int ggxSampleCount = 1024;
    int lambertianSampleCount = 2048;
    int sheenSampleCount = 64;
    float lodBias = 0.0f;
    int lowestMipLevel = 4;
    int lutResolution = 1024;
  };

  // 纹理格式支持
  enum class TextureFormat {
    BYTE,
    HALF_FLOAT,
    FLOAT
  };

  Config config_;
  std::vector<TextureFormat> supportedFormats_;
  TextureFormat preferredFormat_ = TextureFormat::HALF_FLOAT;

  float scaleValue_ = 1.0f;
  int mipmapLevels_ = 0;

  // OpenGL资源
  GLuint inputTextureID_ = 0;
  GLuint cubemapTextureID_ = 0;
  GLuint lambertianTextureID_ = 0;
  GLuint ggxTextureID_ = 0;
  GLuint sheenTextureID_ = 0;
  GLuint ggxLutTextureID_ = 0;
  GLuint charlieLutTextureID_ = 0;
  GLuint framebuffer_ = 0;

  std::unique_ptr<ShaderCache> shaderCache_;

  // 内部处理函数
  bool checkExtensionSupport();
  bool createShaders();
  GLuint loadTextureHDR(const HDRImage &image);
  GLuint createCubemapTexture(bool withMipmaps);
  GLuint createLutTexture();

  void panoramaToCubeMap();
  void cubeMapToLambertian();
  void cubeMapToGGX();
  void cubeMapToSheen();
  void sampleGGXLut();
  void sampleCharlieLut();

  void applyFilter(int distribution, float roughness, int targetMipLevel,
                   GLuint targetTexture, int sampleCount, float lodBias = 0.0f);
  void sampleLut(int distribution, GLuint targetTexture, int textureSize);

  GLenum getInternalFormat() const;
  GLenum getTextureType() const;

  struct TextureData prepareTextureData(const HDRImage &image);
};

class HDRImage {
 public:
  HDRImage(int width, int height, const float *data);
  ~HDRImage();

  int getWidth() const { return width_; }
  int getHeight() const { return height_; }
  const float *getData() const { return data_.data(); }

 private:
  int width_, height_;
  std::vector<float> data_;
};

} // digitalhunman

#endif //LIGHTDIGITALHUMAN_IBLSAMPLER_H
