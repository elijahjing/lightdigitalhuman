//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfSampler.h"
// gltf_sampler.cpp
#include <stdexcept>
#include <sstream>

#ifdef __ANDROID__
#include <GLES3/gl3.h>
#else
#include <GL/gl.h>
#endif
#include "../utils/LogUtils.h"

namespace digitalhumans {

GltfSampler::GltfSampler(int magFilter, int minFilter, int wrapS, int wrapT)
    : GltfObject(), magFilter(magFilter), minFilter(minFilter), wrapS(wrapS),
      wrapT(wrapT), name(std::nullopt) {
  // 验证参数
  if (!validate()) {
    LOGW("GltfSampler created with invalid parameters, using defaults");
    reset();
  }
}

std::unique_ptr<GltfSampler> GltfSampler::createDefault() {
  return std::make_unique<GltfSampler>();
}

bool GltfSampler::validate() const {
  // 调用基类验证
  if (!GltfObject::validate()) {
    return false;
  }

  // 验证过滤器
  if (!validateFilters()) {
    return false;
  }

  // 验证包装模式
  if (!validateWrapModes()) {
    return false;
  }

  return true;
}


void GltfSampler::setMagFilter(int filter) {
  if (isValidMagFilter(filter)) {
    magFilter = filter;
  } else {
    LOGW("Invalid mag filter: %d, keeping current value", filter);
  }
}

void GltfSampler::setMinFilter(int filter) {
  if (isValidMinFilter(filter)) {
    minFilter = filter;
  } else {
    LOGW("Invalid min filter: %d, keeping current value", filter);
  }
}

void GltfSampler::setWrapS(int wrap) {
  if (isValidWrapMode(wrap)) {
    wrapS = wrap;
  } else {
    LOGW("Invalid wrap S mode: %d, keeping current value", wrap);
  }
}

void GltfSampler::setWrapT(int wrap) {
  if (isValidWrapMode(wrap)) {
    wrapT = wrap;
  } else {
    LOGW("Invalid wrap T mode: %d, keeping current value", wrap);
  }
}

void GltfSampler::setWrap(int wrapS, int wrapT) {
  setWrapS(wrapS);
  setWrapT(wrapT);
}

void GltfSampler::setFilter(int magFilter, int minFilter) {
  setMagFilter(magFilter);
  setMinFilter(minFilter);
}

bool GltfSampler::usesMipmap() const {
  return minFilter == GL_NEAREST_MIPMAP_NEAREST ||
      minFilter == GL_NEAREST_MIPMAP_LINEAR ||
      minFilter == GL_LINEAR_MIPMAP_NEAREST ||
      minFilter == GL_LINEAR_MIPMAP_LINEAR;
}

void GltfSampler::getGLParameters(int &outMagFilter, int &outMinFilter,
                                  int &outWrapS, int &outWrapT) const {
  outMagFilter = magFilter;
  outMinFilter = minFilter;
  outWrapS = wrapS;
  outWrapT = wrapT;
}

void GltfSampler::applyToTexture(int target) const {
  glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
  glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
  glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS);
  glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT);
}


void GltfSampler::reset() {
  magFilter = GL_LINEAR;
  minFilter = GL_LINEAR_MIPMAP_LINEAR;
  wrapS = GL_REPEAT;
  wrapT = GL_REPEAT;
  name = std::nullopt;
}

std::string GltfSampler::toString() const {
  std::ostringstream oss;
  oss << "GltfSampler{";
  oss << "magFilter:" << magFilter;
  oss << ", minFilter:" << minFilter;
  oss << ", wrapS:" << wrapS;
  oss << ", wrapT:" << wrapT;

  if (name.has_value()) {
    oss << ", name:\"" << name.value() << "\"";
  }

  std::string baseInfo = GltfObject::toString();
  if (!baseInfo.empty()) {
    oss << ", " << baseInfo;
  }

  oss << "}";
  return oss.str();
}

// 静态验证函数
bool GltfSampler::isValidMagFilter(int filter) {
  return filter == GL_NEAREST || filter == GL_LINEAR;
}

bool GltfSampler::isValidMinFilter(int filter) {
  return filter == GL_NEAREST ||
      filter == GL_LINEAR ||
      filter == GL_NEAREST_MIPMAP_NEAREST ||
      filter == GL_NEAREST_MIPMAP_LINEAR ||
      filter == GL_LINEAR_MIPMAP_NEAREST ||
      filter == GL_LINEAR_MIPMAP_LINEAR;
}

bool GltfSampler::isValidWrapMode(int wrap) {
  return wrap == GL_CLAMP_TO_EDGE ||
      wrap == GL_MIRRORED_REPEAT ||
      wrap == GL_REPEAT;
}

// 预设创建函数
std::unique_ptr<GltfSampler> GltfSampler::createNearest() {
  return std::make_unique<GltfSampler>(GL_NEAREST,
                                       GL_NEAREST,
                                       GL_REPEAT,
                                       GL_REPEAT);
}

std::unique_ptr<GltfSampler> GltfSampler::createLinear() {
  return std::make_unique<GltfSampler>(GL_LINEAR,
                                       GL_LINEAR,
                                       GL_REPEAT,
                                       GL_REPEAT);
}

std::unique_ptr<GltfSampler> GltfSampler::createMipmap() {
  return std::make_unique<GltfSampler>(GL_LINEAR,
                                       GL_LINEAR_MIPMAP_LINEAR,
                                       GL_REPEAT,
                                       GL_REPEAT);
}

std::unique_ptr<GltfSampler> GltfSampler::createClamp() {
  return std::make_unique<GltfSampler>(GL_LINEAR,
                                       GL_LINEAR,
                                       GL_CLAMP_TO_EDGE,
                                       GL_CLAMP_TO_EDGE);
}

// 私有方法实现
bool GltfSampler::validateFilters() const {
  if (!isValidMagFilter(magFilter)) {
    LOGE("Invalid magFilter: %d", magFilter);
    return false;
  }

  if (!isValidMinFilter(minFilter)) {
    LOGE("Invalid minFilter: %d", minFilter);
    return false;
  }

  return true;
}

bool GltfSampler::validateWrapModes() const {
  if (!isValidWrapMode(wrapS)) {
    LOGE("Invalid wrapS: %d", wrapS);
    return false;
  }

  if (!isValidWrapMode(wrapT)) {
    LOGE("Invalid wrapT: %d", wrapT);
    return false;
  }

  return true;
}

} // namespace digitalhumans