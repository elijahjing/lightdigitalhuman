//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_IMAGEBASEDLIGHT_H
#define LIGHTDIGITALHUMAN_IMAGEBASEDLIGHT_H


#ifndef IMAGE_BASED_LIGHT_H
#define IMAGE_BASED_LIGHT_H

#include "GltfObject.h"
#include "../utils.h"
#include <array>
#include <string>
#include <memory>
#include <optional>

namespace digitalhumans {


/**
 * @brief 基于图像的照明（IBL）类
 *
 * 实现了glTF KHR_lights_image_based扩展
 * https://github.com/KhronosGroup/glTF/blob/khr_ktx2_ibl/extensions/2.0/Khronos/KHR_lights_image_based/schema/imageBasedLight.schema.json
 */
class ImageBasedLight: public GltfObject {
 public:
  /**
   * @brief 构造函数
   */
  ImageBasedLight();

  /**
   * @brief 虚析构函数
   */
  virtual ~ImageBasedLight() = default;


  /**
   * @brief 获取旋转四元数
   * @return 旋转四元数 [x, y, z, w]
   */
  const std::array<float, 4> &getRotation() const { return rotation; }

  /**
   * @brief 设置旋转四元数
   * @param rotation 旋转四元数 [x, y, z, w]
   */
  void setRotation(const std::array<float, 4> &rotation) {
    this->rotation = rotation;
  }

  /**
   * @brief 获取亮度因子
   * @return 亮度因子
   */
  float getBrightnessFactor() const { return brightnessFactor; }

  /**
   * @brief 设置亮度因子
   * @param factor 亮度因子
   */
  void setBrightnessFactor(float factor) { brightnessFactor = factor; }

  /**
   * @brief 获取亮度偏移
   * @return 亮度偏移
   */
  float getBrightnessOffset() const { return brightnessOffset; }

  /**
   * @brief 设置亮度偏移
   * @param offset 亮度偏移
   */
  void setBrightnessOffset(float offset) { brightnessOffset = offset; }

  /**
   * @brief 获取镜面环境纹理索引
   * @return 镜面环境纹理索引，如果未设置则返回std::nullopt
   */
  const std::optional<int> &
  getSpecularEnvironmentTexture() const { return specularEnvironmentTexture; }

  /**
   * @brief 设置镜面环境纹理索引
   * @param index 纹理索引
   */
  void setSpecularEnvironmentTexture(int index) {
    specularEnvironmentTexture = index;
  }

  /**
   * @brief 清除镜面环境纹理索引
   */
  void clearSpecularEnvironmentTexture() { specularEnvironmentTexture.reset(); }

  /**
   * @brief 获取漫反射环境纹理索引
   * @return 漫反射环境纹理索引，如果未设置则返回std::nullopt
   */
  const std::optional<int> &
  getDiffuseEnvironmentTexture() const { return diffuseEnvironmentTexture; }

  /**
   * @brief 设置漫反射环境纹理索引
   * @param index 纹理索引
   */
  void setDiffuseEnvironmentTexture(int index) {
    diffuseEnvironmentTexture = index;
  }

  /**
   * @brief 清除漫反射环境纹理索引
   */
  void clearDiffuseEnvironmentTexture() { diffuseEnvironmentTexture.reset(); }

  /**
   * @brief 获取光泽环境纹理索引
   * @return 光泽环境纹理索引，如果未设置则返回std::nullopt
   */
  const std::optional<int> &
  getSheenEnvironmentTexture() const { return sheenEnvironmentTexture; }

  /**
   * @brief 设置光泽环境纹理索引
   * @param index 纹理索引
   */
  void setSheenEnvironmentTexture(int index) {
    sheenEnvironmentTexture = index;
  }

  /**
   * @brief 清除光泽环境纹理索引
   */
  void clearSheenEnvironmentTexture() { sheenEnvironmentTexture.reset(); }

  /**
   * @brief 获取Mipmap层级数量
   * @return Mipmap层级数量
   */
  int getLevelCount() const { return levelCount; }

  /**
   * @brief 设置Mipmap层级数量
   * @param count Mipmap层级数量
   */
  void setLevelCount(int count) { levelCount = count; }

  /**
   * @brief 检查是否有有效的环境纹理
   * @return 如果有任何环境纹理返回true
   */
  bool hasEnvironmentTextures() const;

  /**
   * @brief 检查是否有镜面环境纹理
   * @return 如果有镜面环境纹理返回true
   */
  bool
  hasSpecularEnvironmentTexture() const { return specularEnvironmentTexture.has_value(); }

  /**
   * @brief 检查是否有漫反射环境纹理
   * @return 如果有漫反射环境纹理返回true
   */
  bool
  hasDiffuseEnvironmentTexture() const { return diffuseEnvironmentTexture.has_value(); }

  /**
   * @brief 检查是否有光泽环境纹理
   * @return 如果有光泽环境纹理返回true
   */
  bool
  hasSheenEnvironmentTexture() const { return sheenEnvironmentTexture.has_value(); }

 private:
  std::array<float, 4> rotation;              ///< 旋转四元数 [x, y, z, w]
  float brightnessFactor;                     ///< 亮度因子
  float brightnessOffset;                     ///< 亮度偏移
  std::optional<int> specularEnvironmentTexture;  ///< 镜面环境纹理索引
  std::optional<int> diffuseEnvironmentTexture;   ///< 漫反射环境纹理索引
  std::optional<int> sheenEnvironmentTexture;     ///< 光泽环境纹理索引

  // 非glTF标准属性
  int levelCount;                             ///< Mipmap层级数量
};

} // namespace digitalhumans

#endif // IMAGE_BASED_LIGHT_H

#endif //LIGHTDIGITALHUMAN_IMAGEBASEDLIGHT_H
