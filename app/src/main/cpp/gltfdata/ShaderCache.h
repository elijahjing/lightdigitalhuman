//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_SHADERCACHE_H
#define LIGHTDIGITALHUMAN_SHADERCACHE_H


#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <functional>
#include <GLES3/gl3.h>

namespace digitalhumans {
class GltfShader;
class GltfOpenGLContext;

/**
 * @brief 着色器缓存类
 *
 * 管理着色器源代码、编译着色器、链接着色器程序，并提供缓存功能
 */
class ShaderCache {
 public:
  /**
   * @brief 着色器源代码映射类型
   */
  using SourceMap = std::unordered_map<std::string, std::string>;

  /**
   * @brief 着色器缓存映射类型 (hash -> shader ID)
   */
  using ShaderMap = std::unordered_map<size_t, GLuint>;

  /**
   * @brief 着色器程序缓存映射类型 (hash -> shader program)
   */
  using ProgramMap = std::unordered_map<std::string,
                                        std::shared_ptr<GltfShader>>;

  /**
   * @brief 预处理定义列表类型
   */
  using DefinesList = std::vector<std::string>;

  /**
   * @brief 构造函数
   * @param sources 着色器源代码映射 (着色器名 -> 源代码)
   * @param gl WebGL上下文
   */
  ShaderCache(const SourceMap &sources, std::shared_ptr<GltfOpenGLContext> gl);

  /**
   * @brief 析构函数
   */
  virtual ~ShaderCache();

  /**
   * @brief 禁用拷贝构造
   */
  ShaderCache(const ShaderCache &) = delete;

  /**
   * @brief 禁用拷贝赋值
   */
  ShaderCache &operator=(const ShaderCache &) = delete;

  /**
   * @brief 移动构造函数
   */
  ShaderCache(ShaderCache &&other) noexcept;

  /**
   * @brief 移动赋值操作符
   */
  ShaderCache &operator=(ShaderCache &&other) noexcept;

  /**
   * @brief 销毁缓存并释放所有资源
   */
  void destroy();

  /**
   * @brief 选择并编译着色器
   * @param shaderIdentifier 着色器标识符 (例如: "pbr.vert")
   * @param permutationDefines 预处理定义列表
   * @return 着色器哈希值，失败时返回0
   */
  size_t selectShader(const std::string &shaderIdentifier,
                      const DefinesList &permutationDefines);

  /**
   * @brief 获取着色器程序
   * @param vertexShaderHash 顶点着色器哈希值
   * @param fragmentShaderHash 片段着色器哈希值
   * @return 着色器程序对象，失败时返回nullptr
   */
  std::shared_ptr<GltfShader>
  getShaderProgram(size_t vertexShaderHash, size_t fragmentShaderHash);

  /**
   * @brief 添加着色器源代码
   * @param identifier 着色器标识符
   * @param source 源代码
   */
  void
  addShaderSource(const std::string &identifier, const std::string &source);

  /**
   * @brief 移除着色器源代码
   * @param identifier 着色器标识符
   * @return 如果成功移除返回true
   */
  bool removeShaderSource(const std::string &identifier);

  /**
   * @brief 清除所有缓存
   */
  void clearCache();

  /**
   * @brief 预编译着色器
   * @param shaderIdentifier 着色器标识符
   * @param definesList 预处理定义列表的列表
   * @return 成功编译的着色器数量
   */
  size_t precompileShaders(const std::string &shaderIdentifier,
                           const std::vector<DefinesList> &definesList);

  /**
   * @brief 检查着色器源代码是否存在
   * @param identifier 着色器标识符
   * @return 如果存在返回true
   */
  bool hasShaderSource(const std::string &identifier) const;

  /**
   * @brief 获取着色器源代码
   * @param identifier 着色器标识符
   * @return 源代码，如果不存在返回空字符串
   */
  std::string getShaderSource(const std::string &identifier) const;

  // === 统计和调试信息 ===

  /**
   * @brief 获取缓存的着色器数量
   * @return 着色器数量
   */
  size_t getShaderCount() const { return shaders.size(); }

  /**
   * @brief 获取缓存的程序数量
   * @return 程序数量
   */
  size_t getProgramCount() const { return programs.size(); }

  /**
   * @brief 获取源代码数量
   * @return 源代码数量
   */
  size_t getSourceCount() const { return sources.size(); }

  /**
   * @brief 获取编译失败次数
   * @return 编译失败次数
   */
  size_t getCompilationFailures() const { return compilationFailures; }

  /**
   * @brief 获取链接失败次数
   * @return 链接失败次数
   */
  size_t getLinkFailures() const { return linkFailures; }

  /**
   * @brief 获取缓存命中次数
   * @return 缓存命中次数
   */
  size_t getCacheHits() const { return cacheHits; }

  /**
   * @brief 获取缓存未命中次数
   * @return 缓存未命中次数
   */
  size_t getCacheMisses() const { return cacheMisses; }

  /**
   * @brief 重置统计信息
   */
  void resetStatistics();

  /**
   * @brief 获取调试信息字符串
   * @return 调试信息
   */
  std::string getDebugInfo() const;

  /**
   * @brief 验证所有缓存的着色器程序
   * @return 如果所有程序都有效返回true
   */
  bool validateAllPrograms() const;

  /**
   * @brief 设置最大缓存大小
   * @param maxShaders 最大着色器数量
   * @param maxPrograms 最大程序数量
   */
  void setMaxCacheSize(size_t maxShaders, size_t maxPrograms);

  /**
   * @brief 检查是否需要清理缓存
   * @return 如果需要清理返回true
   */
  bool needsCleanup() const;

  /**
   * @brief 执行LRU缓存清理
   * @param targetShaderCount 目标着色器数量
   * @param targetProgramCount 目标程序数量
   */
  void performLRUCleanup(size_t targetShaderCount, size_t targetProgramCount);

 private:
  /**
   * @brief 解析和展开include指令
   */
  void resolveIncludes();

  /**
   * @brief 计算字符串哈希值
   * @param str 输入字符串
   * @return 哈希值
   */
  size_t stringHash(const std::string &str) const;

  /**
   * @brief 检查着色器标识符是否为顶点着色器
   * @param identifier 着色器标识符
   * @return 如果是顶点着色器返回true
   */
  bool isVertexShader(const std::string &identifier) const;

  /**
   * @brief 检查着色器标识符是否为片段着色器
   * @param identifier 着色器标识符
   * @return 如果是片段着色器返回true
   */
  bool isFragmentShader(const std::string &identifier) const;

  /**
   * @brief 生成预处理定义字符串
   * @param defines 定义列表
   * @return 预处理定义字符串
   */
  std::string generateDefinesString(const DefinesList &defines) const;

  /**
   * @brief 编译着色器
   * @param identifier 着色器标识符
   * @param isVertex 是否为顶点着色器
   * @param source 完整源代码
   * @return 编译后的着色器ID，失败时返回0
   */
  GLuint compileShader(const std::string &identifier,
                       bool isVertex,
                       const std::string &source);

  /**
   * @brief 链接着色器程序
   * @param vertexShader 顶点着色器ID
   * @param fragmentShader 片段着色器ID
   * @return 链接后的程序ID，失败时返回0
   */
  GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);

  /**
   * @brief 检查着色器编译错误
   * @param shader 着色器ID
   * @param identifier 着色器标识符
   * @return 如果编译成功返回true
   */
  bool
  checkShaderCompilation(GLuint shader, const std::string &identifier) const;

  /**
   * @brief 检查程序链接错误
   * @param program 程序ID
   * @return 如果链接成功返回true
   */
  bool checkProgramLinking(GLuint program) const;

  /**
   * @brief 清理单个着色器
   * @param shader 着色器ID
   */
  void cleanupShader(GLuint shader);

  /**
   * @brief 清理单个程序
   * @param program 程序对象
   */
  void cleanupProgram(std::shared_ptr<GltfShader> program);

  /**
   * @brief 生成程序哈希字符串
   * @param vertexHash 顶点着色器哈希
   * @param fragmentHash 片段着色器哈希
   * @return 程序哈希字符串
   */
  std::string generateProgramHash(size_t vertexHash, size_t fragmentHash) const;

  /**
   * @brief 检查OpenGL错误
   * @param operation 操作描述
   * @return 如果没有错误返回true
   */
  bool checkGLError(const std::string &operation) const;

  bool resolveIncludesRecursive(const std::string &currentShader,
                                std::string &source);


 private:
  SourceMap sources;                      ///< 着色器源代码映射
  ShaderMap shaders;                      ///< 编译后的着色器缓存
  ProgramMap programs;                    ///< 链接后的程序缓存
  std::shared_ptr<GltfOpenGLContext> gl;         ///< WebGL上下文

  // 缓存管理
  size_t maxShaderCacheSize;              ///< 最大着色器缓存数量
  size_t maxProgramCacheSize;             ///< 最大程序缓存数量

  // 统计信息
  mutable size_t compilationFailures;     ///< 编译失败次数
  mutable size_t linkFailures;            ///< 链接失败次数
  mutable size_t cacheHits;               ///< 缓存命中次数
  mutable size_t cacheMisses;             ///< 缓存未命中次数

  // LRU支持
  mutable std::unordered_map<size_t, size_t> shaderAccessOrder;     ///< 着色器访问顺序
  mutable std::unordered_map<std::string, size_t>
      programAccessOrder; ///< 程序访问顺序
  mutable size_t accessCounter;           ///< 访问计数器

  // Include循环检测
  std::unordered_set<std::string> resolvedIncludes; ///< 已解析的include
  mutable std::unordered_set<std::string>
      currentlyResolving; ///< 正在解析的include（用于检测循环）

 public:
  /**
   * @brief 启用/禁用详细日志
   * @param enabled 是否启用
   */
  void setVerboseLogging(bool enabled) { verboseLogging = enabled; }

  /**
   * @brief 检查是否启用详细日志
   * @return 如果启用返回true
   */
  bool isVerboseLoggingEnabled() const { return verboseLogging; }

 private:
  bool verboseLogging;                    ///< 详细日志标志
};

} // namespace digitalhumans



#endif //LIGHTDIGITALHUMAN_SHADERCACHE_H
