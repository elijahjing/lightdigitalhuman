//
// Created by vincentsyan on 2025/8/18.
//

#include "Engine.h"
#include "../gltfdata/gl_constants.h"
#include "../gltfdata/GltfRenderer.h"
#include <algorithm>
#include <memory>
#include <set>
#include "../gltfdata/Gltf.h"
#include "../gltfdata/GltfState.h"
#include "../gltfdata/GltfAnimation.h"
#include "../gltfdata/GltfOpenGLContext.h"
#include "../utils/LogUtils.h"
#include "../gltfdata/ibl_sampler.h"

namespace digitalhumans {

Engine::Engine() {
  renderer = std::make_shared<GltfRenderer>();
  state = std::make_shared<GltfState>();
  context = std::make_shared<GltfOpenGLContext>();
  state->getAnimationTimer().start();
}

void Engine::renderFrame(int width, int height) {
  renderer->init(state);
  animate(state);
  renderer->resize(width, height);
  renderer->clearFrame(state->getRenderingParameters().clearColor);
  if (state->getGltf() == nullptr) {
    return;
  }
  const auto &scenes = state->getGltf()->getScenes();
  if (state->getSceneIndex() >= static_cast<int>(scenes.size())) {
    return;
  }
  auto scene = scenes[state->getSceneIndex()];
  if (scene == nullptr) {
    return;
  }
  scene->applyTransformHierarchy(state->getGltf());
  renderer->drawScene(state, scene);
  if (!init) {
    state->getUserCamera()->fitViewToScene(state->getGltf(),
                                           state->getSceneIndex());
    init = true;
  }
}

void Engine::animate(std::shared_ptr<GltfState> state) {
  if (state->getGltf() == nullptr) {
    return;
  }

  const auto &animations = state->getGltf()->getAnimations();
  const auto &animationIndices = state->getAnimationIndices();

  if (animations.empty() || animationIndices.empty()) {
    return;
  }

  for (const auto &[index, animTime]: animationIndices) {
    if (index < 0 || index >= static_cast<int>(animations.size())) {
      animations[index]->advance(state, std::nullopt, -1, index);
    }
  }

  float currentTime = state->getAnimationTimer().elapsedSec();
  for (const auto &[index, animTime]: animationIndices) {
    if (index >= 0 && index < static_cast<int>(animations.size())) {
      animations[index]->advance(state, currentTime, animTime, index);
    }
  }
}

void Engine::setUserCamera(std::shared_ptr<UserCamera> userCamera) const {
  if (state != nullptr) {
    state->setUserCamera(userCamera);
  }
}

std::vector<std::string> Engine::getAnimationAllName() const {
  const auto &animations = state->getGltf()->getAnimations();
  std::vector<std::string> allAnimations;
  allAnimations.reserve(animations.size());
  for (const auto &animation: animations) {
    allAnimations.push_back(animation->getName());
  }
  return allAnimations;
}

void Engine::playAnimation(std::string name, int time) const {

  if (!state) {
    LOGE("Engine状态为空");
    return;
  }

  auto gltf = state->getGltf();
  if (!gltf) {
    LOGE("GLTF对象为空");
    return;
  }
  const auto &animations = state->getGltf()->getAnimations();

  for (int i = 0; i < animations.size(); ++i) {
    auto animation = animations[i];
    if (animation->getName() == name) {
      AnimationEntry info(i, time);
      std::vector<AnimationEntry> animationList = {info};
      state->setAnimationIndices(animationList);
    }
  }
}

void Engine::stopAnimation(const std::string& name) const {
  if (name.empty()) {
    LOGE("动画名称为空");
    return;
  }

  if (!state) {
    LOGE("Engine状态为空");
    return;
  }

  auto gltf = state->getGltf();
  if (!gltf) {
    LOGE("GLTF对象为空");
    return;
  }
  const auto &animations = state->getGltf()->getAnimations();

  for (int i = 0; i < animations.size(); ++i) {
    auto animation = animations[i];
    if (animation->getName() == name) {
      state->removeAnimationIndex(i);
    }
  }
}

const std::shared_ptr<GltfState> &Engine::getState() const {
  return state;
}

void Engine::setState(const std::shared_ptr<GltfState> &state) {
  Engine::state = state;
}

void Engine::setIbL(bool use) const {
  state->getRenderingParameters().useIBL = use;
}

bool Engine::processEnvironmentMap(const HDRImage &hdrImage) const {

  auto startTime = std::chrono::high_resolution_clock::now();

  try {
    if (hdrImage.dataFloat.empty()) {
      LOGE("Failed to load HDR image");
      return false;
    }

    auto iblSampler = std::make_shared<IBLSampler>(context);
    if (!iblSampler->init(hdrImage)) {
      LOGE("Failed to initialize IBL sampler");
      return false;
    }
    iblSampler->filterAll();
    GLuint diffuseTexture = iblSampler->getLambertianTextureID();
    GLuint specularTexture = iblSampler->getGGXTextureID();
    GLuint sheenTexture = iblSampler->getSheenTextureID();
    GLuint ggxLutTexture = iblSampler->getGGXLutTextureID();
    GLuint charlieLutTexture = iblSampler->getCharlieLutTextureID();

    if (diffuseTexture == 0 || specularTexture == 0 || ggxLutTexture == 0) {
      LOGE("Failed to generate IBL textures");
      return false;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();

    auto env = getState()->getEnvironment();
    env->diffuseEnvMap =
        env->createImageInfo(diffuseTexture, GL_TEXTURE_CUBE_MAP, 1);
    env->specularEnvMap =
        env->createImageInfo(specularTexture, GL_TEXTURE_CUBE_MAP, 0);
    env->sheenEnvMap =
        env->createImageInfo(sheenTexture, GL_TEXTURE_CUBE_MAP, 0);
    env->lut = env->createImageInfo(ggxLutTexture, GL_TEXTURE_2D, 1);
    env->sheenLUT = env->createImageInfo(charlieLutTexture, GL_TEXTURE_2D, 1);
    env->mipCount_ = iblSampler->getMipmapLevels();
    env->diffuseEnvMap_ = diffuseTexture;
    return true;

  } catch (const std::exception &e) {
    LOGE("Exception during IBL processing: %s", e.what());
    return false;
  }
  return false;
}
} // namespace digitalhumans