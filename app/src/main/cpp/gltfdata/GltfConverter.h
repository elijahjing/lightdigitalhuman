//
// Created by vincentsyan on 2025/8/19.
//

#ifndef LIGHTDIGITALHUMAN_GLTFCONVERTER_H
#define LIGHTDIGITALHUMAN_GLTFCONVERTER_H

#include "tiny_gltf.h"
#include <memory>

namespace digitalhumans {
class Gltf;

class ImageData;

enum class ImageMimeType;

enum class InterpolationMode;

enum class InterpolationPath;
class GltfBuffer;
class GltfAsset;
class GltfBufferView;
class GltfAccessor;
class GltfImage;
class GltfSampler;
class GltfTexture;
class GltfMaterial;
class GltfMesh;
class GltfNode;
class GltfScene;
class GltfSkin;
class GltfCamera;
class GltfAnimation;
class GltfLight;
class GltfObject;
class Engine;
class GltfConverter {
 public:
  /**
   * @brief 从 tiny_gltf::Model 转换到自定义 Gltf 对象
   */
// 将有默认值的参数放到最后
  static std::shared_ptr<Gltf> convert(const tinygltf::Model &model,
                                       Engine &gltfView,
                                       const std::string &filePath = "");
 private:
  // 转换各种组件
  static std::shared_ptr<GltfAsset> convertAsset(const tinygltf::Asset &asset);

  static std::shared_ptr<GltfAccessor>
  convertAccessor(const tinygltf::Accessor &accessor);

  static std::shared_ptr<GltfBuffer>
  convertBuffer(const tinygltf::Buffer &buffer);

  static std::shared_ptr<GltfBufferView>
  convertBufferView(const tinygltf::BufferView &bufferView);

  static std::shared_ptr<GltfImage> convertImage(const tinygltf::Image &image);

  static std::shared_ptr<GltfSampler>
  convertSampler(const tinygltf::Sampler &sampler);

  static std::shared_ptr<GltfTexture>
  convertTexture(const tinygltf::Texture &texture);

  static std::shared_ptr<GltfMaterial>
  convertMaterial(const tinygltf::Material &material);

  static std::shared_ptr<GltfMesh>
  convertMesh(const tinygltf::Mesh &mesh, std::shared_ptr<Gltf> gltf,
              Engine &gltfView);

  static std::shared_ptr<GltfNode> convertNode(const tinygltf::Node &node);

  static std::shared_ptr<GltfScene> convertScene(const tinygltf::Scene &scene);

  static std::shared_ptr<GltfSkin> convertSkin(const tinygltf::Skin &skin);

  static std::shared_ptr<GltfCamera>
  convertCamera(const tinygltf::Camera &camera);

  static std::shared_ptr<GltfAnimation>
  convertAnimation(const tinygltf::Animation &animation);

  static std::shared_ptr<ImageData>
  createImageDataFromPixels(const tinygltf::Image &image);

  static ImageMimeType convertMimeType(const std::string &mimeType);

  static bool inferLinearSpace(const tinygltf::Texture &texture,
                               std::optional<int> sourceIndex);

  static void convertKHRTextureTransform(const tinygltf::Value &extValue,
                                         GltfTexture *gltfTexture);

  static void convertTextureExtensions(const tinygltf::ExtensionMap &extensions,
                                       GltfTexture *gltfTexture);

  // 工具函数
  static void convertExtensions(const tinygltf::ExtensionMap &extensions,
                                GltfObject *object);

  static void convertExtras(const tinygltf::Value &extras, GltfObject *object);

  static InterpolationMode
  convertInterpolationMode(const std::string &interpolation);

  static InterpolationPath convertTargetPath(const std::string &targetPath);

  static bool convertOrthographicCamera(const tinygltf::Camera &camera,
                                        GltfCamera *gltfCamera);

  static bool convertPerspectiveCamera(const tinygltf::Camera &camera,
                                       GltfCamera *gltfCamera);

  /**
* @brief 转换灯光数据
* @param model tinygltf模型
* @return 转换后的灯光列表
*/
  static std::vector<std::shared_ptr<GltfLight>>
  convertLights(const tinygltf::Model &model);

  /**
   * @brief 转换单个灯光
   * @param lightData 灯光JSON数据
   * @param index 灯光索引
   * @return 转换后的灯光对象
   */
  static std::shared_ptr<GltfLight>
  convertSingleLight(const tinygltf::Value &lightData, int index);

};

} // namespace digitalhumans



#endif //LIGHTDIGITALHUMAN_GLTFCONVERTER_H
