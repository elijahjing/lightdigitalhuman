//
// Created by vincentsyan on 2025/8/18.
//
#include "GltfTexture.h"
#include "../utils/utils.h"
#include <android/log.h>
#include <stdexcept>

#include "../utils/LogUtils.h"

#include "Gltf.h"

namespace digitalhumans {

// === KHRTextureTransform实现 ===
KHRTextureTransform::KHRTextureTransform()
    : GltfObject(), offset({0.0f, 0.0f}), scale({1.0f, 1.0f}), rotation(0.0f) {
}

// === GltfTextureInfo实现 ===
GltfTextureInfo::GltfTextureInfo(std::optional<int> index,
                                 int texCoord,
                                 bool linear,
                                 const std::string &samplerName,
                                 bool generateMips)
    : GltfObject(), index(index), texCoord(texCoord), linear(linear),
      samplerName(samplerName), strength(1.0f), scale(1.0f),
      generateMips(generateMips), extensions() {
}


void GltfTextureInfo::initGl(std::shared_ptr<Gltf> gltf,
                             std::shared_ptr<GltfOpenGLContext> openGlContext) {
  if (!linear && index.has_value()) {
    const int textureIndex = index.value();
    if (textureIndex >= 0
        && textureIndex < static_cast<int>(gltf->getTextures().size())) {
      auto texture = gltf->getTextures()[textureIndex];
      if (texture) {
        texture->setLinear(false);
      }
    }
  }

  utils::initGlForMembers(*this, gltf, openGlContext);
}

std::shared_ptr<KHRTextureTransform>
GltfTextureInfo::getTextureTransform() const {
  auto it = extensions.find("KHR_texture_transform");
  if (it != extensions.end()) {
    try {
      return std::any_cast<std::shared_ptr<KHRTextureTransform>>(it->second);
    } catch (const std::bad_any_cast &) {
      return nullptr;
    }
  }
  return nullptr;
}

void
GltfTextureInfo::setTextureTransform(std::shared_ptr<KHRTextureTransform> transform) {
  if (transform) {
    extensions["KHR_texture_transform"] = transform;
  } else {
    extensions.erase("KHR_texture_transform");
  }
}


// === GltfTexture实现 ===

// 静态成员定义
GltfTexture::GltfTexture(std::optional<int> sampler,
                         std::optional<int> source,
                         GLenum type)
    : GltfObject(), sampler(sampler), source(source), glTexture(0), type(type),
      initialized(false), mipLevelCount(0), linear(true) {
}

GltfTexture::~GltfTexture() {
  destroy();
}


void GltfTexture::destroy() {
  if (glTexture != 0) {
    // 尝试获取WebGL上下文
    glDeleteTextures(1, &glTexture);
    glTexture = 0;
  }
  initialized = false;
}


} // namespace digitalhumans