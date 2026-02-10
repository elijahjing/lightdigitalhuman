//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_IMAGEMIMETYPES_H
#define LIGHTDIGITALHUMAN_IMAGEMIMETYPES_H


#include <string>
#include <unordered_map>
#include <iostream>
#include <vector>

namespace digitalhumans {

/**
 * @brief 图像MIME类型枚举
 */
enum class ImageMimeType {
  JPEG,
  PNG,
  WEBP,
  HDR,
  KTX2,
  GLTEXTURE,
  UNKNOWN
};

/**
 * @brief 图像MIME类型工具类
 */
class ImageMimeTypes {
 public:
  // MIME类型字符串常量
  static const std::string JPEG;
  static const std::string PNG;
  static const std::string WEBP;
  static const std::string HDR;
  static const std::string KTX2;
  static const std::string GLTEXTURE;

  /**
   * @brief 从枚举值获取MIME类型字符串
   * @param type 图像MIME类型枚举值
   * @return MIME类型字符串
   */
  static std::string toString(ImageMimeType type);

  /**
   * @brief 从MIME类型字符串获取枚举值
   * @param mimeType MIME类型字符串
   * @return 图像MIME类型枚举值
   */
  static ImageMimeType fromString(const std::string &mimeType);

  /**
   * @brief 检查是否为有效的图像MIME类型
   * @param mimeType MIME类型字符串
   * @return 如果是有效的图像MIME类型返回true，否则返回false
   */
  static bool isValidImageMimeType(const std::string &mimeType);

  /**
   * @brief 从文件扩展名推断MIME类型
   * @param extension 文件扩展名（包含或不包含点号）
   * @return 推断的MIME类型枚举值
   */
  static ImageMimeType fromFileExtension(const std::string &extension);

  /**
   * @brief 获取MIME类型对应的文件扩展名
   * @param type 图像MIME类型枚举值
   * @return 文件扩展名（包含点号）
   */
  static std::string getFileExtension(ImageMimeType type);

  /**
   * @brief 检查MIME类型是否支持透明度
   * @param type 图像MIME类型枚举值
   * @return 如果支持透明度返回true，否则返回false
   */
  static bool supportsTransparency(ImageMimeType type);

  /**
   * @brief 检查MIME类型是否为压缩纹理格式
   * @param type 图像MIME类型枚举值
   * @return 如果是压缩纹理格式返回true，否则返回false
   */
  static bool isCompressedFormat(ImageMimeType type);

  /**
   * @brief 获取所有支持的MIME类型字符串
   * @return 包含所有支持的MIME类型字符串的vector
   */
  static std::vector<std::string> getAllMimeTypes();

 private:
  // 静态映射表，用于快速查找
  static const std::unordered_map<std::string, ImageMimeType> stringToEnum;
  static const std::unordered_map<ImageMimeType, std::string> enumToString;
  static const std::unordered_map<std::string, ImageMimeType> extensionToEnum;
  static const std::unordered_map<ImageMimeType, std::string> enumToExtension;
};

} // namespace digitalhumans

#endif //LIGHTDIGITALHUMAN_IMAGEMIMETYPES_H
