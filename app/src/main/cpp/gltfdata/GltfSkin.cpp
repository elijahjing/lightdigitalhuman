//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfSkin.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/matrix_inverse.hpp"
#include <cmath>
#include <algorithm>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <string>
#include <utility>
#include <vector>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "tiny_gltf.h"
#include "../utils/LogUtils.h"
#include "GltfOpenGLContext.h"
#include "Gltf.h"
#include "GltfNode.h"
#include "GltfAccessor.h"
#include "ImageMimeTypes.h"
#include "GltfSampler.h"

#include "GltfImage.h"

namespace digitalhumans {

GltfSkin::GltfSkin()
    : GltfObject(), name(""), inverseBindMatrices(std::nullopt), joints(),
      skeleton(std::nullopt),
      jointTextureInfo(nullptr), jointMatrices(), jointNormalMatrices(),
      webglResourcesInitialized(false) {
}


void GltfSkin::initGl(std::shared_ptr<Gltf> gltf,
                      std::shared_ptr<GltfOpenGLContext> openGlContext) {
  if (webglResourcesInitialized) {
    return;
  }

  // 创建关节纹理
  jointWebGlTexture = openGlContext->createTexture();
  openGlContext->bindTexture(GL_TEXTURE_2D, jointWebGlTexture);

  // 确保禁用mipmap并正确配置采样器
  openGlContext->texParameteri(GL_TEXTURE_2D,
                               GL_TEXTURE_WRAP_S,
                               GL_CLAMP_TO_EDGE);
  openGlContext->texParameteri(GL_TEXTURE_2D,
                               GL_TEXTURE_WRAP_T,
                               GL_CLAMP_TO_EDGE);
  openGlContext->texParameteri(GL_TEXTURE_2D,
                               GL_TEXTURE_WRAP_R,
                               GL_CLAMP_TO_EDGE);
  openGlContext->texParameteri(GL_TEXTURE_2D,
                               GL_TEXTURE_MIN_FILTER,
                               GL_NEAREST);
  openGlContext->texParameteri(GL_TEXTURE_2D,
                               GL_TEXTURE_MAG_FILTER,
                               GL_NEAREST);

  createJointTextureResources(std::move(gltf));
  webglResourcesInitialized = true;
}

void GltfSkin::createJointTextureResources(std::shared_ptr<Gltf> gltf) {
  // 创建关节图像资源
  auto jointsImage = std::make_shared<GltfImage>(
      "",
      GL_TEXTURE_2D,
      0,
      std::nullopt,
      "",
      ImageMimeType::GLTEXTURE,
      jointWebGlTexture
  );
  gltf->addImage(jointsImage);
  // 创建采样器
  auto jointSampler = std::make_shared<GltfSampler>(
      GL_NEAREST,
      GL_NEAREST,
      GL_CLAMP_TO_EDGE,
      GL_CLAMP_TO_EDGE
  );
  gltf->addSampler(jointSampler);

  // 创建纹理
  auto jointsTexture = std::make_shared<GltfTexture>(
      static_cast<int>(gltf->getSamplers().size() - 1),
      static_cast<int>(gltf->getImages().size() - 1),
      GL_TEXTURE_2D
  );

  jointsTexture->setInitialized(true);
  gltf->addTexture(jointsTexture);

  // 创建纹理信息
  jointTextureInfo = std::make_shared<GltfTextureInfo>(
      static_cast<int>(gltf->getTextures().size() - 1),
      0,
      true
  );
  jointTextureInfo->setSamplerName("u_jointsSampler");
  jointTextureInfo->setGenerateMips(false);
}

void GltfSkin::computeJoints(std::shared_ptr<Gltf> gltf,
                             std::shared_ptr<GltfOpenGLContext> openGlContext) {
  if (!gltf || joints.empty()) {
    return;
  }

  // 获取逆绑定矩阵访问器
  std::shared_ptr<GltfAccessor> ibmAccessor = nullptr;
  if (inverseBindMatrices.has_value()) {
    const int ibmIndex = inverseBindMatrices.value();
    if (ibmIndex >= 0
        && ibmIndex < static_cast<int>(gltf->getAccessors().size())) {
      ibmAccessor = gltf->getAccessors()[ibmIndex];
    }
  }

  jointMatrices.clear();
  jointNormalMatrices.clear();
  jointMatrices.reserve(joints.size());
  jointNormalMatrices.reserve(joints.size());

  const int width = static_cast<int>(std::ceil(std::sqrt(joints.size() * 8)));
  std::vector<float> textureData(width * width * 4, 0.0f);
  int jointIndex = 0;
  for (const int joint: joints) {
    if (joint < 0 || joint >= static_cast<int>(gltf->getNodes().size())) {
      continue;
    }

    const auto &node = gltf->getNodes()[joint];
    if (!node) {
      continue;
    }
    glm::mat4 jointMatrix = node->getWorldTransform();

    if (ibmAccessor && jointIndex < ibmAccessor->getCount()) {
      try {
        const auto ibmData = ibmAccessor->getDeinterlacedView(*gltf);
        if (ibmData.second >= (jointIndex + 1) * 16 * sizeof(float)) {
          const auto *floatData = static_cast<const float *>(ibmData.first);
          glm::mat4 ibm;
          std::memcpy(glm::value_ptr(ibm),
                      floatData + jointIndex * 16,
                      16 * sizeof(float));
          jointMatrix = jointMatrix * ibm;
        }
      } catch (const std::exception &e) {
        LOGE("Error applying inverse bind matrix: %s", e.what());
      }
    }

    glm::mat4 normalMatrix = glm::transpose(glm::inverse(jointMatrix));

    // 存储到向量中
    jointMatrices.push_back(jointMatrix);
    jointNormalMatrices.push_back(normalMatrix);


    const int baseOffset = jointIndex * 32;

    // 复制关节矩阵（16个float）
    if (baseOffset + 15 < textureData.size()) {
      const float *jointMatrixData = glm::value_ptr(jointMatrix);
      std::memcpy(textureData.data() + baseOffset,
                  jointMatrixData,
                  16 * sizeof(float));
    }

    // 复制法线矩阵（16个float）
    if (baseOffset + 16 + 15 < textureData.size()) {
      const float *normalMatrixData = glm::value_ptr(normalMatrix);
      std::memcpy(textureData.data() + baseOffset + 16,
                  normalMatrixData,
                  16 * sizeof(float));
    }
    ++jointIndex;
  }

  // 上传纹理数据
  openGlContext->bindTexture(GL_TEXTURE_2D, jointWebGlTexture);
  openGlContext->texImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA32F,
      width,
      width,
      0,
      GL_RGBA,
      GL_FLOAT,
      textureData.data()
  );

}

glm::mat4
GltfSkin::simulateShaderMatrixRead(const std::vector<float> &textureData,
                                   int width, int shaderIndex) {
  // 模拟着色器的 getMatrixFromTexture 函数
  glm::mat4 result(1.0f);
  int pixelIndex = shaderIndex * 4;

  for (int i = 0; i < 4; ++i) {
    int currentPixel = pixelIndex + i;
    int x = currentPixel % width;
    int y = currentPixel / width;
    int dataIndex = (y * width + x) * 4;

    if (dataIndex + 3 < textureData.size()) {
      // result[i] = texelFetch(...)
      result[i][0] = textureData[dataIndex + 0];
      result[i][1] = textureData[dataIndex + 1];
      result[i][2] = textureData[dataIndex + 2];
      result[i][3] = textureData[dataIndex + 3];
    }
  }

  return result;
}


void GltfSkin::addJoint(int jointIndex) {
  if (std::find(joints.begin(), joints.end(), jointIndex) == joints.end()) {
    joints.push_back(jointIndex);
  }
}


GLenum GltfSkin::getJointWebGlTexture() const {
  return jointWebGlTexture;
}

} // namespace digitalhumans