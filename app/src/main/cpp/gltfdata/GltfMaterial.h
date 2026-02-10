//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFMATERIAL_H
#define LIGHTDIGITALHUMAN_GLTFMATERIAL_H


#include "GltfObject.h"
#include "GltfTexture.h"
#include "vec4.hpp"
#include "vec3.hpp"
#include "tiny_gltf.h"
#include <array>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <optional>

namespace digitalhumans {
class GltfState;
class RenderingParameters;
class GltfShader;

/**
 * @brief 纹理变换信息
 */
struct TextureTransform {
  std::string key;                           ///< 变换键名
  std::optional<float> rotation;             ///< 旋转角度
  std::optional<std::array<float, 2>> scale; ///< 缩放
  std::optional<std::array<float, 2>> offset; ///< 偏移
  std::optional<int> texCoord;               ///< 纹理坐标索引
};

/**
 * @brief Alpha混合模式枚举
 */
enum class AlphaMode {
  OPAQUE = 0,
  MASK = 1,
  BLEND = 2
};

/**
 * @brief 材质类型枚举
 */
enum class MaterialType {
  UNLIT,
  MR,  // Metallic Roughness
  SG   // Specular Glossiness
};

// ===== 材质扩展类 =====

/**
 * @brief KHR_materials_pbrSpecularGlossiness扩展
 */
class KHRMaterialsPbrSpecularGlossiness: public GltfObject {
 public:
  KHRMaterialsPbrSpecularGlossiness();
  virtual ~KHRMaterialsPbrSpecularGlossiness() = default;


  const std::array<float, 4> &getDiffuseFactor() const { return diffuseFactor; }
  void setDiffuseFactor(const std::array<float, 4> &factor) {
    diffuseFactor = factor;
  }

  std::shared_ptr<GltfTextureInfo>
  getDiffuseTexture() const { return diffuseTexture; }
  void setDiffuseTexture(std::shared_ptr<GltfTextureInfo> texture) {
    diffuseTexture = texture;
  }

  const std::array<float, 3> &
  getSpecularFactor() const { return specularFactor; }
  void setSpecularFactor(const std::array<float, 3> &factor) {
    specularFactor = factor;
  }

  std::shared_ptr<GltfTextureInfo>
  getSpecularGlossinessTexture() const { return specularGlossinessTexture; }
  void setSpecularGlossinessTexture(std::shared_ptr<GltfTextureInfo> texture) {
    specularGlossinessTexture = texture;
  }

  float getGlossinessFactor() const { return glossinessFactor; }
  void setGlossinessFactor(float factor) { glossinessFactor = factor; }

 private:
  std::array<float, 4> diffuseFactor;
  std::shared_ptr<GltfTextureInfo> diffuseTexture;
  std::array<float, 3> specularFactor;
  std::shared_ptr<GltfTextureInfo> specularGlossinessTexture;
  float glossinessFactor;
};

/**
 * @brief KHR_materials_clearcoat扩展
 */
class KHRMaterialsClearcoat: public GltfObject {
 public:
  KHRMaterialsClearcoat();
  virtual ~KHRMaterialsClearcoat() = default;

  float getClearcoatFactor() const { return clearcoatFactor; }
  void setClearcoatFactor(float factor) { clearcoatFactor = factor; }

  std::shared_ptr<GltfTextureInfo>
  getClearcoatTexture() const { return clearcoatTexture; }
  void setClearcoatTexture(std::shared_ptr<GltfTextureInfo> texture) {
    clearcoatTexture = texture;
  }

  float getClearcoatRoughnessFactor() const { return clearcoatRoughnessFactor; }
  void setClearcoatRoughnessFactor(float factor) {
    clearcoatRoughnessFactor = factor;
  }

  std::shared_ptr<GltfTextureInfo>
  getClearcoatRoughnessTexture() const { return clearcoatRoughnessTexture; }
  void setClearcoatRoughnessTexture(std::shared_ptr<GltfTextureInfo> texture) {
    clearcoatRoughnessTexture = texture;
  }

  std::shared_ptr<GltfTextureInfo>
  getClearcoatNormalTexture() const { return clearcoatNormalTexture; }
  void setClearcoatNormalTexture(std::shared_ptr<GltfTextureInfo> texture) {
    clearcoatNormalTexture = texture;
  }

 private:
  float clearcoatFactor;
  std::shared_ptr<GltfTextureInfo> clearcoatTexture;
  float clearcoatRoughnessFactor;
  std::shared_ptr<GltfTextureInfo> clearcoatRoughnessTexture;
  std::shared_ptr<GltfTextureInfo> clearcoatNormalTexture;
};

/**
 * @brief KHR_materials_sheen扩展
 */
class KHRMaterialsSheen: public GltfObject {
 public:
  KHRMaterialsSheen();
  virtual ~KHRMaterialsSheen() = default;

  float getSheenRoughnessFactor() const { return sheenRoughnessFactor; }
  void setSheenRoughnessFactor(float factor) { sheenRoughnessFactor = factor; }

  const std::array<float, 3> &
  getSheenColorFactor() const { return sheenColorFactor; }
  void setSheenColorFactor(const std::array<float, 3> &factor) {
    sheenColorFactor = factor;
  }

  std::shared_ptr<GltfTextureInfo>
  getSheenColorTexture() const { return sheenColorTexture; }
  void setSheenColorTexture(std::shared_ptr<GltfTextureInfo> texture) {
    sheenColorTexture = texture;
  }

  std::shared_ptr<GltfTextureInfo>
  getSheenRoughnessTexture() const { return sheenRoughnessTexture; }
  void setSheenRoughnessTexture(std::shared_ptr<GltfTextureInfo> texture) {
    sheenRoughnessTexture = texture;
  }

 private:
  float sheenRoughnessFactor;
  std::array<float, 3> sheenColorFactor;
  std::shared_ptr<GltfTextureInfo> sheenColorTexture;
  std::shared_ptr<GltfTextureInfo> sheenRoughnessTexture;
};

/**
 * @brief KHR_materials_transmission扩展
 */
class KHRMaterialsTransmission: public GltfObject {
 public:
  KHRMaterialsTransmission();
  virtual ~KHRMaterialsTransmission() = default;
  float getTransmissionFactor() const { return transmissionFactor; }
  void setTransmissionFactor(float factor) { transmissionFactor = factor; }

  std::shared_ptr<GltfTextureInfo>
  getTransmissionTexture() const { return transmissionTexture; }
  void setTransmissionTexture(std::shared_ptr<GltfTextureInfo> texture) {
    transmissionTexture = texture;
  }

 private:
  float transmissionFactor;
  std::shared_ptr<GltfTextureInfo> transmissionTexture;
};

// 其他扩展类的声明（简化）
class KHRMaterialsSpecular;
class KHRMaterialsVolume;
class KHRMaterialsIridescence;
class KHRMaterialsAnisotropy;
class KHRMaterialsEmissiveStrength;
class KHRMaterialsIor;
class KHRMaterialsDispersion;
class KHRMaterialsDiffuseTransmission;

/**
 * @brief PBR金属粗糙度工作流
 */
class PbrMetallicRoughness: public GltfObject {
 public:
  PbrMetallicRoughness();
  virtual ~PbrMetallicRoughness() = default;

  const glm::vec4 getBaseColorFactor() const { return baseColorFactor; }
  void setBaseColorFactor(const glm::vec4 &factor) { baseColorFactor = factor; }

  std::shared_ptr<GltfTextureInfo>
  getBaseColorTexture() const { return baseColorTexture; }
  void setBaseColorTexture(std::shared_ptr<GltfTextureInfo> texture) {
    baseColorTexture = texture;
  }

  float getMetallicFactor() const { return metallicFactor; }
  void setMetallicFactor(float factor) { metallicFactor = factor; }

  float getRoughnessFactor() const { return roughnessFactor; }
  void setRoughnessFactor(float factor) { roughnessFactor = factor; }

  std::shared_ptr<GltfTextureInfo>
  getMetallicRoughnessTexture() const { return metallicRoughnessTexture; }
  void setMetallicRoughnessTexture(std::shared_ptr<GltfTextureInfo> texture) {
    metallicRoughnessTexture = texture;
  }

 private:
  glm::vec4 baseColorFactor;
  std::shared_ptr<GltfTextureInfo> baseColorTexture;
  float metallicFactor;
  float roughnessFactor;
  std::shared_ptr<GltfTextureInfo> metallicRoughnessTexture;
};

/**
 * @brief glTF材质类
 */
class GltfMaterial: public GltfObject {
 public:
  /**
   * @brief 构造函数
   */
  GltfMaterial();

  /**
   * @brief 虚析构函数
   */
  virtual ~GltfMaterial() = default;

  /**
   * @brief 创建默认材质
   * @return 默认材质实例
   */
  static std::shared_ptr<GltfMaterial> createDefault();



  /**
   * @brief 获取材质宏定义
   * @param renderingParameters 渲染参数
   * @return 宏定义列表
   */
  std::vector<std::string>
  getDefines(const RenderingParameters &renderingParameters) const;

  /**
   * @brief 更新纹理变换
   * @param shader 着色器对象
   */
  void updateTextureTransforms(std::shared_ptr<GltfShader> shader);

  // === Getter/Setter方法 ===
  const std::string &getName() const { return name; }
  void setName(const std::string &name) { this->name = name; }

  std::shared_ptr<PbrMetallicRoughness>
  getPbrMetallicRoughness() const { return pbrMetallicRoughness; }
  void setPbrMetallicRoughness(std::shared_ptr<PbrMetallicRoughness> pbr) {
    pbrMetallicRoughness = pbr;
  }

  std::shared_ptr<GltfTextureInfo>
  getNormalTexture() const { return normalTexture; }
  void setNormalTexture(std::shared_ptr<GltfTextureInfo> texture) {
    normalTexture = texture;
  }

  std::shared_ptr<GltfTextureInfo>
  getOcclusionTexture() const { return occlusionTexture; }
  void setOcclusionTexture(std::shared_ptr<GltfTextureInfo> texture) {
    occlusionTexture = texture;
  }

  std::shared_ptr<GltfTextureInfo>
  getEmissiveTexture() const { return emissiveTexture; }
  void setEmissiveTexture(std::shared_ptr<GltfTextureInfo> texture) {
    emissiveTexture = texture;
  }

  const glm::vec3 &getEmissiveFactor() const { return emissiveFactor; }
  void setEmissiveFactor(const glm::vec3 &factor) { emissiveFactor = factor; }

  AlphaMode getAlphaMode() const { return alphaMode; }
  void setAlphaMode(AlphaMode mode) { alphaMode = mode; }

  float getAlphaCutoff() const { return alphaCutoff; }
  void setAlphaCutoff(float cutoff) { alphaCutoff = cutoff; }

  bool isDoubleSided() const { return doubleSided; }
  void setDoubleSided(bool doubleSided) { this->doubleSided = doubleSided; }

  MaterialType getType() const { return type; }
  void setType(MaterialType type) { this->type = type; }

  const std::vector<std::shared_ptr<GltfTextureInfo>> &
  getTextures() const { return textures; }
  const std::vector<std::string> &getMaterialDefines() const { return defines; }

  // === 扩展标志 ===
  bool hasClearcoatExtension() const { return hasClearcoat; }

  const std::vector<TextureTransform> &getTextureTransforms() const;

  bool hasSheenExtension() const { return hasSheen; }
  bool hasTransmissionExtension() const { return hasTransmission; }
  bool
  hasDiffuseTransmissionExtension() const { return hasDiffuseTransmission; }
  bool hasIorExtension() const { return hasIOR; }
  bool hasEmissiveStrengthExtension() const { return hasEmissiveStrength; }
  bool hasVolumeExtension() const { return hasVolume; }
  bool hasIridescenceExtension() const { return hasIridescence; }
  bool hasAnisotropyExtension() const { return hasAnisotropy; }
  bool hasDispersionExtension() const { return hasDispersion; }
  bool hasSpecularExtension() const { return hasSpecular; }


  bool hasExtension() const { return false; } // TODO

  void initObjects();



 private:


  /**
   * @brief 解析纹理信息扩展
   * @param textureInfo 纹理信息对象
   * @param textureKey 纹理键名
   */
  void parseTextureInfoExtensions(std::shared_ptr<GltfTextureInfo> textureInfo,
                                  const std::string &textureKey);

 public:
  void setHasClearcoat(bool hasClearcoat);

  void setHasSheen(bool hasSheen);

  void setHasTransmission(bool hasTransmission);

  void setHasDiffuseTransmission(bool hasDiffuseTransmission);

  void setHasIor(bool hasIor);

  void setHasEmissiveStrength(bool hasEmissiveStrength);

  void setHasVolume(bool hasVolume);

  void setHasIridescence(bool hasIridescence);

  void setHasAnisotropy(bool hasAnisotropy);

  void setHasDispersion(bool hasDispersion);

  void setHasSpecular(bool hasSpecular);

 private:
  // === glTF标准属性 ===
  std::string name;                                           ///< 材质名称
  std::shared_ptr<PbrMetallicRoughness> pbrMetallicRoughness; ///< PBR金属粗糙度
  std::shared_ptr<GltfTextureInfo> normalTexture;             ///< 法线贴图
  std::shared_ptr<GltfTextureInfo> occlusionTexture;          ///< 遮挡贴图
  std::shared_ptr<GltfTextureInfo> emissiveTexture;           ///< 发光贴图
  glm::vec3 emissiveFactor;                                   ///< 发光因子
  AlphaMode alphaMode;                                        ///< Alpha混合模式
  float alphaCutoff;                                          ///< Alpha裁剪阈值
  bool doubleSided;                                           ///< 是否双面

  // === 扩展标志 ===
  bool hasClearcoat;                                          ///< 是否有清漆扩展
  bool hasSheen;                                              ///< 是否有光泽扩展
  bool hasTransmission;                                       ///< 是否有透射扩展
  bool hasDiffuseTransmission;                                ///< 是否有漫反射透射扩展
  bool hasIOR;                                                ///< 是否有折射率扩展
  bool hasEmissiveStrength;                                   ///< 是否有发光强度扩展
  bool hasVolume;                                             ///< 是否有体积扩展
  bool hasIridescence;                                        ///< 是否有彩虹色扩展
  bool hasAnisotropy;                                         ///< 是否有各向异性扩展
  bool hasDispersion;                                         ///< 是否有色散扩展
  bool hasSpecular;                                           ///< 是否有镜面扩展

  // === 非glTF标准属性（运行时数据） ===
  MaterialType type;                                          ///< 材质类型
  std::vector<std::shared_ptr<GltfTextureInfo>> textures;     ///< 纹理列表
  std::vector<TextureTransform> textureTransforms;            ///< 纹理变换列表
  std::vector<std::string> defines;                           ///< 着色器宏定义

//        // === 扩展对象映射 ===
//        std::unordered_map<std::string, std::shared_ptr<GltfObject>> extensions;

  tinygltf::ExtensionMap extensions;
  template<typename T>
  std::shared_ptr<T> getExtension(const std::string &extensionName) const;

  void processExtensions();
};

} // namespace digitalhumans



#endif //LIGHTDIGITALHUMAN_GLTFMATERIAL_H
