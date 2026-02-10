// GltfEnvironment.h
#ifndef GLTF_ENVIRONMENT_H
#define GLTF_ENVIRONMENT_H

#include <string>
#include <memory>
#include <GLES3/gl3.h>
#include <android/asset_manager.h>

namespace digitalhumans {
class Gltf;
class GltfTextureInfo;

class GltfEnvironment {
 public:
  GltfEnvironment();

  explicit GltfEnvironment(const std::string &envMapPath);

  ~GltfEnvironment();

  // 移动语义
  GltfEnvironment(GltfEnvironment &&other) noexcept;

  GltfEnvironment &operator=(GltfEnvironment &&other) noexcept;

  int getMipCount() const { return mipCount_; }

  float getIBLIntensityScale() const { return iblIntensityScale_; }
  GLuint getSpecularEnvMap() const { return specularEnvMap_; }

  std::shared_ptr<GltfTextureInfo>
  createImageInfo(GLuint &textureId, GLenum type, int classType);

  void setGltf(const std::shared_ptr<Gltf> &gltf);
  std::shared_ptr<GltfTextureInfo>
      diffuseEnvMap;                     ///< 加载的glTF数据
  std::shared_ptr<GltfTextureInfo>
      specularEnvMap;                     ///< 加载的glTF数据
  std::shared_ptr<GltfTextureInfo>
      sheenEnvMap;                     ///< 加载的glTF数据
  std::shared_ptr<GltfTextureInfo> lut;                     ///< 加载的glTF数据
  std::shared_ptr<GltfTextureInfo> sheenELUT;                     ///< 加载的glTF数据
  std::shared_ptr<GltfTextureInfo> sheenLUT;                     ///< 加载的glTF数据

  int mipCount_;
  GLuint diffuseEnvMap_ = 0;
  GLuint specularEnvMap_ = 0;
  GLuint brdfLUT_ = 0;
  GLuint sheenEnvMap_ = 0;
  GLuint sheenLUT_ = 0;
  GLuint sheenELUT_ = 0;

 private:
  // OpenGL纹理ID


  std::shared_ptr<Gltf> gltf;                     ///< 加载的glTF数据

  float iblIntensityScale_ = 1.0f;
  bool isLoaded_;

};

} // namespace digitalhumans

#endif // GLTF_ENVIRONMENT_H