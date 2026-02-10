//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFIMAGE_H
#define LIGHTDIGITALHUMAN_GLTFIMAGE_H


#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <future>
#include <GLES3/gl3.h>
#include <memory>
#include "GltfObject.h"
#include "ImageMimeTypes.h"

namespace digitalhumans {
class Gltf;

/**
* @brief 图像数据类
*/
class ImageData {
 public:
  ImageData() = default;

  virtual ~ImageData() = default;

  virtual int getWidth() const = 0;

  virtual int getHeight() const = 0;

  virtual int getChannels() const = 0;

  virtual const uint8_t *getData() const = 0;

  virtual size_t getDataSize() const = 0;

  virtual int getLevelCount() const { return 1; }

  virtual GLenum getInternalFormat() const { return GL_RGBA8; }

  virtual GLenum getFormat() const { return GL_RGBA; }

  virtual GLenum getType() const { return GL_UNSIGNED_BYTE; }

  virtual bool isCompressed() const { return false; }
};

/**
 * @brief 基础图像数据实现
 */
class BasicImageData: public ImageData {
 public:
  BasicImageData(int width,
                 int height,
                 int channels,
                 std::vector<uint8_t> data);

  ~BasicImageData() override = default;

  int getWidth() const override { return width; }

  int getHeight() const override { return height; }

  int getChannels() const override { return channels; }

  const uint8_t *getData() const override { return data.data(); }

  size_t getDataSize() const override { return data.size(); }

 private:
  int width;
  int height;
  int channels;
  std::vector<uint8_t> data;
};

/**
 * @brief KTX2图像数据
 */
class KtxImageData: public ImageData {
 public:
  KtxImageData(int width,
               int height,
               int levelCount,
               GLenum internalFormat,
               GLenum format,
               GLenum type,
               std::vector<uint8_t> data,
               bool compressed);

  ~KtxImageData() override = default;

  int getWidth() const override { return width; }

  int getHeight() const override { return height; }

  int getChannels() const override { return 4; } // 通常为RGBA
  const uint8_t *getData() const override { return data.data(); }

  size_t getDataSize() const override { return data.size(); }

  int getLevelCount() const override { return levelCount; }

  GLenum getInternalFormat() const override { return internalFormat; }

  GLenum getFormat() const override { return format; }

  GLenum getType() const override { return type; }

  bool isCompressed() const override { return compressed; }

 private:
  int width;
  int height;
  int levelCount;
  GLenum internalFormat;
  GLenum format;
  GLenum type;
  std::vector<uint8_t> data;
  bool compressed;
};

/**
 * @brief glTF图像类
 */
class GltfImage: public GltfObject {
 public:

  /**
   * @brief 构造函数
   */
  GltfImage(const std::string &uri = "",
            GLenum type = GL_TEXTURE_2D,
            int mipLevel = 0,
            std::optional<int> bufferView = std::nullopt,
            const std::string &name = "",
            ImageMimeType mimeType = ImageMimeType::UNKNOWN,
            GLenum texture = 0);

  /**
   * @brief 析构函数
   */
  ~GltfImage() override = default;

  /**
   * @brief 解析相对路径
   * @param basePath 基础路径
   */
  void resolveRelativePath(const std::string &basePath);

  /**
   * @brief 加载图像数据
   * @param gltf glTF根对象
   * @param additionalFiles 额外文件列表（可选）
   * @return 加载成功返回true
   */
  std::future<bool> loadAsync(std::shared_ptr<Gltf> gltf,
                              const std::vector<std::pair<std::string,
                                                          std::vector<uint8_t>>> *additionalFiles = nullptr);

  /**
   * @brief 同步加载图像数据
   * @param gltf glTF根对象
   * @param additionalFiles 额外文件列表（可选）
   * @return 加载成功返回true
   */
  bool loadSync(std::shared_ptr<Gltf> gltf,
                const std::vector<std::pair<std::string,
                                            std::vector<uint8_t>>> *additionalFiles = nullptr);

  /**
   * @brief 从文件名设置MIME类型
   * @param filename 文件名
   */
  void setMimeTypeFromFilename(const std::string &filename);

  /**
   * @brief 从字节数组设置图像数据
   * @param gltf glTF根对象
   * @param data 字节数组
   * @return 设置成功返回true
   */
  std::future<bool> setImageFromBytesAsync(std::shared_ptr<Gltf> gltf,
                                           const std::vector<uint8_t> &data);

  /**
   * @brief 从Base64数据设置图像
   * @param gltf glTF根对象
   * @return 设置成功返回true
   */
  std::future<bool> setImageFromBase64Async(std::shared_ptr<Gltf> gltf);

  /**
   * @brief 从URI设置图像
   * @param gltf glTF根对象
   * @return 设置成功返回true
   */
  std::future<bool> setImageFromUriAsync(std::shared_ptr<Gltf> gltf);

  /**
   * @brief 从缓冲区视图设置图像
   * @param gltf glTF根对象
   * @return 设置成功返回true
   */
  std::future<bool> setImageFromBufferViewAsync(std::shared_ptr<Gltf> gltf);

  /**
   * @brief 从文件列表设置图像
   * @param gltf glTF根对象
   * @param files 文件列表
   * @return 设置成功返回true
   */
  std::future<bool> setImageFromFilesAsync(std::shared_ptr<Gltf> gltf,
                                           const std::vector<std::pair<std::string,
                                                                       std::vector<
                                                                           uint8_t>>> &files);


  // === Getter/Setter方法 ===
  const std::string &getUri() const { return uri; }

  void setUri(const std::string &uri) { this->uri = uri; }

  const std::optional<int> &getBufferView() const { return bufferView; }

  void setBufferView(int bufferView) { this->bufferView = bufferView; }

  void clearBufferView() { bufferView.reset(); }

  ImageMimeType getMimeType() const { return mimeType; }

  GLenum getTexture() const;

  void setMimeType(ImageMimeType mimeType) { this->mimeType = mimeType; }

  std::shared_ptr<ImageData> getImageData() const { return imageData; }

  void setImageData(std::shared_ptr<ImageData> data) { imageData = data; }

  const std::string &getName() const { return name; }

  void setName(const std::string &name) { this->name = name; }

  GLenum getType() const { return type; }

  void setType(GLenum type) { this->type = type; }

  int getMipLevel() const { return mipLevel; }

  void setMipLevel(int level) { mipLevel = level; }

  bool isLoaded() const { return imageData != nullptr; }

 private:
  /**
   * @brief 从字节数组同步设置图像数据
   * @param gltf glTF根对象
   * @param data 字节数组
   * @return 设置成功返回true
   */
  bool setImageFromBytes(std::shared_ptr<Gltf> gltf,
                         const std::vector<uint8_t> &data);

  /**
   * @brief 解码JPEG图像
   * @param data JPEG数据
   * @return 图像数据
   */
  std::shared_ptr<ImageData> decodeJpeg(const std::vector<uint8_t> &data);

  /**
   * @brief 解码PNG图像
   * @param data PNG数据
   * @return 图像数据
   */
  std::shared_ptr<ImageData> decodePng(const std::vector<uint8_t> &data);

  /**
   * @brief 解码WEBP图像
   * @param data WEBP数据
   * @return 图像数据
   */
  std::shared_ptr<ImageData> decodeWebp(const std::vector<uint8_t> &data);

  /**
   * @brief 从Base64字符串解码数据
   * @param base64 Base64字符串
   * @return 解码后的字节数组
   */
  std::vector<uint8_t> decodeBase64(const std::string &base64);

  /**
   * @brief 检查URI是否为Data URI
   * @param uri URI字符串
   * @return 如果是Data URI返回true
   */
  bool isDataUri(const std::string &uri) const;

  /**
   * @brief 从Data URI解析MIME类型
   * @param dataUri Data URI字符串
   * @return MIME类型
   */
  ImageMimeType parseDataUriMimeType(const std::string &dataUri) const;

  /**
   * @brief 获取文件扩展名
   * @param filename 文件名
   * @return 扩展名（小写，不含点号）
   */
  std::string getFileExtension(const std::string &filename) const;

  /**
   * @brief 读取文件内容
   * @param filepath 文件路径
   * @return 文件内容
   */
  std::future<std::vector<uint8_t>> readFileAsync(const std::string &filepath);

 private:
  std::string uri;                        ///< 图像URI
  std::optional<int> bufferView;          ///< 缓冲区视图索引
  ImageMimeType mimeType;                 ///< MIME类型
  std::shared_ptr<ImageData> imageData;   ///< 图像数据
  std::string name;                       ///< 图像名称
  GLenum type;                           ///< OpenGL纹理类型（非标准）
  GLenum texture;                           ///< OpenGL纹理类型（非标准）

  int mipLevel;                          ///< Mipmap级别（非标准）
};

} // namespace digitalhumans

#endif //LIGHTDIGITALHUMAN_GLTFIMAGE_H
