//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFASSET_H
#define LIGHTDIGITALHUMAN_GLTFASSET_H



#include "GltfObject.h"
#include <string>
#include <optional>
#include <memory>

namespace digitalhumans {

/**
 * @brief glTF资源信息类
 *
 * 包含glTF文件的版本信息、版权声明、生成器信息等元数据
 */
class GltfAsset: public GltfObject {
 public:
  /**
   * @brief 构造函数
   */
  GltfAsset();

  /**
   * @brief 析构函数
   */
  ~GltfAsset() override = default;


  /**
   * @brief 验证资源信息的有效性
   * @return 如果资源信息有效返回true
   */
  bool validate() const override;

  /**
   * @brief 检查版本兼容性
   * @param targetVersion 目标版本
   * @return 如果兼容返回true
   */
  bool isCompatibleWith(const std::string &targetVersion) const;

  /**
   * @brief 获取主版本号
   * @return 主版本号，如果版本未设置则返回0
   */
  int getMajorVersion() const;

  /**
   * @brief 获取次版本号
   * @return 次版本号，如果版本未设置则返回0
   */
  int getMinorVersion() const;

  /**
   * @brief 检查是否支持glTF 2.0
   * @return 如果支持glTF 2.0返回true
   */
  bool isGltf2Compatible() const;

  /**
   * @brief 重置为默认值
   */
  void reset();

  /**
   * @brief 设置为标准glTF 2.0资源信息
   * @param generatorName 生成器名称（可选）
   */
  void setAsGltf2(const std::string &generatorName = "");

  // === Getter/Setter方法 ===

  /**
   * @brief 获取版权信息
   * @return 版权信息字符串，如果未设置则返回std::nullopt
   */
  const std::optional<std::string> &getCopyright() const { return copyright; }

  /**
   * @brief 设置版权信息
   * @param copyrightText 版权信息字符串
   */
  void setCopyright(const std::string &copyrightText) {
    copyright = copyrightText;
  }

  /**
   * @brief 清除版权信息
   */
  void clearCopyright() { copyright.reset(); }

  /**
   * @brief 检查是否有版权信息
   * @return 如果有版权信息返回true
   */
  bool hasCopyright() const { return copyright.has_value(); }

  /**
   * @brief 获取生成器信息
   * @return 生成器信息字符串，如果未设置则返回std::nullopt
   */
  const std::optional<std::string> &getGenerator() const { return generator; }

  /**
   * @brief 设置生成器信息
   * @param generatorText 生成器信息字符串
   */
  void setGenerator(const std::string &generatorText) {
    generator = generatorText;
  }

  /**
   * @brief 清除生成器信息
   */
  void clearGenerator() { generator.reset(); }

  /**
   * @brief 检查是否有生成器信息
   * @return 如果有生成器信息返回true
   */
  bool hasGenerator() const { return generator.has_value(); }

  /**
   * @brief 获取版本信息
   * @return 版本信息字符串，如果未设置则返回std::nullopt
   */
  const std::optional<std::string> &getVersion() const { return version; }

  /**
   * @brief 设置版本信息
   * @param versionText 版本信息字符串
   */
  void setVersion(const std::string &versionText) { version = versionText; }

  /**
   * @brief 清除版本信息
   */
  void clearVersion() { version.reset(); }

  /**
   * @brief 检查是否有版本信息
   * @return 如果有版本信息返回true
   */
  bool hasVersion() const { return version.has_value(); }

  /**
   * @brief 获取最小版本信息
   * @return 最小版本信息字符串，如果未设置则返回std::nullopt
   */
  const std::optional<std::string> &getMinVersion() const { return minVersion; }

  /**
   * @brief 设置最小版本信息
   * @param minVersionText 最小版本信息字符串
   */
  void setMinVersion(const std::string &minVersionText) {
    minVersion = minVersionText;
  }

  /**
   * @brief 清除最小版本信息
   */
  void clearMinVersion() { minVersion.reset(); }

  /**
   * @brief 检查是否有最小版本信息
   * @return 如果有最小版本信息返回true
   */
  bool hasMinVersion() const { return minVersion.has_value(); }

  /**
   * @brief 获取版本信息字符串（用于显示）
   * @return 格式化的版本信息字符串
   */
  std::string getVersionString() const;

  /**
   * @brief 获取资源信息摘要
   * @return 资源信息摘要字符串
   */
  std::string getSummary() const;

  // 常用版本常量
  static const std::string GLTF_VERSION_2_0;
  static const std::string GLTF_VERSION_1_0;

 private:
  /**
   * @brief 解析版本号
   * @param versionStr 版本字符串
   * @param outMajor 输出主版本号
   * @param outMinor 输出次版本号
   * @return 如果解析成功返回true
   */
  bool parseVersion(const std::string &versionStr,
                    int &outMajor,
                    int &outMinor) const;

  /**
   * @brief 比较版本号
   * @param version1 版本1
   * @param version2 版本2
   * @return -1表示version1 < version2，0表示相等，1表示version1 > version2
   */
  int compareVersions(const std::string &version1,
                      const std::string &version2) const;

 private:
  std::optional<std::string> copyright;       ///< 版权信息
  std::optional<std::string> generator;       ///< 生成器信息
  std::optional<std::string> version;         ///< 版本信息
  std::optional<std::string> minVersion;      ///< 最小版本信息
};

} // namespace digitalhumans



#endif //LIGHTDIGITALHUMAN_GLTFASSET_H
