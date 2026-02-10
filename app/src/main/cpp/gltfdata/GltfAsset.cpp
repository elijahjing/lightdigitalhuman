//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfAsset.h"
#include "../utils/utils.h"
#include <android/log.h>
#include <regex>
#include <sstream>

#include "../utils/LogUtils.h"

namespace digitalhumans {

// 版本常量定义
const std::string GltfAsset::GLTF_VERSION_2_0 = "2.0";
const std::string GltfAsset::GLTF_VERSION_1_0 = "1.0";

GltfAsset::GltfAsset()
    : GltfObject(), copyright(), generator(), version(), minVersion() {
}


bool GltfAsset::validate() const {
  // 版本信息是必需的
  if (!version.has_value() || version->empty()) {
    LOGE("Asset version is required");
    return false;
  }

  // 验证版本格式
  int major, minor;
  if (!parseVersion(version.value(), major, minor)) {
    LOGE("Invalid version format: %s", version->c_str());
    return false;
  }

  // 如果有最小版本，也要验证格式
  if (minVersion.has_value() && !minVersion->empty()) {
    int minMajor, minMinor;
    if (!parseVersion(minVersion.value(), minMajor, minMinor)) {
      LOGE("Invalid minVersion format: %s", minVersion->c_str());
      return false;
    }

    // 最小版本不能大于当前版本
    if (compareVersions(minVersion.value(), version.value()) > 0) {
      LOGE("minVersion (%s) cannot be greater than version (%s)",
           minVersion->c_str(), version->c_str());
      return false;
    }
  }

  return true;
}

bool GltfAsset::isCompatibleWith(const std::string &targetVersion) const {
  if (!version.has_value()) {
    return false;
  }

  // 检查当前版本是否兼容目标版本
  int comparison = compareVersions(version.value(), targetVersion);

  // 如果有最小版本要求
  if (minVersion.has_value()) {
    int minComparison = compareVersions(targetVersion, minVersion.value());
    return minComparison >= 0; // 目标版本 >= 最小版本
  }

  // 简单的版本兼容性检查：主版本号相同
  int currentMajor, currentMinor;
  int targetMajor, targetMinor;

  if (parseVersion(version.value(), currentMajor, currentMinor) &&
      parseVersion(targetVersion, targetMajor, targetMinor)) {
    return currentMajor == targetMajor;
  }

  return false;
}

int GltfAsset::getMajorVersion() const {
  if (!version.has_value()) {
    return 0;
  }

  int major, minor;
  if (parseVersion(version.value(), major, minor)) {
    return major;
  }

  return 0;
}

int GltfAsset::getMinorVersion() const {
  if (!version.has_value()) {
    return 0;
  }

  int major, minor;
  if (parseVersion(version.value(), major, minor)) {
    return minor;
  }

  return 0;
}

bool GltfAsset::isGltf2Compatible() const {
  return isCompatibleWith(GLTF_VERSION_2_0);
}

void GltfAsset::reset() {
  copyright.reset();
  generator.reset();
  version.reset();
  minVersion.reset();
}

void GltfAsset::setAsGltf2(const std::string &generatorName) {
  version = GLTF_VERSION_2_0;
  if (!generatorName.empty()) {
    generator = generatorName;
  }
  // 通常glTF 2.0不需要设置minVersion
  minVersion.reset();
}

std::string GltfAsset::getVersionString() const {
  std::ostringstream oss;

  if (version.has_value()) {
    oss << "Version: " << version.value();

    if (minVersion.has_value()) {
      oss << " (Min: " << minVersion.value() << ")";
    }
  } else {
    oss << "Version: Not specified";
  }

  return oss.str();
}

std::string GltfAsset::getSummary() const {
  std::ostringstream oss;

  oss << "glTF Asset Summary:\n";

  // 版本信息
  oss << "  " << getVersionString() << "\n";

  // 生成器信息
  if (generator.has_value()) {
    oss << "  Generator: " << generator.value() << "\n";
  }

  // 版权信息
  if (copyright.has_value()) {
    oss << "  Copyright: " << copyright.value() << "\n";
  }

  // 兼容性信息
  oss << "  glTF 2.0 Compatible: " << (isGltf2Compatible() ? "Yes" : "No");

  return oss.str();
}

// === 私有方法实现 ===

bool GltfAsset::parseVersion(const std::string &versionStr,
                             int &outMajor,
                             int &outMinor) const {
  // 使用正则表达式解析版本号 (例如: "2.0", "1.0")
  std::regex versionRegex(R"(^(\d+)\.(\d+)$)");
  std::smatch matches;

  if (std::regex_match(versionStr, matches, versionRegex)) {
    try {
      outMajor = std::stoi(matches[1].str());
      outMinor = std::stoi(matches[2].str());
      return true;
    } catch (const std::exception &) {
      return false;
    }
  }

  return false;
}

int GltfAsset::compareVersions(const std::string &version1,
                               const std::string &version2) const {
  int major1, minor1, major2, minor2;

  if (!parseVersion(version1, major1, minor1) ||
      !parseVersion(version2, major2, minor2)) {
    return 0; // 无法比较时返回相等
  }

  if (major1 < major2) return -1;
  if (major1 > major2) return 1;

  if (minor1 < minor2) return -1;
  if (minor1 > minor2) return 1;

  return 0; // 相等
}

} // namespace digitalhumans