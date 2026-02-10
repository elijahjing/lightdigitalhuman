//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTF_H
#define LIGHTDIGITALHUMAN_GLTF_H


#include <vector>
#include <string>
#include <memory>
#include "GltfObject.h"

namespace digitalhumans {

//class GltfObject;

class GltfLight;

class GltfVariant;

class GltfSampler;

class GltfScene;

class GltfBufferView;

class GltfBuffer;

class GltfAccessor;

class GltfImage;

class GltfAsset;

class GltfMesh;

class GltfSkin;

class GltfAnimation;

class GltfCamera;


class GltfNode;

class ImageBasedLight;

class GltfMaterial;

class GltfTexture;

const std::vector<std::string> ALLOWED_EXTENSIONS = {
    "KHR_animation_pointer",
    "KHR_draco_mesh_compression",
    "KHR_lights_image_based",
    "KHR_lights_punctual",
    "KHR_materials_anisotropy",
    "KHR_materials_clearcoat",
    "KHR_materials_dispersion",
    "KHR_materials_emissive_strength",
    "KHR_materials_ior",
    "KHR_materials_iridescence",
    "KHR_materials_pbrSpecularGlossiness",
    "KHR_materials_sheen",
    "KHR_materials_specular",
    "KHR_materials_transmission",
    "KHR_materials_unlit",
    "KHR_materials_variants",
    "KHR_materials_volume",
    "KHR_mesh_quantization",
    "KHR_texture_basisu",
    "KHR_texture_transform",
    "KHR_xmp_json_ld",
    "EXT_texture_webp",
};

/**
 * @brief glTF根对象类
 * 包含glTF文件的所有资源和数据
 */
class Gltf: public GltfObject {
 public:
  /**
   * @brief 构造函数
   * @param file 文件路径
   */
  Gltf(const std::string &file = "") : GltfObject() {}

  /**
   * @brief 虚析构函数
   */
  ~Gltf() override = default;

  // Getter方法
  std::shared_ptr<GltfAsset> getAsset() const { return asset; }

  const std::vector<std::shared_ptr<GltfAccessor>> &
  getAccessors() const { return accessors; }

  const std::vector<std::shared_ptr<GltfNode>> &
  getNodes() const { return nodes; }

  const std::vector<std::shared_ptr<GltfScene>> &
  getScenes() const { return scenes; }

  const std::vector<std::shared_ptr<GltfCamera>> &
  getCameras() const { return cameras; }

  const std::vector<std::shared_ptr<GltfLight>> &
  getLights() const { return lights; }

  const std::vector<std::shared_ptr<ImageBasedLight>> &
  getImageBasedLights() const { return imageBasedLights; }

  const std::vector<std::shared_ptr<GltfTexture>> &
  getTextures() const { return textures; }

  const std::vector<std::shared_ptr<GltfImage>> &
  getImages() const { return images; }

  const std::vector<std::shared_ptr<GltfSampler>> &
  getSamplers() const { return samplers; }

  const std::vector<std::shared_ptr<GltfMesh>> &
  getMeshes() const { return meshes; }

  const std::vector<std::shared_ptr<GltfBuffer>> &
  getBuffers() const { return buffers; }

  const std::vector<std::shared_ptr<GltfBufferView>> &
  getBufferViews() const { return bufferViews; }

  const std::vector<std::shared_ptr<GltfMaterial>> &
  getMaterials() const { return materials; }

  const std::vector<std::shared_ptr<GltfAnimation>> &
  getAnimations() const { return animations; }

  const std::vector<std::shared_ptr<GltfSkin>> &
  getSkins() const { return skins; }

  const std::vector<std::shared_ptr<GltfVariant>> &
  getVariants() const { return variants; }

  void setAsset(std::shared_ptr<GltfAsset> asset) { this->asset = asset; }

  void setScene(int scene) { this->scene = scene; }

  /**
   * @brief 添加访问器
   * @param accessor 访问器对象
   * @return 添加的访问器在数组中的索引
   */
  int addAccessor(std::shared_ptr<GltfAccessor> accessor);

  /**
   * @brief 添加节点
   * @param node 节点对象
   * @return 添加的节点在数组中的索引
   */
  int addNode(std::shared_ptr<GltfNode> node);

  /**
   * @brief 添加场景
   * @param scene 场景对象
   * @return 添加的场景在数组中的索引
   */
  int addScene(std::shared_ptr<GltfScene> scene);

  /**
   * @brief 添加摄像机
   * @param camera 摄像机对象
   * @return 添加的摄像机在数组中的索引
   */
  int addCamera(const std::shared_ptr<GltfCamera>& camera);

  /**
   * @brief 添加光源
   * @param light 光源对象
   * @return 添加的光源在数组中的索引
   */
  int addLight(const std::shared_ptr<GltfLight>& light);

  /**
   * @brief 添加基于图像的光照
   * @param imageBasedLight 基于图像的光照对象
   * @return 添加的光照在数组中的索引
   */
  int addImageBasedLight(const std::shared_ptr<ImageBasedLight>& imageBasedLight);

  /**
   * @brief 添加纹理
   * @param texture 纹理对象
   * @return 添加的纹理在数组中的索引
   */
  int addTexture(const std::shared_ptr<GltfTexture>& texture);

  /**
   * @brief 添加图像
   * @param image 图像对象
   * @return 添加的图像在数组中的索引
   */
  int addImage(const std::shared_ptr<GltfImage>& image);

  /**
   * @brief 添加采样器
   * @param sampler 采样器对象
   * @return 添加的采样器在数组中的索引
   */
  int addSampler(const std::shared_ptr<GltfSampler>& sampler);

  /**
   * @brief 添加网格
   * @param mesh 网格对象
   * @return 添加的网格在数组中的索引
   */
  int addMesh(const std::shared_ptr<GltfMesh>& mesh);

  /**
   * @brief 添加缓冲区
   * @param buffer 缓冲区对象
   * @return 添加的缓冲区在数组中的索引
   */
  int addBuffer(const std::shared_ptr<GltfBuffer>& buffer);

  /**
   * @brief 添加缓冲区视图
   * @param bufferView 缓冲区视图对象
   * @return 添加的缓冲区视图在数组中的索引
   */
  int addBufferView(const std::shared_ptr<GltfBufferView>& bufferView);

  /**
   * @brief 添加材质
   * @param material 材质对象
   * @return 添加的材质在数组中的索引
   */
  int addMaterial(const std::shared_ptr<GltfMaterial>& material);

  /**
   * @brief 添加动画
   * @param animation 动画对象
   * @return 添加的动画在数组中的索引
   */
  int addAnimation(const std::shared_ptr<GltfAnimation>& animation);

  /**
   * @brief 添加蒙皮
   * @param skin 蒙皮对象
   * @return 添加的蒙皮在数组中的索引
   */
  int addSkin(const std::shared_ptr<GltfSkin>& skin);

  /**
   * @brief 添加材质变体
   * @param variant 材质变体对象
   * @return 添加的变体在数组中的索引
   */
  int addVariant(const std::shared_ptr<GltfVariant>& variant);

  /**
   * @brief 批量添加多个对象
   */
  void
  addAccessors(const std::vector<std::shared_ptr<GltfAccessor>> &accessorList);

  void addNodes(const std::vector<std::shared_ptr<GltfNode>> &nodeList);

  void addScenes(const std::vector<std::shared_ptr<GltfScene>> &sceneList);

  void addCameras(const std::vector<std::shared_ptr<GltfCamera>> &cameraList);

  void addLights(const std::vector<std::shared_ptr<GltfLight>> &lightList);

  void
  addTextures(const std::vector<std::shared_ptr<GltfTexture>> &textureList);

  void addImages(const std::vector<std::shared_ptr<GltfImage>> &imageList);

  void
  addSamplers(const std::vector<std::shared_ptr<GltfSampler>> &samplerList);

  void addMeshes(const std::vector<std::shared_ptr<GltfMesh>> &meshList);

  void addBuffers(const std::vector<std::shared_ptr<GltfBuffer>> &bufferList);

  void
  addBufferViews(const std::vector<std::shared_ptr<GltfBufferView>> &bufferViewList);

  void
  addMaterials(const std::vector<std::shared_ptr<GltfMaterial>> &materialList);

  void
  addAnimations(const std::vector<std::shared_ptr<GltfAnimation>> &animationList);

  void addSkins(const std::vector<std::shared_ptr<GltfSkin>> &skinList);

  void
  addVariants(const std::vector<std::shared_ptr<GltfVariant>> &variantList);

  /**
   * @brief 根据名称查找节点索引
   * @param name 节点名称
   * @return 节点索引，如果未找到返回-1
   */
  int findNodeByName(const std::string &name) const;

  /**
   * @brief 根据名称查找材质索引
   * @param name 材质名称
   * @return 材质索引，如果未找到返回-1
   */
  int findMaterialByName(const std::string &name) const;

  /**
   * @brief 根据名称查找动画索引
   * @param name 动画名称
   * @return 动画索引，如果未找到返回-1
   */
  int findAnimationByName(const std::string &name) const;

  // === 验证方法 ===
  /**
   * @brief 验证索引是否有效
   */
  bool isValidAccessorIndex(int index) const;

  bool isValidNodeIndex(int index) const;

  bool isValidSceneIndex(int index) const;

  bool isValidCameraIndex(int index) const;

  bool isValidLightIndex(int index) const;

  bool isValidTextureIndex(int index) const;

  bool isValidImageIndex(int index) const;

  bool isValidSamplerIndex(int index) const;

  bool isValidMeshIndex(int index) const;

  bool isValidBufferIndex(int index) const;

  bool isValidBufferViewIndex(int index) const;

  bool isValidMaterialIndex(int index) const;

  bool isValidAnimationIndex(int index) const;

  bool isValidSkinIndex(int index) const;

  bool isValidVariantIndex(int index) const;



 public:
  // === glTF标准属性 ===
  std::shared_ptr<GltfAsset> asset;                           ///< 资产信息
  std::vector<std::shared_ptr<GltfAccessor>> accessors;       ///< 访问器数组
  std::vector<std::shared_ptr<GltfNode>> nodes;               ///< 节点数组
  int scene;                                                  ///< 默认显示的场景索引
  std::vector<std::shared_ptr<GltfScene>> scenes;             ///< 场景数组
  std::vector<std::shared_ptr<GltfCamera>> cameras;           ///< 摄像机数组
  std::vector<std::shared_ptr<GltfLight>> lights;             ///< 点光源数组
  std::vector<std::shared_ptr<ImageBasedLight>> imageBasedLights; ///< 基于图像的光照数组
  std::vector<std::shared_ptr<GltfTexture>> textures;         ///< 纹理数组
  std::vector<std::shared_ptr<GltfImage>> images;             ///< 图像数组
  std::vector<std::shared_ptr<GltfSampler>> samplers;         ///< 采样器数组
  std::vector<std::shared_ptr<GltfMesh>> meshes;              ///< 网格数组
  std::vector<std::shared_ptr<GltfBuffer>> buffers;           ///< 缓冲区数组
  std::vector<std::shared_ptr<GltfBufferView>> bufferViews;   ///< 缓冲区视图数组
  std::vector<std::shared_ptr<GltfMaterial>> materials;       ///< 材质数组
  std::vector<std::shared_ptr<GltfAnimation>> animations;     ///< 动画数组
  std::vector<std::shared_ptr<GltfSkin>> skins;               ///< 蒙皮数组
  std::vector<std::shared_ptr<GltfVariant>> variants;         ///< 材质变体数组
};

} // namespace digitalhumans

#endif //LIGHTDIGITALHUMAN_GLTF_H