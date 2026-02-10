// GltfEnvironment.cpp
#include "GltfEnvironment.h"
#include "ktx.h"
#include <iostream>
#include <stdexcept>
#include <android/log.h>
#include <vector>          // ✅ 必需：std::vector
#include <GLES3/gl3.h>
#include "glm.hpp"
#include "../utils/LogUtils.h"
#include "ImageMimeTypes.h"
#include "GltfSampler.h"
#include "GltfImage.h"
#include "Gltf.h"
#include "GltfTexture.h"

#include <sstream>

namespace digitalhumans {


bool checkGLError(const std::string &operation) {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    LOGE("OpenGL error after %s: 0x%x", operation.c_str(), error);
    return false;
  }
  return true;
}

std::shared_ptr<GltfTextureInfo>
GltfEnvironment::createImageInfo(GLuint &textureId,
                                 GLenum type,
                                 int classType) {
  // 创建gltf纹理资源
  auto morphTargetImage = std::make_shared<GltfImage>(
      "",
      type,
      0,
      std::nullopt,
      "",
      ImageMimeType::GLTEXTURE,
      textureId
  );
  gltf->addImage(morphTargetImage);
  if (classType == 1) {
    auto sampler = std::make_shared<GltfSampler>(
        GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE
    );
    gltf->addSampler(sampler);
  } else {
    auto sampler = std::make_shared<GltfSampler>(
        GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE
    );
    gltf->addSampler(sampler);
  }

  auto texture = std::make_shared<GltfTexture>(
      static_cast<int>(gltf->getSamplers().size() - 1),
      static_cast<int>(gltf->getImages().size() - 1),
      type
  );
  texture->setInitialized(true);
  gltf->addTexture(texture);

  auto morphTargetTextureInfo = std::make_shared<GltfTextureInfo>(
      static_cast<int>(gltf->getTextures().size() - 1), 0, true
  );
  morphTargetTextureInfo->setGenerateMips(false);
  return morphTargetTextureInfo;
}


GltfEnvironment::GltfEnvironment() {

}

void GltfEnvironment::setGltf(const std::shared_ptr<Gltf> &gltf) {
  GltfEnvironment::gltf = gltf;
}

GltfEnvironment::~GltfEnvironment() {

}

} // namespace digitalhumans