//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFSHADER_H
#define LIGHTDIGITALHUMAN_GLTFSHADER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <any>
#include <variant>
#include <GLES3/gl3.h>
#include "glm.hpp"
#include "UniformTypes.h"

namespace digitalhumans {

/**
 * @brief uniform信息结构体
 */
struct UniformInfo {
  GLenum type;        ///< OpenGL uniform类型
  GLint location;     ///< uniform位置

  UniformInfo() : type(0), location(-1) {}
  UniformInfo(GLenum t, GLint loc) : type(t), location(loc) {}
};

/**
 * @brief glTF着色器类
 *
 * 封装OpenGL着色器程序，提供uniform和attribute的管理功能
 */
class GltfShader {
 public:
  /**
   * @brief 构造函数
   * @param program OpenGL着色器程序ID
   * @param hash 着色器哈希值
   * @param webgl WebGL上下文
   */
  GltfShader(GLuint program,
             std::string hash,
             std::shared_ptr<GltfOpenGLContext> webgl);

  /**
   * @brief 析构函数
   */
  virtual ~GltfShader();

  /**
   * @brief 禁用拷贝构造
   */
  GltfShader(const GltfShader &) = delete;

  /**
   * @brief 禁用拷贝赋值
   */
  GltfShader &operator=(const GltfShader &) = delete;

  /**
   * @brief 移动构造函数
   */
  GltfShader(GltfShader &&other) noexcept;

  /**
   * @brief 移动赋值操作符
   */
  GltfShader &operator=(GltfShader &&other) noexcept;

  /**
   * @brief 销毁着色器资源
   */
  void destroy();

  /**
   * @brief 使用此着色器程序
   */
  void use() const;

  /**
   * @brief 检查着色器程序是否有效
   * @return 如果有效返回true
   */
  bool isValid() const { return program != 0; }

  /**
   * @brief 获取attribute位置
   * @param name attribute名称
   * @return attribute位置，如果不存在返回-1
   */
  GLint getAttributeLocation(const std::string &name);

  /**
   * @brief 获取uniform位置
   * @param name uniform名称
   * @return uniform位置，如果不存在返回-1
   */
  GLint getUniformLocation(const std::string &name);

  /**
   * @brief 更新uniform值
   * @param objectName uniform名称
   * @param object uniform值
   * @param log 是否记录日志
   */
  void updateUniform(const std::string &objectName,
                     const UniformValue &object,
                     bool log = false);

  void updateLightArray(const std::string &objectName,
                        const std::shared_ptr<std::vector<UniformLight>> &lights,
                        bool log);        /**
         * @brief 更新uniform数组
         * @param arrayName 数组名称
         * @param array 数组值
         * @param log 是否记录日志
         */
  void updateUniformArray(const std::string &arrayName,
                          const std::vector<UniformValue> &array,
                          bool log = false);

  /**
   * @brief 更新uniform结构体
   * @param structName 结构体名称
   * @param object 结构体对象
   * @param log 是否记录日志
   */
  void updateUniformStruct(const std::string &structName,
                           std::shared_ptr<UniformStruct> object,
                           bool log = false);

  /**
   * @brief 更新uniform值（具体实现）
   * @param uniformName uniform名称
   * @param value uniform值
   * @param log 是否记录日志
   */
  void updateUniformValue(const std::string &uniformName,
                          const UniformValue &value,
                          bool log = false);

  // === 便利的设置方法 ===

  /**
   * @brief 设置浮点数uniform
   * @param name uniform名称
   * @param value 浮点数值
   */
  void setUniform(const std::string &name, float value);

  /**
   * @brief 设置整数uniform
   * @param name uniform名称
   * @param value 整数值
   */
  void setUniform(const std::string &name, int value);

  /**
   * @brief 设置布尔uniform
   * @param name uniform名称
   * @param value 布尔值
   */
  void setUniform(const std::string &name, bool value);

  /**
   * @brief 设置2D向量uniform
   * @param name uniform名称
   * @param value 2D向量值
   */
  void setUniform(const std::string &name, const glm::vec2 &value);

  /**
   * @brief 设置3D向量uniform
   * @param name uniform名称
   * @param value 3D向量值
   */
  void setUniform(const std::string &name, const glm::vec3 &value);

  /**
   * @brief 设置4D向量uniform
   * @param name uniform名称
   * @param value 4D向量值
   */
  void setUniform(const std::string &name, const glm::vec4 &value);

  /**
   * @brief 设置3x3矩阵uniform
   * @param name uniform名称
   * @param value 3x3矩阵值
   */
  void setUniform(const std::string &name, const glm::mat3 &value);

  /**
   * @brief 设置4x4矩阵uniform
   * @param name uniform名称
   * @param value 4x4矩阵值
   */
  void setUniform(const std::string &name, const glm::mat4 &value);

  /**
   * @brief 设置浮点数数组uniform
   * @param name uniform名称
   * @param values 浮点数数组
   */
  void setUniform(const std::string &name, const std::vector<float> &values);

  /**
   * @brief 设置整数数组uniform
   * @param name uniform名称
   * @param values 整数数组
   */
  void setUniform(const std::string &name, const std::vector<int> &values);

  // === Getter方法 ===

  /**
   * @brief 获取着色器程序ID
   * @return 着色器程序ID
   */
  GLuint getProgram() const { return program; }

  /**
   * @brief 获取着色器哈希值
   * @return 哈希值
   */
  const std::string &getHash() const { return hash; }

  /**
   * @brief 获取uniform数量
   * @return uniform数量
   */
  size_t getUniformCount() const { return uniforms.size(); }

  /**
   * @brief 获取attribute数量
   * @return attribute数量
   */
  size_t getAttributeCount() const { return attributes.size(); }

  /**
   * @brief 获取未知uniform列表
   * @return 未知uniform名称列表
   */
  const std::vector<std::string> &
  getUnknownUniforms() const { return unknownUniforms; }

  /**
   * @brief 获取未知attribute列表
   * @return 未知attribute名称列表
   */
  const std::vector<std::string> &
  getUnknownAttributes() const { return unknownAttributes; }

  /**
   * @brief 检查是否有未知uniform或attribute
   * @return 如果有未知项返回true
   */
  bool hasUnknownItems() const {
    return !unknownUniforms.empty() || !unknownAttributes.empty();
  }

  /**
   * @brief 清除未知项记录
   */
  void clearUnknownItems();

  /**
   * @brief 获取着色器信息字符串
   * @return 信息字符串
   */
  std::string getInfoString() const;

  /**
   * @brief 验证着色器程序
   * @return 如果验证成功返回true
   */
  bool validate() const;

  /**
   * @brief 检查OpenGL错误
   * @param operation 操作描述
   * @return 如果没有错误返回true
   */
  bool checkGLError(const std::string &operation) const;

 private:
  /**
   * @brief 初始化uniform和attribute信息
   */
  void initializeUniforms();

  /**
   * @brief 初始化attribute信息
   */
  void initializeAttributes();

  /**
   * @brief 扁平化数组以便上传到GPU
   * @param array 输入数组
   * @return 扁平化后的数组
   */
  std::vector<float> flattenArray(const std::vector<UniformValue> &array) const;

  /**
   * @brief 记录未知uniform
   * @param name uniform名称
   */
  void recordUnknownUniform(const std::string &name);

  /**
   * @brief 记录未知attribute
   * @param name attribute名称
   */
  void recordUnknownAttribute(const std::string &name);

  /**
   * @brief 获取OpenGL类型名称
   * @param type OpenGL类型
   * @return 类型名称字符串
   */
  std::string getGLTypeName(GLenum type) const;

 private:
  GLuint program;                                           ///< OpenGL着色器程序ID
  std::string hash;                                         ///< 着色器哈希值
  std::unordered_map<std::string, UniformInfo> uniforms;   ///< uniform信息映射
  std::unordered_map<std::string, GLint> attributes;       ///< attribute位置映射
  std::vector<std::string> unknownUniforms;                ///< 未知uniform名称列表
  std::vector<std::string> unknownAttributes;              ///< 未知attribute名称列表
  std::unordered_set<std::string>
      reportedUnknownUniforms; ///< 已报告的未知uniform（避免重复日志）
  std::unordered_set<std::string>
      reportedUnknownAttributes; ///< 已报告的未知attribute（避免重复日志）
  std::shared_ptr<GltfOpenGLContext> gl;                           ///< WebGL上下文

  // 统计信息
  mutable size_t uniformUpdateCount;      ///< uniform更新次数
  mutable size_t attributeQueryCount;     ///< attribute查询次数

 public:
  /**
   * @brief 获取uniform更新次数
   * @return 更新次数
   */
  size_t getUniformUpdateCount() const { return uniformUpdateCount; }

  /**
   * @brief 获取attribute查询次数
   * @return 查询次数
   */
  size_t getAttributeQueryCount() const { return attributeQueryCount; }

  /**
   * @brief 重置统计信息
   */
  void resetStatistics();
};

} // namespace digitalhumans


#endif //LIGHTDIGITALHUMAN_GLTFSHADER_H
