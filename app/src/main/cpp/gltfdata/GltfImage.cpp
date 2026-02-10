//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfImage.h"

#include "../utils/utils.h"
#include <android/log.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>
#include <thread>
#include "ImageMimeTypes.h"
#include "GltfBufferView.h"
#include "GltfBuffer.h"
#include "ImageMimeTypes.h"


// 第三方图像解码库（需要添加到项目中）
#ifdef HAVE_LIBJPEG
#include "jpeglib.h"
#endif

#ifdef HAVE_LIBPNG
#include "png.h"
#endif

#ifdef HAVE_LIBWEBP
#include "webp/decode.h"
#endif

#include "../utils/LogUtils.h"
#include "Gltf.h"


namespace digitalhumans {

// ===== BasicImageData实现 =====

BasicImageData::BasicImageData(int width,
                               int height,
                               int channels,
                               std::vector<uint8_t> data)
    : width(width), height(height), channels(channels), data(std::move(data)) {
}

// ===== KtxImageData实现 =====

KtxImageData::KtxImageData(int width,
                           int height,
                           int levelCount,
                           GLenum internalFormat,
                           GLenum format,
                           GLenum type,
                           std::vector<uint8_t> data,
                           bool compressed)
    : width(width), height(height), levelCount(levelCount),
      internalFormat(internalFormat), format(format),
      type(type), data(std::move(data)), compressed(compressed) {
}

// ===== GltfImage实现 =====

void GltfImage::resolveRelativePath(const std::string &basePath) {
  if (uri.empty() || isDataUri(uri)) {
    return;
  }

  // 处理相对路径
  if (uri.substr(0, 2) == "./") {
    uri = uri.substr(2);
  }

  // 拼接基础路径
  if (!basePath.empty() && basePath.back() != '/') {
    uri = basePath + "/" + uri;
  } else {
    uri = basePath + uri;
  }
}

std::future<bool> GltfImage::loadAsync(std::shared_ptr<Gltf> gltf,
                                       const std::vector<std::pair<std::string,
                                                                   std::vector<
                                                                       uint8_t>>> *additionalFiles) {
  return std::async(std::launch::async, [this, gltf, additionalFiles]() {
    return loadSync(gltf, additionalFiles);
  });
}

bool GltfImage::loadSync(std::shared_ptr<Gltf> gltf,
                         const std::vector<std::pair<std::string,
                                                     std::vector<uint8_t>>> *additionalFiles) {
  if (imageData != nullptr) {
    if (mimeType != ImageMimeType::GLTEXTURE) {
      LOGE("Image has already been loaded");
    }
    return true;
  }

  // 尝试不同的加载方式
  if (setImageFromBufferViewAsync(gltf).get() ||
      (additionalFiles && setImageFromFilesAsync(gltf, *additionalFiles).get())
      ||
          setImageFromUriAsync(gltf).get() ||
      setImageFromBase64Async(gltf).get()) {
    return true;
  }

  LOGE("Failed to load image: %s", name.c_str());
  return false;
}


void GltfImage::setMimeTypeFromFilename(const std::string &filename) {
  std::string extension = getFileExtension(filename);

  if (extension == "ktx2" || extension == "ktx") {
    mimeType = ImageMimeType::KTX2;
  } else if (extension == "jpg" || extension == "jpeg") {
    mimeType = ImageMimeType::JPEG;
  } else if (extension == "png") {
    mimeType = ImageMimeType::PNG;
  } else if (extension == "webp") {
    mimeType = ImageMimeType::WEBP;
  } else if (extension == "hdr") {
    mimeType = ImageMimeType::HDR;
  } else {
    LOGW("MIME type not defined for extension: %s", extension.c_str());
    // 假设为JPEG编码作为最佳猜测
    mimeType = ImageMimeType::JPEG;
  }
}

std::future<bool> GltfImage::setImageFromBytesAsync(std::shared_ptr<Gltf> gltf,
                                                    const std::vector<uint8_t> &data) {
  return std::async(std::launch::async, [this, gltf, data]() {
    return setImageFromBytes(gltf, data);
  });
}

bool GltfImage::setImageFromBytes(std::shared_ptr<Gltf> gltf,
                                  const std::vector<uint8_t> &data) {
  if (data.empty()) {
    LOGE("Empty image data");
    return false;
  }

  try {
    if (mimeType == ImageMimeType::KTX2) {
      // 处理KTX2格式
      //TODO 不支持KTX2
//                auto ktxDecoder = gltf->getKtxDecoder();
//                if (ktxDecoder) {
//                    imageData = ktxDecoder->decodeFromBuffer(data);
//                    return imageData != nullptr;
//                } else {
//                    LOGW("Loading of KTX images failed: KtxDecoder not initialized");
//                    return false;
//                }
      return false;
    } else if (mimeType == ImageMimeType::JPEG) {
      imageData = decodeJpeg(data);
      return imageData != nullptr;
    } else if (mimeType == ImageMimeType::PNG) {
      imageData = decodePng(data);
      return imageData != nullptr;
    } else if (mimeType == ImageMimeType::WEBP) {
      imageData = decodeWebp(data);
      return imageData != nullptr;
    } else {
      LOGE("Unsupported image type: %d", static_cast<int>(mimeType));
      return false;
    }
  } catch (const std::exception &e) {
    LOGE("Error decoding image: %s", e.what());
    return false;
  }
}

std::future<bool>
GltfImage::setImageFromBase64Async(std::shared_ptr<Gltf> gltf) {
  return std::async(std::launch::async, [this, gltf]() {
    if (uri.empty() || !isDataUri(uri)) {
      return false;
    }

    // 解析Data URI
    size_t commaPos = uri.find(',');
    if (commaPos == std::string::npos) {
      LOGE("Invalid data URI format");
      return false;
    }

    std::string header = uri.substr(0, commaPos);
    std::string base64Data = uri.substr(commaPos + 1);

    // 如果MIME类型未定义，从Data URI头部解析
    if (mimeType == ImageMimeType::UNKNOWN) {
      mimeType = parseDataUriMimeType(header);
      if (mimeType == ImageMimeType::UNKNOWN) {
        LOGW("Unsupported data URI: %s", header.c_str());
        return false;
      }
    }

    // 解码Base64数据
    std::vector<uint8_t> decodedData = decodeBase64(base64Data);
    if (decodedData.empty()) {
      LOGE("Failed to decode base64 data");
      return false;
    }

    return setImageFromBytes(gltf, decodedData);
  });
}

std::future<bool> GltfImage::setImageFromUriAsync(std::shared_ptr<Gltf> gltf) {
  return std::async(std::launch::async, [this, gltf]() {
    if (uri.empty() || isDataUri(uri)) {
      return false;
    }

    // 如果MIME类型未定义，从文件名推断
    if (mimeType == ImageMimeType::UNKNOWN) {
      setMimeTypeFromFilename(uri);
    }

    try {
      if (mimeType == ImageMimeType::KTX2) {
//                    auto ktxDecoder = gltf->getKtxDecoder();
//                    if (ktxDecoder) {
//                        imageData = ktxDecoder->loadFromUri(uri);
//                        return imageData != nullptr;
//                    } else {
//                        LOGW("Loading of KTX images failed: KtxDecoder not initialized");
//                        return false;
//                    }
        return false;
      } else {
        // 读取文件
        auto fileData = readFileAsync(uri).get();
        if (fileData.empty()) {
          LOGE("Could not read file: %s", uri.c_str());
          return false;
        }
        return setImageFromBytes(gltf, fileData);
      }
    } catch (const std::exception &e) {
      LOGE("Error loading image from URI %s: %s", uri.c_str(), e.what());
      return false;
    }
  });
}

std::future<bool>
GltfImage::setImageFromBufferViewAsync(std::shared_ptr<Gltf> gltf) {
  return std::async(std::launch::async, [this, gltf]() {
    if (!bufferView.has_value()) {
      return false;
    }

    const auto &bufferViews = gltf->getBufferViews();
    int viewIndex = bufferView.value();

    if (viewIndex < 0 || viewIndex >= static_cast<int>(bufferViews.size())) {
      LOGE("Invalid buffer view index: %d", viewIndex);
      return false;
    }

    auto view = bufferViews[viewIndex];
    if (!view) {
      LOGE("Buffer view is null");
      return false;
    }

    const auto &buffers = gltf->getBuffers();
    int bufferIndex = view->getBuffer().value();

    if (bufferIndex < 0 || bufferIndex >= static_cast<int>(buffers.size())) {
      LOGE("Invalid buffer index: %d", bufferIndex);
      return false;
    }

    auto buffer = buffers[bufferIndex];
    if (!buffer || buffer->buffer.empty()) {
      LOGE("Buffer is null or has no data");
      return false;
    }

    const uint8_t
        *bufferData = static_cast<const uint8_t *>(buffer->buffer.data());
    size_t byteOffset = view->getByteOffset();
    size_t byteLength = view->getByteLength().value();

    // 复制数据到vector
    std::vector<uint8_t> imageBytes
        (bufferData + byteOffset, bufferData + byteOffset + byteLength);

    return setImageFromBytes(gltf, imageBytes);
  });
}

std::future<bool> GltfImage::setImageFromFilesAsync(std::shared_ptr<Gltf> gltf,
                                                    const std::vector<std::pair<
                                                        std::string,
                                                        std::vector<uint8_t>>> &files) {
  return std::async(std::launch::async, [this, gltf, &files]() {
    if (uri.empty()) {
      return false;
    }

    // 查找匹配的文件
    std::string targetUri = "/" + uri;
    auto foundFile = std::find_if(files.begin(), files.end(),
                                  [&targetUri](const std::pair<std::string,
                                                               std::vector<
                                                                   uint8_t>> &file) {
                                    return file.first == targetUri;
                                  });

    if (foundFile == files.end()) {
      return false;
    }

    // 如果MIME类型未定义，从文件名推断
    if (mimeType == ImageMimeType::UNKNOWN) {
      setMimeTypeFromFilename(foundFile->first);
    }

    try {
      if (mimeType == ImageMimeType::KTX2) {
//                    auto ktxDecoder = gltf->getKtxDecoder();
//                    if (ktxDecoder) {
//                        imageData = ktxDecoder->decodeFromBuffer(foundFile->second);
//                        return imageData != nullptr;
//                    } else {
//                        LOGW("Loading of KTX images failed: KtxDecoder not initialized");
//                        return false;
//                    }
        return false;
      } else {
        return setImageFromBytes(gltf, foundFile->second);
      }
    } catch (const std::exception &e) {
      LOGE("Error reading image from file %s: %s", uri.c_str(), e.what());
      return false;
    }
  });
}

std::shared_ptr<ImageData>
GltfImage::decodeJpeg(const std::vector<uint8_t> &data) {
#ifdef HAVE_LIBJPEG
  // 使用libjpeg解码JPEG
struct jpeg_decompress_struct cinfo;
struct jpeg_error_mgr jerr;

cinfo.err = jpeg_std_error(&jerr);
jpeg_create_decompress(&cinfo);

jpeg_mem_src(&cinfo, data.data(), data.size());
jpeg_read_header(&cinfo, TRUE);
jpeg_start_decompress(&cinfo);

int width = cinfo.output_width;
int height = cinfo.output_height;
int channels = cinfo.output_components;

std::vector<uint8_t> imageData(width * height * channels);

while (cinfo.output_scanline < cinfo.output_height) {
uint8_t* row = &imageData[cinfo.output_scanline * width * channels];
jpeg_read_scanlines(&cinfo, &row, 1);
}

jpeg_finish_decompress(&cinfo);
jpeg_destroy_decompress(&cinfo);

return std::make_shared<BasicImageData>(width, height, channels, std::move(imageData));
#else
  LOGE("JPEG decoding not supported - libjpeg not available");
  return nullptr;
#endif
}

std::shared_ptr<ImageData>
GltfImage::decodePng(const std::vector<uint8_t> &data) {
#ifdef HAVE_LIBPNG
  // 使用libpng解码PNG
png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
if (!png) {
return nullptr;
}

png_infop info = png_create_info_struct(png);
if (!info) {
png_destroy_read_struct(&png, NULL, NULL);
return nullptr;
}

if (setjmp(png_jmpbuf(png))) {
png_destroy_read_struct(&png, &info, NULL);
return nullptr;
}

// 设置内存输入
struct MemoryBuffer {
const uint8_t* data;
size_t size;
size_t offset;
} memBuffer = {data.data(), data.size(), 0};

png_set_read_fn(png, &memBuffer, [](png_structp png, png_bytep buffer, png_size_t size) {
MemoryBuffer* mem = static_cast<MemoryBuffer*>(png_get_io_ptr(png));
if (mem->offset + size > mem->size) {
  png_error(png, "Read past end of buffer");
}
memcpy(buffer, mem->data + mem->offset, size);
mem->offset += size;
});

png_read_info(png, info);

int width = png_get_image_width(png, info);
int height = png_get_image_height(png, info);
png_byte color_type = png_get_color_type(png, info);
png_byte bit_depth = png_get_bit_depth(png, info);

// 转换为RGBA格式
if (bit_depth == 16) {
png_set_strip_16(png);
}
if (color_type == PNG_COLOR_TYPE_PALETTE) {
png_set_palette_to_rgb(png);
}
if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
png_set_expand_gray_1_2_4_to_8(png);
}
if (png_get_valid(png, info, PNG_INFO_tRNS)) {
png_set_tRNS_to_alpha(png);
}
if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE) {
png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
}
if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
png_set_gray_to_rgb(png);
}

png_read_update_info(png, info);

std::vector<uint8_t> imageData(width * height * 4);
std::vector<png_bytep> rows(height);

for (int i = 0; i < height; i++) {
rows[i] = &imageData[i * width * 4];
}

png_read_image(png, rows.data());
png_destroy_read_struct(&png, &info, NULL);

return std::make_shared<BasicImageData>(width, height, 4, std::move(imageData));
#else
  LOGE("PNG decoding not supported - libpng not available");
  return nullptr;
#endif
}

std::shared_ptr<ImageData>
GltfImage::decodeWebp(const std::vector<uint8_t> &data) {
#ifdef HAVE_LIBWEBP
  int width, height;
uint8_t* decodedData = WebPDecodeRGBA(data.data(), data.size(), &width, &height);

if (!decodedData) {
LOGE("Failed to decode WebP image");
return nullptr;
}

std::vector<uint8_t> imageData(decodedData, decodedData + width * height * 4);
WebPFree(decodedData);

return std::make_shared<BasicImageData>(width, height, 4, std::move(imageData));
#else
  LOGE("WebP decoding not supported - libwebp not available");
  return nullptr;
#endif
}

std::vector<uint8_t> GltfImage::decodeBase64(const std::string &base64) {
  // Base64解码实现
  const std::string chars =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::vector<uint8_t> result;

  int in_len = base64.size();
  int i = 0;
  int in = 0;
  uint8_t char_array_4[4], char_array_3[3];

  while (in_len-- && (base64[in] != '=')
      && (isalnum(base64[in]) || (base64[in] == '+') || (base64[in] == '/'))) {
    char_array_4[i++] = base64[in];
    in++;
    if (i == 4) {
      for (i = 0; i < 4; i++) {
        char_array_4[i] = static_cast<uint8_t>(chars.find(char_array_4[i]));
      }

      char_array_3[0] =
          (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] =
          ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++) {
        result.push_back(char_array_3[i]);
      }
      i = 0;
    }
  }

  if (i) {
    for (int j = i; j < 4; j++) {
      char_array_4[j] = 0;
    }

    for (int j = 0; j < 4; j++) {
      char_array_4[j] = static_cast<uint8_t>(chars.find(char_array_4[j]));
    }

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] =
        ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (int j = 0; (j < i - 1); j++) {
      result.push_back(char_array_3[j]);
    }
  }

  return result;
}

bool GltfImage::isDataUri(const std::string &uri) const {
  return uri.substr(0, 5) == "data:";
}

ImageMimeType
GltfImage::parseDataUriMimeType(const std::string &dataUri) const {
  if (dataUri.find("data:image/jpeg;base64") == 0) {
    return ImageMimeType::JPEG;
  } else if (dataUri.find("data:image/png;base64") == 0) {
    return ImageMimeType::PNG;
  } else if (dataUri.find("data:image/webp;base64") == 0) {
    return ImageMimeType::WEBP;
  } else if (dataUri.find("data:image/ktx2;base64") == 0) {
    return ImageMimeType::KTX2;
  } else if (dataUri.find("data:image/vnd.radiance;base64") == 0) {
    return ImageMimeType::HDR;
  } else if (dataUri.find("data:image/texture;base64") == 0) {
    return ImageMimeType::GLTEXTURE;
  }
  return ImageMimeType::UNKNOWN;
}

std::string GltfImage::getFileExtension(const std::string &filename) const {
  size_t lastDotPos = filename.find_last_of('.');
  if (lastDotPos == std::string::npos) {
    return "";
  }

  std::string extension = filename.substr(lastDotPos + 1);
  std::transform(extension.begin(),
                 extension.end(),
                 extension.begin(),
                 ::tolower);
  return extension;
}

std::future<std::vector<uint8_t>>
GltfImage::readFileAsync(const std::string &filepath) {
  return std::async(std::launch::async, [filepath]() -> std::vector<uint8_t> {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      LOGE("Failed to open file: %s", filepath.c_str());
      return {};
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
      LOGE("Failed to read file: %s", filepath.c_str());
      return {};
    }

    return buffer;
  });
}


GLenum GltfImage::getTexture() const {
  return texture;
}

GltfImage::GltfImage(const std::string &uri,
                     GLenum type,
                     int mipLevel,
                     std::optional<int> bufferView,
                     const std::string &name,
                     ImageMimeType mimeType,
                     GLenum texture) :
    uri(uri), type(type), mipLevel(mipLevel), bufferView(bufferView),
    name(name), mimeType(mimeType), texture(texture) {
}

} // namespace digitalhumans