//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfOpenGLContext.h"

#include <android/log.h>
#include <iostream>
#include <sstream>
#include <regex>
#include <stdexcept>

#include "../utils/LogUtils.h"
#include "GltfTexture.h"
#include "GltfSampler.h"
#include "Gltf.h"
#include "GltfImage.h"
#include "GltfAccessor.h"


namespace digitalhumans {

GltfOpenGLContext::GltfOpenGLContext()
    : supportsEXTTextureFilterAnisotropic(false), anisotropyParameter(0),
      maxAnisotropy(1.0f),
      initialized(false) {
}

bool GltfOpenGLContext::initialize() {
  if (initialized) {
    return true;
  }

  loadOpenGLExtensions();
  initialized = true;
  return true;
}

void GltfOpenGLContext::loadOpenGLExtensions() {
  // 检查各向异性过滤扩展
  const char
      *extensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
  if (extensions && strstr(extensions, "GL_EXT_texture_filter_anisotropic")) {
    supportsEXTTextureFilterAnisotropic = true;
    anisotropyParameter = GL_TEXTURE_MAX_ANISOTROPY_EXT;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    LOGI("Anisotropic filtering supported, max level: %.1f", maxAnisotropy);
  } else {
    LOGW("Anisotropic filtering is not supported");
    supportsEXTTextureFilterAnisotropic = false;
    maxAnisotropy = 1.0f;
  }
}

bool GltfOpenGLContext::setTexture(GLint uniformLocation,
                                   std::shared_ptr<Gltf> gltf,
                                   std::shared_ptr<GltfTextureInfo> textureInfo,
                                   GLint texSlot) {
  if (uniformLocation == -1 || !gltf || !textureInfo) {
    return false;
  }

  const int textureIndex = textureInfo->getIndex().value();
  if (textureIndex < 0
      || textureIndex >= static_cast<int>(gltf->getTextures().size())) {
    return false;
  }

  auto gltfTexture = gltf->getTextures()[textureIndex];
  if (!gltfTexture) {
    return false;
  }

  const int imageIndex = gltfTexture->getSource().value();
  if (imageIndex < 0
      || imageIndex >= static_cast<int>(gltf->getImages().size())) {
    return false;
  }

  auto image = gltf->getImages()[imageIndex];
  if (!image) {
    return false;
  }

  // 创建纹理对象（如果尚未创建）
  if (gltfTexture->getGLTexture() == 0) {
    const ImageMimeType mimeType = image->getMimeType();
    if (mimeType == ImageMimeType::KTX2
        || mimeType == ImageMimeType::GLTEXTURE) {
      // 这些图像资源直接由资源加载器加载到GPU资源
      gltfTexture->setGLTexture(image->getTexture());
    } else {
      // 其他图像将在稍后步骤中上传
      gltfTexture->setGLTexture(createTexture());
    }
  }

  // 激活纹理槽并绑定纹理
  glActiveTexture(GL_TEXTURE0 + texSlot);
  glBindTexture(gltfTexture->getType(), gltfTexture->getGLTexture());

  // 设置uniform
  glUniform1i(uniformLocation, texSlot);

  // 初始化纹理（如果尚未初始化）
  if (!gltfTexture->isInitialized()) {
    const int samplerIndex = gltfTexture->getSampler().value();
    if (samplerIndex < 0
        || samplerIndex >= static_cast<int>(gltf->getSamplers().size())) {
      LOGW("Sampler is undefined for texture: %d", textureIndex);
      return false;
    }

    auto gltfSampler = gltf->getSamplers()[samplerIndex];
    if (!gltfSampler) {
      return false;
    }

    // 上传图像数据
    const ImageMimeType mimeType = image->getMimeType();
    if (mimeType == ImageMimeType::PNG ||
        mimeType == ImageMimeType::JPEG ||
        mimeType == ImageMimeType::WEBP ||
        mimeType == ImageMimeType::HDR) {

      uploadImageToTexture(gltfTexture, image);
    }

    setSampler(gltfSampler,
               gltfTexture->getType(),
               textureInfo->shouldGenerateMips());

    if (textureInfo->shouldGenerateMips()) {
      GLenum minFilter = gltfSampler->getMinFilter();
      switch (minFilter) {
        case GL_NEAREST_MIPMAP_NEAREST:
        case GL_NEAREST_MIPMAP_LINEAR:
        case GL_LINEAR_MIPMAP_NEAREST:
        case GL_LINEAR_MIPMAP_LINEAR:
          glGenerateMipmap(gltfTexture->getType());
          break;
        default:
          break;
      }
    }

    gltfTexture->setInitialized(true);
  }

  return gltfTexture->isInitialized();
}

bool
GltfOpenGLContext::setIndices(std::shared_ptr<Gltf> gltf, int accessorIndex) {
  if (!gltf || accessorIndex < 0
      || accessorIndex >= static_cast<int>(gltf->getAccessors().size())) {
    return false;
  }

  auto gltfAccessor = gltf->getAccessors()[accessorIndex];
  if (!gltfAccessor) {
    return false;
  }

  if (gltfAccessor->getGLBuffer() == 0) {
    gltfAccessor->setGLBuffer(createBuffer());

    auto data = gltfAccessor->getTypedView(*gltf);
    if (data.second <= 0) {
      return false;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gltfAccessor->getGLBuffer());
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 data.second,
                 data.first,
                 GL_STATIC_DRAW);
  } else {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gltfAccessor->getGLBuffer());
  }

  return true;
}

bool GltfOpenGLContext::enableAttribute(std::shared_ptr<Gltf> gltf,
                                        GLint attributeLocation,
                                        std::shared_ptr<GltfAccessor> accessor) {
  if (attributeLocation == -1 || !accessor) {
    LOGW("Tried to access unknown attribute or null accessor");
    return false;
  }

  if (accessor->getGLBuffer() == 0) {
    accessor->setGLBuffer(createBuffer());
    auto data = accessor->getTypedView(*gltf);
    if (data.second <= 0) {
      return false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, accessor->getGLBuffer());
    glBufferData(GL_ARRAY_BUFFER, data.second, data.first, GL_STATIC_DRAW);
  } else {
    glBindBuffer(GL_ARRAY_BUFFER, accessor->getGLBuffer());
  }

  glVertexAttribPointer(
      attributeLocation,
      accessor->getComponentCount(),
      accessor->getComponentType().value(),
      accessor->isNormalized() ? GL_TRUE : GL_FALSE,
      accessor->getByteStride(*gltf),
      nullptr
  );
  glEnableVertexAttribArray(attributeLocation);

  return true;
}

GLuint GltfOpenGLContext::compileShader(const std::string &shaderIdentifier,
                                        bool isVertex,
                                        const std::string &shaderSource) {
  GLuint
      shader = glCreateShader(isVertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
  const char *source = shaderSource.c_str();
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  GLint compiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

  if (!compiled) {
    GLint infoLogLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

    std::string infoLog;
    if (infoLogLength > 0) {
      infoLog.resize(infoLogLength);
      glGetShaderInfoLog(shader, infoLogLength, nullptr, &infoLog[0]);
    }

    std::string
        errorInfo = parseShaderError(shaderIdentifier, shaderSource, infoLog);
    glDeleteShader(shader);

    throw std::runtime_error(
        "Could not compile shader '" + shaderIdentifier + "': " + errorInfo);
  }

  return shader;
}

GLuint
GltfOpenGLContext::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  GLint linked = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);

  if (!linked) {
    GLint infoLogLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

    std::string infoLog;
    if (infoLogLength > 0) {
      infoLog.resize(infoLogLength);
      glGetProgramInfoLog(program, infoLogLength, nullptr, &infoLog[0]);
    }

    glDeleteProgram(program);
    throw std::runtime_error("Could not link WebGL program: " + infoLog);
  }

  return program;
}

void GltfOpenGLContext::setSampler(std::shared_ptr<GltfSampler> samplerObj,
                                   GLenum textureType,
                                   bool generateMipmaps) {
  if (!samplerObj) {
    return;
  }

  if (generateMipmaps) {
    glTexParameteri(textureType, GL_TEXTURE_WRAP_S, samplerObj->getWrapS());
    glTexParameteri(textureType, GL_TEXTURE_WRAP_T, samplerObj->getWrapT());
  } else {
    glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  // 如果没有mip-map，强制使用非mip-map采样器
  GLenum minFilter = samplerObj->getMinFilter();
  if (!generateMipmaps && minFilter != GL_NEAREST && minFilter != GL_LINEAR) {
    if (minFilter == GL_NEAREST_MIPMAP_NEAREST
        || minFilter == GL_NEAREST_MIPMAP_LINEAR) {
      glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    } else {
      glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
  } else {
    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, minFilter);
  }

  glTexParameteri(textureType,
                  GL_TEXTURE_MAG_FILTER,
                  samplerObj->getMagFilter());

  // 设置各向异性过滤
  if (supportsEXTTextureFilterAnisotropic) {
    GLenum magFilter = samplerObj->getMagFilter();
    if (magFilter != GL_NEAREST &&
        (minFilter == GL_NEAREST_MIPMAP_LINEAR
            || minFilter == GL_LINEAR_MIPMAP_LINEAR)) {
      glTexParameterf(textureType, anisotropyParameter, maxAnisotropy);
    }
  }
}

GLenum GltfOpenGLContext::createTexture() {
  GLuint texture;
  glGenTextures(1, &texture);
  return texture;
}

GLuint GltfOpenGLContext::createBuffer() {
  GLuint buffer;
  glGenBuffers(1, &buffer);
  return buffer;
}

void GltfOpenGLContext::deleteTexture(GLuint texture) {
  if (texture != 0) {
    glDeleteTextures(1, &texture);
  }
}

void GltfOpenGLContext::deleteBuffer(GLuint buffer) {
  if (buffer != 0) {
    glDeleteBuffers(1, &buffer);
  }
}

std::string
GltfOpenGLContext::parseShaderError(const std::string &shaderIdentifier,
                                    const std::string &shaderSource,
                                    const std::string &infoLog) {
  std::string info = "";
  std::istringstream logStream(infoLog);
  std::string line;

  // 使用正则表达式匹配错误信息格式
  std::regex errorPattern(R"((WARNING|ERROR): ([0-9]*):([0-9]*):(.*))");
  std::smatch matches;

  while (std::getline(logStream, line)) {
    if (std::regex_match(line, matches, errorPattern) && matches.size() == 5) {
      int lineNumber = std::stoi(matches[3].str()) - 1;
      std::string errorType = matches[1].str();
      std::string errorMessage = matches[4].str();

      info += errorType + ": " + shaderIdentifier + "+includes:" +
          std::to_string(lineNumber) + ": " + errorMessage + "\n";

      // 显示错误周围的源代码
      std::istringstream sourceStream(shaderSource);
      std::vector<std::string> sourceLines;
      std::string sourceLine;
      while (std::getline(sourceStream, sourceLine)) {
        sourceLines.push_back(sourceLine);
      }

      for (int i = std::max(0, lineNumber - 2);
           i < std::min(static_cast<int>(sourceLines.size()), lineNumber + 3);
           i++) {
        if (lineNumber == i) {
          info += "-> ";
        }
        info += "\t" + sourceLines[i] + "\n";
      }
    } else {
      info += line + "\n";
    }
  }

  return info;
}

void
GltfOpenGLContext::uploadImageToTexture(std::shared_ptr<GltfTexture> gltfTexture,
                                        std::shared_ptr<GltfImage> image) {
  if (!gltfTexture || !image) {
    return;
  }

  // Android平台上，检查是否支持SRGB格式
  bool supportsSRGB = false;
  const char
      *extensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
  if (extensions && strstr(extensions, "GL_EXT_sRGB")) {
    supportsSRGB = true;
  }

  // 确定内部格式
  GLenum internalFormat = GL_RGBA;
  if (!gltfTexture->isLinear() && supportsSRGB) {
    internalFormat = GL_SRGB8_ALPHA8;
  }

  // 上传图像数据
  glTexImage2D(
      image->getType(),           // target
      image->getMipLevel(),       // level
      internalFormat,             // internal format
      image->getImageData()->getWidth(),          // width
      image->getImageData()->getHeight(),         // height
      0,                          // border
      GL_RGBA,                    // format
      GL_UNSIGNED_BYTE,           // type
      image->getImageData()->getData()      // data
  );

  // 检查OpenGL错误
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    LOGE("OpenGL error uploading texture: 0x%x", error);
  }
}

void GltfOpenGLContext::bindTexture(GLenum target, GLuint texture) {
  glBindTexture(target, texture);
  checkGLError("bindTexture");
}

void
GltfOpenGLContext::texParameteri(GLenum target, GLenum pname, GLint param) {
  glTexParameteri(target, pname, param);
  checkGLError("texParameteri");
}

void
GltfOpenGLContext::texParameterf(GLenum target, GLenum pname, GLfloat param) {
  glTexParameterf(target, pname, param);
  checkGLError("texParameterf");
}

// 错误检查辅助函数
void GltfOpenGLContext::checkGLError(const char *operation) {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    LOGE("OpenGL error after %s: 0x%x", operation, error);
  }
}

void
GltfOpenGLContext::texImage2D(GLenum target, GLint level, GLint internalformat,
                              GLsizei width, GLsizei height, GLint border,
                              GLenum format, GLenum type, const void *data) {
  glTexImage2D(target,
               level,
               internalformat,
               width,
               height,
               border,
               format,
               type,
               data);
  checkGLError("texImage2D");
}

// 重载版本 - float 数据
void
GltfOpenGLContext::texImage2D(GLenum target,
                              GLint level,
                              GLint internalformat,
                              GLsizei width,
                              GLsizei height,
                              GLint border,
                              GLenum format,
                              GLenum type,
                              const std::vector<float> &data) {
  const void *dataPtr = data.empty() ? nullptr : data.data();
  texImage2D(target,
             level,
             internalformat,
             width,
             height,
             border,
             format,
             type,
             dataPtr);
}

// 重载版本 - uint8_t 数据
void
GltfOpenGLContext::texImage2D(GLenum target,
                              GLint level,
                              GLint internalformat,
                              GLsizei width,
                              GLsizei height,
                              GLint border,
                              GLenum format,
                              GLenum type,
                              const std::vector<uint8_t> &data) {
  const void *dataPtr = data.empty() ? nullptr : data.data();
  texImage2D(target,
             level,
             internalformat,
             width,
             height,
             border,
             format,
             type,
             dataPtr);
}

} // namespace digitalhumans