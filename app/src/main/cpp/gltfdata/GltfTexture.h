//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFTEXTURE_H
#define LIGHTDIGITALHUMAN_GLTFTEXTURE_H

#include "GltfObject.h"
#include <GLES3/gl3.h>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <any>

namespace digitalhumans {

/**
 * @brief KHR纹理变换扩展类
 */
class KHRTextureTransform: public GltfObject {
 public:
  /**
   * @brief 构造函数
   */
  KHRTextureTransform();

  /**
   * @brief 虚析构函数
   */
  virtual ~KHRTextureTransform() = default;


  // === Getter/Setter方法 ===
  const std::array<float, 2> &getOffset() const { return offset; }
  void setOffset(const std::array<float, 2> &offset) { this->offset = offset; }

  const std::array<float, 2> &getScale() const { return scale; }
  void setScale(const std::array<float, 2> &scale) { this->scale = scale; }

  float getRotation() const { return rotation; }
  void setRotation(float rotation) { this->rotation = rotation; }


 private:
  std::array<float, 2> offset;    ///< 偏移量
  std::array<float, 2> scale;     ///< 缩放
  float rotation;                 ///< 旋转角度
};

/**
 * @brief glTF纹理信息类
 */
class GltfTextureInfo: public GltfObject {
 public:
  /**
   * @brief 构造函数
   * @param index 纹理索引
   * @param texCoord 纹理坐标索引
   * @param linear 是否为线性空间
   * @param samplerName 采样器名称
   * @param generateMips 是否生成mipmap
   */
  GltfTextureInfo(std::optional<int> index = std::nullopt,
                  int texCoord = 0,
                  bool linear = true,
                  const std::string &samplerName = "",
                  bool generateMips = true);

  /**
   * @brief 虚析构函数
   */
  ~GltfTextureInfo() override = default;


  // === Getter/Setter方法 ===
  std::optional<int> getIndex() const { return index; }
  void setIndex(std::optional<int> index) { this->index = index; }

  int getTexCoord() const { return texCoord; }
  void setTexCoord(int texCoord) { this->texCoord = texCoord; }

  bool isLinear() const { return linear; }
  void setLinear(bool linear) { this->linear = linear; }

  const std::string &getSamplerName() const { return samplerName; }
  void setSamplerName(const std::string &samplerName) {
    this->samplerName = samplerName;
  }

  float getStrength() const { return strength; }
  void setStrength(float strength) { this->strength = strength; }

  float getScale() const { return scale; }
  void setScale(float scale) { this->scale = scale; }

  bool shouldGenerateMips() const { return generateMips; }
  void setGenerateMips(bool generateMips) { this->generateMips = generateMips; }

  std::shared_ptr<KHRTextureTransform> getTextureTransform() const;
  void setTextureTransform(std::shared_ptr<KHRTextureTransform> transform);


 private:
  std::optional<int> index;                           ///< 纹理引用索引
  int texCoord;                                       ///< UV坐标集索引
  bool linear;                                        ///< 是否线性空间
  std::string samplerName;                            ///< 采样器名称
  float strength;                                     ///< 强度（遮挡贴图用）
  float scale;                                        ///< 缩放（法线贴图用）
  bool generateMips;                                  ///< 是否生成mipmap

  std::unordered_map<std::string, std::any> extensions;  ///< 扩展数据
  void initGl(std::shared_ptr<Gltf> gltf,
              std::shared_ptr<GltfOpenGLContext> openGlContext);
};

/**
 * @brief glTF纹理类
 */
class GltfTexture: public GltfObject {
 public:
  /**
   * @brief 构造函数
   * @param sampler 采样器索引
   * @param source 图像源索引
   * @param type 纹理类型
   */
  GltfTexture(std::optional<int> sampler = std::nullopt,
              std::optional<int> source = std::nullopt,
              GLenum type = GL_TEXTURE_2D);

  /**
   * @brief 虚析构函数
   */
  virtual ~GltfTexture();


  /**
   * @brief 销毁资源
   */
  void destroy();

  // === Getter/Setter方法 ===
  std::optional<int> getSampler() const { return sampler; }
  void setSampler(std::optional<int> sampler) { this->sampler = sampler; }

  std::optional<int> getSource() const { return source; }
  void setSource(std::optional<int> source) { this->source = source; }

  GLuint getGLTexture() const { return glTexture; }
  void setGLTexture(GLuint glTexture) { this->glTexture = glTexture; }

  GLenum getType() const { return type; }
  void setType(GLenum type) { this->type = type; }

  bool isInitialized() const { return initialized; }
  void setInitialized(bool initialized) { this->initialized = initialized; }

  int getMipLevelCount() const { return mipLevelCount; }
  void setMipLevelCount(int mipLevelCount) {
    this->mipLevelCount = mipLevelCount;
  }

  bool isLinear() const { return linear; }
  void setLinear(bool linear) { this->linear = linear; }


 private:
  // === glTF标准属性 ===
  std::optional<int> sampler;     ///< 采样器索引
  std::optional<int> source;      ///< 图像源索引

  // === 非glTF标准属性（运行时数据） ===
  GLuint glTexture;               ///< OpenGL纹理对象
  GLenum type;                    ///< 纹理类型
  bool initialized;               ///< 是否已初始化
  int mipLevelCount;              ///< mip级别数量
  bool linear;                    ///< 是否线性空间


};

} // namespace digitalhumans


#endif //LIGHTDIGITALHUMAN_GLTFTEXTURE_H
