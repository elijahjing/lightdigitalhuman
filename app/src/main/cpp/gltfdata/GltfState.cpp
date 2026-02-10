//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfState.h"
#include "../../engine/Engine.h"
#include <algorithm>
#include <unordered_map>

namespace digitalhumans {

GltfState::GltfState()
    : gltf(nullptr), environment(std::make_shared<GltfEnvironment>()),
      userCamera(std::make_shared<UserCamera>()), sceneIndex(0),
      cameraNodeIndex(std::nullopt), animationIndices(), animationTimer(),
      variant(std::nullopt), renderingParameters() {
}

void GltfState::addAnimationIndex(int animationIndex, int loopCount = -1) {
  // 检查是否已存在相同索引的动画
  auto it = std::find_if(animationIndices.begin(), animationIndices.end(),
                         [animationIndex](const AnimationEntry &info) {
                           return info.first == animationIndex;
                         });

  if (it == animationIndices.end()) {
    // 不存在，添加新动画
    animationIndices.emplace_back(animationIndex, loopCount);
  } else {
  }
}

void GltfState::removeAnimationIndex(int animationIndex) {
  auto it = std::remove_if(animationIndices.begin(), animationIndices.end(),
                           [animationIndex](const AnimationEntry &info) {
                             return info.first == animationIndex;
                           });

  if (it != animationIndices.end()) {
    size_t removedCount = std::distance(it, animationIndices.end());
    animationIndices.erase(it, animationIndices.end());
  } else {
  }
}

void GltfState::clearAnimationIndices() {
  animationIndices.clear();
}

bool GltfState::isAnimationActive(int animationIndex) const {
  // 使用std::find_if
  auto it = std::find_if(animationIndices.begin(), animationIndices.end(),
                         [animationIndex](const AnimationEntry &entry) {
                           return entry.first == animationIndex;
                         });

  return it != animationIndices.end();
}

std::string GltfState::toneMapToString(ToneMap toneMap) {
  static const std::unordered_map<ToneMap, std::string> toneMapStrings = {
      {ToneMap::KHR_PBR_NEUTRAL, "Khronos PBR Neutral"},
      {ToneMap::ACES_HILL_EXPOSURE_BOOST,
       "ACES Filmic Tone Mapping (Hill - Exposure Boost)"},
      {ToneMap::ACES_NARKOWICZ, "ACES Filmic Tone Mapping (Narkowicz)"},
      {ToneMap::ACES_HILL, "ACES Filmic Tone Mapping (Hill)"},
      {ToneMap::NONE, "None (Linear mapping, clamped at 1.0)"}
  };

  auto it = toneMapStrings.find(toneMap);
  return (it != toneMapStrings.end()) ? it->second : "Unknown";
}

ToneMap GltfState::stringToToneMap(const std::string &str) {
  static const std::unordered_map<std::string, ToneMap> stringToToneMap = {
      {"Khronos PBR Neutral", ToneMap::KHR_PBR_NEUTRAL},
      {"ACES Filmic Tone Mapping (Hill - Exposure Boost)",
       ToneMap::ACES_HILL_EXPOSURE_BOOST},
      {"ACES Filmic Tone Mapping (Narkowicz)", ToneMap::ACES_NARKOWICZ},
      {"ACES Filmic Tone Mapping (Hill)", ToneMap::ACES_HILL},
      {"None (Linear mapping, clamped at 1.0)", ToneMap::NONE}
  };

  auto it = stringToToneMap.find(str);
  return (it != stringToToneMap.end()) ? it->second : ToneMap::KHR_PBR_NEUTRAL;
}

std::string GltfState::debugOutputToString(DebugOutput debugOutput) {
  static const std::unordered_map<DebugOutput, std::string>
      debugOutputStrings = {
      {DebugOutput::NONE, "None"},
      {DebugOutput::UV_COORDS_0, "Texture Coordinates 0"},
      {DebugOutput::UV_COORDS_1, "Texture Coordinates 1"},
      {DebugOutput::NORMAL, "Normal Texture"},
      {DebugOutput::GEOMETRYNORMAL, "Geometry Normal"},
      {DebugOutput::TANGENT, "Geometry Tangent"},
      {DebugOutput::BITANGENT, "Geometry Bitangent"},
      {DebugOutput::WORLDSPACENORMAL, "Shading Normal"},
      {DebugOutput::ALPHA, "Alpha"},
      {DebugOutput::OCCLUSION, "Occlusion"},
      {DebugOutput::EMISSIVE, "Emissive"},
      {DebugOutput::BASECOLOR, "Base Color"},
      {DebugOutput::METALLIC, "Metallic"},
      {DebugOutput::ROUGHNESS, "Roughness"},
      {DebugOutput::CLEARCOAT_FACTOR, "ClearCoat Strength"},
      {DebugOutput::CLEARCOAT_ROUGHNESS, "ClearCoat Roughness"},
      {DebugOutput::CLEARCOAT_NORMAL, "ClearCoat Normal"},
      {DebugOutput::SHEEN_COLOR, "Sheen Color"},
      {DebugOutput::SHEEN_ROUGHNESS, "Sheen Roughness"},
      {DebugOutput::SPECULAR_FACTOR, "Specular Strength"},
      {DebugOutput::SPECULAR_COLOR, "Specular Color"},
      {DebugOutput::TRANSMISSION_FACTOR, "Transmission Strength"},
      {DebugOutput::VOLUME_THICKNESS, "Volume Thickness"},
      {DebugOutput::DIFFUSE_TRANSMISSION_FACTOR,
       "Diffuse Transmission Strength"},
      {DebugOutput::DIFFUSE_TRANSMISSION_COLOR_FACTOR,
       "Diffuse Transmission Color"},
      {DebugOutput::IRIDESCENCE_FACTOR, "Iridescence Strength"},
      {DebugOutput::IRIDESCENCE_THICKNESS, "Iridescence Thickness"},
      {DebugOutput::ANISOTROPIC_STRENGTH, "Anisotropic Strength"},
      {DebugOutput::ANISOTROPIC_DIRECTION, "Anisotropic Direction"}
  };

  auto it = debugOutputStrings.find(debugOutput);
  return (it != debugOutputStrings.end()) ? it->second : "Unknown";
}

DebugOutput GltfState::stringToDebugOutput(const std::string &str) {
  static const std::unordered_map<std::string, DebugOutput>
      stringToDebugOutput = {
      {"None", DebugOutput::NONE},
      {"Texture Coordinates 0", DebugOutput::UV_COORDS_0},
      {"Texture Coordinates 1", DebugOutput::UV_COORDS_1},
      {"Normal Texture", DebugOutput::NORMAL},
      {"Geometry Normal", DebugOutput::GEOMETRYNORMAL},
      {"Geometry Tangent", DebugOutput::TANGENT},
      {"Geometry Bitangent", DebugOutput::BITANGENT},
      {"Shading Normal", DebugOutput::WORLDSPACENORMAL},
      {"Alpha", DebugOutput::ALPHA},
      {"Occlusion", DebugOutput::OCCLUSION},
      {"Emissive", DebugOutput::EMISSIVE},
      {"Base Color", DebugOutput::BASECOLOR},
      {"Metallic", DebugOutput::METALLIC},
      {"Roughness", DebugOutput::ROUGHNESS},
      {"ClearCoat Strength", DebugOutput::CLEARCOAT_FACTOR},
      {"ClearCoat Roughness", DebugOutput::CLEARCOAT_ROUGHNESS},
      {"ClearCoat Normal", DebugOutput::CLEARCOAT_NORMAL},
      {"Sheen Color", DebugOutput::SHEEN_COLOR},
      {"Sheen Roughness", DebugOutput::SHEEN_ROUGHNESS},
      {"Specular Strength", DebugOutput::SPECULAR_FACTOR},
      {"Specular Color", DebugOutput::SPECULAR_COLOR},
      {"Transmission Strength", DebugOutput::TRANSMISSION_FACTOR},
      {"Volume Thickness", DebugOutput::VOLUME_THICKNESS},
      {"Diffuse Transmission Strength",
       DebugOutput::DIFFUSE_TRANSMISSION_FACTOR},
      {"Diffuse Transmission Color",
       DebugOutput::DIFFUSE_TRANSMISSION_COLOR_FACTOR},
      {"Iridescence Strength", DebugOutput::IRIDESCENCE_FACTOR},
      {"Iridescence Thickness", DebugOutput::IRIDESCENCE_THICKNESS},
      {"Anisotropic Strength", DebugOutput::ANISOTROPIC_STRENGTH},
      {"Anisotropic Direction", DebugOutput::ANISOTROPIC_DIRECTION}
  };

  auto it = stringToDebugOutput.find(str);
  return (it != stringToDebugOutput.end()) ? it->second : DebugOutput::NONE;
}

void GltfState::setUserCamera(const std::shared_ptr<UserCamera> &userCamera) {
  GltfState::userCamera = userCamera;
}

} // namespace digitalhumans