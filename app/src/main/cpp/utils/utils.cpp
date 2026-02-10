//
// Created by vincentsyan on 2025/8/18.
//

#include "utils.h"
#include <cctype>
#include <sstream>
#include <typeinfo>
#include <stdexcept>
#include "../gltfdata/GltfOpenGLContext.h"
#include "../gltfdata/Gltf.h"
namespace digitalhumans {
namespace utils {


// glm::mat4 特化实现
template<>
glm::mat4 jsToGl<glm::mat4>(const std::vector<float> &array) {
  if (array.size() != 16) {
    return glm::mat4(1.0f); // 返回单位矩阵作为默认值
  }

  // GLM 矩阵是列主序（column-major）存储
  // JavaScript/JSON 通常是行主序，所以需要转置
  return glm::mat4(
      array[0], array[1], array[2], array[3],   // 第一列
      array[4], array[5], array[6], array[7],   // 第二列
      array[8], array[9], array[10], array[11],  // 第三列
      array[12], array[13], array[14], array[15]   // 第四列
  );
}

// 如果需要其他类型，也可以实现
template<>
glm::mat3 jsToGl<glm::mat3>(const std::vector<float> &array) {
  if (array.size() != 9) {
    return glm::mat3(1.0f);
  }

  return glm::mat3(
      array[0], array[1], array[2],  // 第一列
      array[3], array[4], array[5],  // 第二列
      array[6], array[7], array[8]   // 第三列
  );
}

template<>
glm::vec3 jsToGl<glm::vec3>(const std::vector<float> &array) {
  if (array.size() != 3) {
    return glm::vec3(0.0f);
  }

  return glm::vec3(array[0], array[1], array[2]);
}

template<>
glm::vec4 jsToGl<glm::vec4>(const std::vector<float> &array) {
  if (array.size() != 4) {
    return glm::vec4(0.0f);
  }

  return glm::vec4(array[0], array[1], array[2], array[3]);
}


void initGlForMembers(GltfObject &gltfObj,
                      std::shared_ptr<Gltf> gltf,
                      std::shared_ptr<GltfOpenGLContext> webGlContext) {
  // 注意：在C++中我们不能像JavaScript那样动态遍历对象成员
  // 这个函数需要每个具体的glTF对象类型重写以处理其特定成员
  // gltfObj.initGl(gltf, webGlContext);
}

void fromKeys(std::any &target,
              const std::unordered_map<std::string, std::any> &jsonObj,
              const std::vector<std::string> &ignore) {
  for (const auto &[key, value]: jsonObj) {
    // 检查是否在忽略列表中
    if (std::find(ignore.begin(), ignore.end(), key) != ignore.end()) {
      continue;
    }

    // 移除键名中的 "^@" 前缀
    std::string normalizedKey = key;
    if (normalizedKey.substr(0, 2) == "^@") {
      normalizedKey = normalizedKey.substr(2);
    }

    // 在C++中，我们需要知道target的具体类型才能设置成员
    // 这里提供一个基础实现，具体类型需要重写此逻辑
    try {
      auto &targetObj =
          std::any_cast<std::unordered_map<std::string, std::any> &>(target);
      targetObj[normalizedKey] = value;
    } catch (const std::bad_any_cast &e) {
      // target不是map类型，需要具体类型处理
    }
  }
}

void fromParams(const std::vector<std::string> &parameters,
                std::any &target,
                const std::unordered_map<std::string, std::any> &jsonObj) {
  for (const std::string &param: parameters) {
    auto it = jsonObj.find(param);
    if (it != jsonObj.end()) {
      try {
        auto &targetObj =
            std::any_cast<std::unordered_map<std::string, std::any> &>(target);
        targetObj[param] = it->second;
      } catch (const std::bad_any_cast &e) {
        // target不是map类型，需要具体类型处理
      }
    }
  }
}

int32_t stringHash(const std::string &str, int32_t seed) {
  int32_t hash = seed;
  if (str.empty()) return hash;

  for (char c: str) {
    hash = ((hash << 5) - hash) + static_cast<int32_t>(c);
    // 转换为32位整数（在C++中int32_t已经是32位）
  }
  return hash;
}

bool getIsGlb(const std::string &filename) {
  auto ext = getExtension(filename);
  return ext && *ext == "glb";
}

bool getIsGltf(const std::string &filename) {
  auto ext = getExtension(filename);
  return ext && *ext == "gltf";
}

bool getIsHdr(const std::string &filename) {
  auto ext = getExtension(filename);
  return ext && *ext == "hdr";
}

std::optional<std::string> getExtension(const std::string &filename) {
  std::string lowerFilename = filename;
  std::transform(lowerFilename.begin(),
                 lowerFilename.end(),
                 lowerFilename.begin(),
                 [](char c) { return std::tolower(c); });

  size_t dotPos = lowerFilename.find_last_of('.');
  if (dotPos == std::string::npos) {
    return std::nullopt;
  }

  return lowerFilename.substr(dotPos + 1);
}

std::string getFileName(const std::string &filePath) {
  size_t slashPos = filePath.find_last_of('/');
  if (slashPos == std::string::npos) {
    return filePath;
  }
  return filePath.substr(slashPos + 1);
}

std::string getFileNameWithoutExtension(const std::string &filePath) {
  std::string filename = getFileName(filePath);
  size_t dotPos = filename.find_last_of('.');
  if (dotPos == std::string::npos) {
    return filename;
  }
  return filename.substr(0, dotPos);
}

std::string getContainingFolder(const std::string &filePath) {
  size_t slashPos = filePath.find_last_of('/');
  if (slashPos == std::string::npos) {
    return "";
  }
  return filePath.substr(0, slashPos + 1);
}

std::string combinePaths(const std::vector<std::string> &paths) {
  if (paths.empty()) {
    return "";
  }

  std::ostringstream result;
  for (size_t i = 0; i < paths.size(); ++i) {
    if (i > 0) {
      result << "/";
    }
    result << paths[i];
  }
  return result.str();
}

Timer::Timer() : seconds(0.0) {
}

void Timer::start() {
  startTime = std::chrono::high_resolution_clock::now();
  seconds = 0.0;
}

void Timer::stop() {
  endTime = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      endTime - startTime);
  seconds = duration.count() / 1000000.0;
}

AnimationTimer::AnimationTimer()
    : paused(true), fixedTime(std::nullopt), pausedTime(0.0) {
}

double AnimationTimer::elapsedSec() {
  if (paused) {
    return pausedTime / 1000.0;
  } else {
    if (fixedTime.has_value()) {
      return fixedTime.value();
    } else {
      auto now = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
          now - startTimePoint);
      return duration.count() / 1000.0;
    }
  }
}

void AnimationTimer::toggle() {
  if (paused) {
    unpause();
  } else {
    pause();
  }
}

void AnimationTimer::start() {
  startTimePoint = std::chrono::high_resolution_clock::now();
  paused = false;
}

void AnimationTimer::pause() {
  auto now = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      now - startTimePoint);
  pausedTime = static_cast<double>(duration.count());
  paused = true;
}

void AnimationTimer::unpause() {
  auto now = std::chrono::high_resolution_clock::now();
  auto pauseDuration =
      std::chrono::milliseconds(static_cast<long long>(pausedTime));
  startTimePoint = now - pauseDuration;
  paused = false;
}

void AnimationTimer::reset() {
  if (!paused) {
    startTimePoint = std::chrono::high_resolution_clock::now();
  } else {
    startTimePoint = std::chrono::high_resolution_clock::time_point{};
  }
  pausedTime = 0.0;
}

void AnimationTimer::setFixedTime(double timeInSec) {
  paused = false;
  fixedTime = timeInSec;
}

} // namespace utils
} // namespace digitalhumans