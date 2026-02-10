//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFSAMPLER_H
#define LIGHTDIGITALHUMAN_GLTFSAMPLER_H

#include "GltfObject.h"
#include "gl_constants.h"  // 或者包含OpenGL相关头文件
#include <string>
#include <vector>
#include <optional>
#include <memory>

namespace digitalhumans {

/**
 * @brief glTF采样器类
 * 定义纹理采样参数，包括过滤和包装模式
 */
class GltfSampler: public GltfObject {
 public:
  // 静态成员：动画属性列表
  static const std::vector<std::string> animatedProperties;

  /**
   * @brief 构造函数
   * @param magFilter 放大过滤器，默认为LINEAR
   * @param minFilter 缩小过滤器，默认为LINEAR_MIPMAP_LINEAR
   * @param wrapS S轴包装模式，默认为REPEAT
   * @param wrapT T轴包装模式，默认为REPEAT
   */
  explicit GltfSampler(
      int magFilter = GL_LINEAR,
      int minFilter = GL_LINEAR_MIPMAP_LINEAR,
      int wrapS = GL_REPEAT,
      int wrapT = GL_REPEAT
  );

  // 析构函数
  virtual ~GltfSampler() = default;

  // 拷贝构造函数和赋值操作符
  GltfSampler(const GltfSampler &other) = default;
  GltfSampler &operator=(const GltfSampler &other) = default;

  // 移动构造函数和赋值操作符
  GltfSampler(GltfSampler &&other) noexcept = default;
  GltfSampler &operator=(GltfSampler &&other) noexcept = default;

  /**
   * @brief 创建默认采样器
   * @return 默认采样器实例
   */
  static std::unique_ptr<GltfSampler> createDefault();


  /**
   * @brief 验证数据有效性
   * @return true如果数据有效
   */
  bool validate() const override;


  // 访问器方法
  int getMagFilter() const { return magFilter; }
  void setMagFilter(int filter);

  int getMinFilter() const { return minFilter; }
  void setMinFilter(int filter);

  int getWrapS() const { return wrapS; }
  void setWrapS(int wrap);

  int getWrapT() const { return wrapT; }
  void setWrapT(int wrap);

  const std::optional<std::string> &getName() const { return name; }
  void setName(const std::string &samplerName) { name = samplerName; }
  void clearName() { name = std::nullopt; }
  bool hasName() const { return name.has_value(); }

  /**
   * @brief 设置所有包装参数
   * @param wrapS S轴包装模式
   * @param wrapT T轴包装模式
   */
  void setWrap(int wrapS, int wrapT);

  /**
   * @brief 设置所有过滤参数
   * @param magFilter 放大过滤器
   * @param minFilter 缩小过滤器
   */
  void setFilter(int magFilter, int minFilter);

  /**
   * @brief 检查是否使用mipmap
   * @return true如果缩小过滤器使用mipmap
   */
  bool usesMipmap() const;

  /**
   * @brief 获取OpenGL兼容的参数
   * @param outMagFilter 输出放大过滤器
   * @param outMinFilter 输出缩小过滤器
   * @param outWrapS 输出S轴包装模式
   * @param outWrapT 输出T轴包装模式
   */
  void getGLParameters(int &outMagFilter, int &outMinFilter,
                       int &outWrapS, int &outWrapT) const;

  /**
   * @brief 应用到OpenGL纹理
   * @param target 纹理目标（通常是GL_TEXTURE_2D）
   */
  void applyToTexture(int target = GL_TEXTURE_2D) const;


  /**
   * @brief 重置到默认值
   */
  void reset();

  /**
   * @brief 调试输出
   * @return 描述字符串
   */
  std::string toString() const override;

  // 静态验证函数
  static bool isValidMagFilter(int filter);
  static bool isValidMinFilter(int filter);
  static bool isValidWrapMode(int wrap);

  // 常用预设
  static std::unique_ptr<GltfSampler> createNearest();
  static std::unique_ptr<GltfSampler> createLinear();
  static std::unique_ptr<GltfSampler> createMipmap();
  static std::unique_ptr<GltfSampler> createClamp();

 private:
  // glTF 采样器属性
  int magFilter;      ///< 放大过滤器
  int minFilter;      ///< 缩小过滤器
  int wrapS;          ///< S轴包装模式
  int wrapT;          ///< T轴包装模式
  std::optional<std::string> name;  ///< 采样器名称

  /**
   * @brief 验证过滤器参数
   * @return true如果所有过滤器参数有效
   */
  bool validateFilters() const;

  /**
   * @brief 验证包装参数
   * @return true如果所有包装参数有效
   */
  bool validateWrapModes() const;
};

} // namespace digitalhumans

#endif //LIGHTDIGITALHUMAN_GLTFSAMPLER_H
