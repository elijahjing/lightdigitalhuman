//
// Created by vincentsyan on 2025/8/18.
//

#include "ShaderCache.h"

#include <android/log.h>
#include <algorithm>
#include <sstream>
#include <regex>
#include <iomanip>
#include "GltfShader.h"

#include "../utils/LogUtils.h"


namespace digitalhumans {

ShaderCache::ShaderCache(const SourceMap &sourceMaps,
                         std::shared_ptr<GltfOpenGLContext> webgl)
    : sources(sourceMaps), shaders(), programs(), gl(webgl),
      maxShaderCacheSize(1000), maxProgramCacheSize(500),
      compilationFailures(0), linkFailures(0), cacheHits(0), cacheMisses(0),
      shaderAccessOrder(),
      programAccessOrder(), accessCounter(0), resolvedIncludes(),
      currentlyResolving(), verboseLogging(false) {
  if (!gl) {
    LOGE("Invalid WebGL context provided to ShaderCache");
    return;
  }

  // 解析和展开include指令
  resolveIncludes();
}

ShaderCache::~ShaderCache() {
  destroy();
}

ShaderCache::ShaderCache(ShaderCache &&other) noexcept
    : sources(std::move(other.sources)), shaders(std::move(other.shaders)),
      programs(std::move(other.programs)),
      gl(std::move(other.gl)), maxShaderCacheSize(other.maxShaderCacheSize),
      maxProgramCacheSize(other.maxProgramCacheSize),
      compilationFailures(other.compilationFailures),
      linkFailures(other.linkFailures), cacheHits(other.cacheHits),
      cacheMisses(other.cacheMisses),
      shaderAccessOrder(std::move(other.shaderAccessOrder)),
      programAccessOrder(std::move(other.programAccessOrder)),
      accessCounter(other.accessCounter),
      resolvedIncludes(std::move(other.resolvedIncludes)),
      currentlyResolving(std::move(other.currentlyResolving)),
      verboseLogging(other.verboseLogging) {
  other.maxShaderCacheSize = 0;
  other.maxProgramCacheSize = 0;
  other.compilationFailures = 0;
  other.linkFailures = 0;
  other.cacheHits = 0;
  other.cacheMisses = 0;
  other.accessCounter = 0;
  other.verboseLogging = false;
}

ShaderCache &ShaderCache::operator=(ShaderCache &&other) noexcept {
  if (this != &other) {
    destroy();

    sources = std::move(other.sources);
    shaders = std::move(other.shaders);
    programs = std::move(other.programs);
    gl = std::move(other.gl);
    maxShaderCacheSize = other.maxShaderCacheSize;
    maxProgramCacheSize = other.maxProgramCacheSize;
    compilationFailures = other.compilationFailures;
    linkFailures = other.linkFailures;
    cacheHits = other.cacheHits;
    cacheMisses = other.cacheMisses;
    shaderAccessOrder = std::move(other.shaderAccessOrder);
    programAccessOrder = std::move(other.programAccessOrder);
    accessCounter = other.accessCounter;
    resolvedIncludes = std::move(other.resolvedIncludes);
    currentlyResolving = std::move(other.currentlyResolving);
    verboseLogging = other.verboseLogging;

    other.maxShaderCacheSize = 0;
    other.maxProgramCacheSize = 0;
    other.compilationFailures = 0;
    other.linkFailures = 0;
    other.cacheHits = 0;
    other.cacheMisses = 0;
    other.accessCounter = 0;
    other.verboseLogging = false;
  }
  return *this;
}

void ShaderCache::destroy() {

  // 清理所有着色器程序
  for (auto &[hash, program]: programs) {
    if (program) {
      program->destroy();
    }
  }
  programs.clear();

  // 清理所有着色器
  if (gl) {
    for (auto &[hash, shader]: shaders) {
      if (shader != 0) {
        glDeleteShader(shader);
      }
    }
  }
  shaders.clear();

  // 清理其他数据
  sources.clear();
  shaderAccessOrder.clear();
  programAccessOrder.clear();
  resolvedIncludes.clear();
  currentlyResolving.clear();

  if (gl) {
    checkGLError("destroy shader cache");
  }
}

size_t ShaderCache::selectShader(const std::string &shaderIdentifier,
                                 const DefinesList &permutationDefines) {
  // 查找着色器源代码
  auto sourceIt = sources.find(shaderIdentifier);
  if (sourceIt == sources.end()) {
    LOGE("Shader source for %s not found", shaderIdentifier.c_str());
    return 0;
  }

  const std::string &src = sourceIt->second;
  const bool isVert = isVertexShader(shaderIdentifier);

  // 计算哈希值
  size_t hash = stringHash(shaderIdentifier);
  for (const auto &define: permutationDefines) {
    hash ^= stringHash(define);
  }

  // 检查缓存
  auto shaderIt = shaders.find(hash);
  if (shaderIt != shaders.end()) {
    cacheHits++;
    shaderAccessOrder[hash] = ++accessCounter;
    return hash;
  }

  cacheMisses++;

  // 生成完整的着色器源代码
  std::string defines = "#version 300 es\n";
  for (const auto &define: permutationDefines) {
    defines += "#define " + define + "\n";
    LOGV("Adding define: %s", define.c_str());
  }

  std::string fullSource = defines + src;

  // 编译着色器
  GLuint shader = compileShader(shaderIdentifier, isVert, fullSource);
  if (shader == 0) {
    compilationFailures++;
    LOGE("Failed to compile shader: %s", shaderIdentifier.c_str());
    return 0;
  }

  // 缓存着色器
  shaders[hash] = shader;
  shaderAccessOrder[hash] = ++accessCounter;

  // 检查是否需要清理缓存
  if (needsCleanup()) {
    performLRUCleanup(maxShaderCacheSize * 0.8, maxProgramCacheSize * 0.8);
  }

  return hash;
}

std::shared_ptr<GltfShader>
ShaderCache::getShaderProgram(size_t vertexShaderHash,
                              size_t fragmentShaderHash) {
  if (vertexShaderHash == 0 || fragmentShaderHash == 0) {
    LOGE("Invalid shader hashes provided: vertex=%zu, fragment=%zu",
         vertexShaderHash,
         fragmentShaderHash);
    return nullptr;
  }

  // 生成程序哈希
  std::string programHashStr =
      generateProgramHash(vertexShaderHash, fragmentShaderHash);

  // 检查程序缓存
  auto programIt = programs.find(programHashStr);
  if (programIt != programs.end() && programIt->second) {
    cacheHits++;
    programAccessOrder[programHashStr] = ++accessCounter;
    return programIt->second;
  }

  cacheMisses++;

  // 获取着色器
  auto vertexShaderIt = shaders.find(vertexShaderHash);
  auto fragmentShaderIt = shaders.find(fragmentShaderHash);

  if (vertexShaderIt == shaders.end() || fragmentShaderIt == shaders.end()) {
    LOGE("Referenced shaders not found in cache: vertex=%s, fragment=%s",
         vertexShaderIt == shaders.end() ? "missing" : "found",
         fragmentShaderIt == shaders.end() ? "missing" : "found");
    return nullptr;
  }

  // 链接程序
  GLuint linkedProgram =
      linkProgram(vertexShaderIt->second, fragmentShaderIt->second);
  if (linkedProgram == 0) {
    linkFailures++;
    LOGE("Failed to link shader program");
    return nullptr;
  }

  // 创建着色器程序对象
  auto
      program = std::make_shared<GltfShader>(linkedProgram, programHashStr, gl);
  if (!program->isValid()) {
    LOGE("Failed to create valid GltfShader object");
    glDeleteProgram(linkedProgram);
    return nullptr;
  }

  // 缓存程序
  programs[programHashStr] = program;
  programAccessOrder[programHashStr] = ++accessCounter;
  return program;
}

void ShaderCache::addShaderSource(const std::string &identifier,
                                  const std::string &source) {
  sources[identifier] = source;
}

bool ShaderCache::removeShaderSource(const std::string &identifier) {
  auto it = sources.find(identifier);
  if (it != sources.end()) {
    sources.erase(it);
    return true;
  }
  return false;
}

void ShaderCache::clearCache() {
  // 清理程序
  for (auto &[hash, program]: programs) {
    if (program) {
      program->destroy();
    }
  }
  programs.clear();

  // 清理着色器
  if (gl) {
    for (auto &[hash, shader]: shaders) {
      if (shader != 0) {
        glDeleteShader(shader);
      }
    }
  }
  shaders.clear();

  // 清理访问记录
  shaderAccessOrder.clear();
  programAccessOrder.clear();
  accessCounter = 0;
}

size_t
ShaderCache::precompileShaders(const std::string &shaderIdentifier,
                               const std::vector<DefinesList> &definesList) {

  size_t successCount = 0;
  for (const auto &defines: definesList) {
    size_t hash = selectShader(shaderIdentifier, defines);
    if (hash != 0) {
      successCount++;
    }
  }
  return successCount;
}

bool ShaderCache::hasShaderSource(const std::string &identifier) const {
  return sources.find(identifier) != sources.end();
}

std::string ShaderCache::getShaderSource(const std::string &identifier) const {
  auto it = sources.find(identifier);
  return (it != sources.end()) ? it->second : std::string();
}

void ShaderCache::resetStatistics() {
  compilationFailures = 0;
  linkFailures = 0;
  cacheHits = 0;
  cacheMisses = 0;
}


bool ShaderCache::validateAllPrograms() const {
  bool allValid = true;
  for (const auto &[hash, program]: programs) {
    if (program && !program->validate()) {
      LOGE("Program validation failed for hash: %s", hash.c_str());
      allValid = false;
    }
  }
  return allValid;
}

void ShaderCache::setMaxCacheSize(size_t maxShaders, size_t maxPrograms) {
  maxShaderCacheSize = maxShaders;
  maxProgramCacheSize = maxPrograms;
  LOGI("Updated cache limits: shaders=%zu, programs=%zu",
       maxShaders,
       maxPrograms);
}

bool ShaderCache::needsCleanup() const {
  return shaders.size() > maxShaderCacheSize
      || programs.size() > maxProgramCacheSize;
}

void ShaderCache::performLRUCleanup(size_t targetShaderCount,
                                    size_t targetProgramCount) {
  LOGV("Performing LRU cleanup: target shaders=%zu, programs=%zu",
       targetShaderCount,
       targetProgramCount);

  // 清理着色器
  if (shaders.size() > targetShaderCount) {
    std::vector<std::pair<size_t, size_t>>
        shadersByAccess; // (hash, access_time)
    for (const auto &[hash, shader]: shaders) {
      size_t accessTime =
          shaderAccessOrder.count(hash) ? shaderAccessOrder[hash] : 0;
      shadersByAccess.emplace_back(hash, accessTime);
    }

    std::sort(shadersByAccess.begin(), shadersByAccess.end(),
              [](const auto &a, const auto &b) { return a.second < b.second; });

    size_t toRemove = shaders.size() - targetShaderCount;
    for (size_t i = 0; i < toRemove && i < shadersByAccess.size(); ++i) {
      size_t hash = shadersByAccess[i].first;
      auto it = shaders.find(hash);
      if (it != shaders.end()) {
        cleanupShader(it->second);
        shaders.erase(it);
        shaderAccessOrder.erase(hash);
      }
    }
    LOGV("Cleaned up %zu shaders", toRemove);
  }

  // 清理程序
  if (programs.size() > targetProgramCount) {
    std::vector<std::pair<std::string, size_t>>
        programsByAccess; // (hash, access_time)
    for (const auto &[hash, program]: programs) {
      size_t accessTime =
          programAccessOrder.count(hash) ? programAccessOrder[hash] : 0;
      programsByAccess.emplace_back(hash, accessTime);
    }

    std::sort(programsByAccess.begin(), programsByAccess.end(),
              [](const auto &a, const auto &b) { return a.second < b.second; });

    size_t toRemove = programs.size() - targetProgramCount;
    for (size_t i = 0; i < toRemove && i < programsByAccess.size(); ++i) {
      const std::string &hash = programsByAccess[i].first;
      auto it = programs.find(hash);
      if (it != programs.end()) {
        cleanupProgram(it->second);
        programs.erase(it);
        programAccessOrder.erase(hash);
      }
    }
    LOGV("Cleaned up %zu programs", toRemove);
  }
}

// === 私有方法实现 ===

void ShaderCache::resolveIncludes() {
  LOGV("Resolving includes in %zu shader sources", sources.size());

  bool changed = true;
  int iterations = 0;
  const int maxIterations = 10; // 防止无限循环

  while (changed && iterations < maxIterations) {
    changed = false;
    iterations++;

    for (auto &[key, src]: sources) {
      currentlyResolving.clear();
      if (resolveIncludesRecursive(key, src)) {
        changed = true;
      }
    }
  }

  if (iterations >= maxIterations) {
    LOGW(
        "Include resolution stopped after %d iterations (possible circular includes)",
        maxIterations);
  } else {
    LOGV("Include resolution completed in %d iterations", iterations);
  }

  resolvedIncludes.clear();
  currentlyResolving.clear();
}

bool ShaderCache::resolveIncludesRecursive(const std::string &currentShader,
                                           std::string &source) {
  if (currentlyResolving.count(currentShader)) {
    LOGE("Circular include detected in shader: %s", currentShader.c_str());
    return false;
  }

  currentlyResolving.insert(currentShader);
  bool changed = false;

  for (const auto &[includeName, includeSource]: sources) {
    if (includeName == currentShader) continue; // 跳过自己

    std::string pattern = "#include <" + includeName + ">";
    size_t pos = source.find(pattern);

    if (pos != std::string::npos) {
      // 替换第一个出现的位置
      source.replace(pos, pattern.length(), includeSource);
      changed = true;

      // 移除其他出现的位置
      while ((pos = source.find(pattern)) != std::string::npos) {
        source.replace(pos, pattern.length(), "");
      }

      LOGV("Resolved include %s in shader %s",
           includeName.c_str(),
           currentShader.c_str());
    }
  }

  currentlyResolving.erase(currentShader);
  return changed;
}

size_t ShaderCache::stringHash(const std::string &str) const {
  return std::hash<std::string>{}(str);
}

bool ShaderCache::isVertexShader(const std::string &identifier) const {
  return identifier.find(".vert") != std::string::npos;
}

bool ShaderCache::isFragmentShader(const std::string &identifier) const {
  return identifier.find(".frag") != std::string::npos;
}

std::string
ShaderCache::generateDefinesString(const DefinesList &defines) const {
  std::string result = "#version 300 es\n";
  for (const auto &define: defines) {
    result += "#define " + define + "\n";
  }
  return result;
}

GLuint ShaderCache::compileShader(const std::string &identifier,
                                  bool isVertex,
                                  const std::string &source) {
  if (!gl) {
    LOGE("Invalid WebGL context during shader compilation");
    return 0;
  }

  GLenum shaderType = isVertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
  GLuint shader = glCreateShader(shaderType);

  if (shader == 0) {
    LOGE("Failed to create shader object for: %s", identifier.c_str());
    return 0;
  }

  const char *sourcePtr = source.c_str();
  glShaderSource(shader, 1, &sourcePtr, nullptr);
  glCompileShader(shader);

  if (!checkShaderCompilation(shader, identifier)) {

    glDeleteShader(shader);
    return 0;
  }

  LOGV("Successfully compiled %s shader: %s",
       isVertex ? "vertex" : "fragment", identifier.c_str());
  return shader;
}

GLuint ShaderCache::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
  if (!gl) {
    LOGE("Invalid WebGL context during program linking");
    return 0;
  }

  GLuint program = glCreateProgram();
  if (program == 0) {
    LOGE("Failed to create shader program");
    return 0;
  }

  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  if (!checkProgramLinking(program)) {
    glDeleteProgram(program);
    return 0;
  }

  // 链接成功后，可以分离着色器（但不删除，因为缓存中还需要）
  glDetachShader(program, vertexShader);
  glDetachShader(program, fragmentShader);

  LOGV("Successfully linked shader program: %u", program);
  return program;
}

bool ShaderCache::checkShaderCompilation(GLuint shader,
                                         const std::string &identifier) const {
  GLint compileStatus;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

  if (compileStatus != GL_TRUE) {
    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength > 0) {
      std::vector<char> log(logLength);
      glGetShaderInfoLog(shader, logLength, nullptr, log.data());
      LOGE("Shader compilation error for %s: %s",
           identifier.c_str(),
           log.data());
    } else {
      LOGE("Shader compilation failed for %s (no error log available)",
           identifier.c_str());
    }
    return false;
  }

  return true;
}

bool ShaderCache::checkProgramLinking(GLuint program) const {
  GLint linkStatus;
  glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

  if (linkStatus != GL_TRUE) {
    GLint logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength > 0) {
      std::vector<char> log(logLength);
      glGetProgramInfoLog(program, logLength, nullptr, log.data());
      LOGE("Program linking error: %s", log.data());
    } else {
      LOGE("Program linking failed (no error log available)");
    }
    return false;
  }

  return true;
}

void ShaderCache::cleanupShader(GLuint shader) {
  if (shader != 0 && gl) {
    glDeleteShader(shader);
    checkGLError("cleanup shader");
  }
}

void ShaderCache::cleanupProgram(std::shared_ptr<GltfShader> program) {
  if (program) {
    program->destroy();
  }
}

std::string
ShaderCache::generateProgramHash(size_t vertexHash, size_t fragmentHash) const {
  return std::to_string(vertexHash) + "," + std::to_string(fragmentHash);
}

bool ShaderCache::checkGLError(const std::string &operation) const {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    const char *errorString = nullptr;

    switch (error) {
      case GL_INVALID_ENUM:
        errorString = "GL_INVALID_ENUM";
        break;
      case GL_INVALID_VALUE:
        errorString = "GL_INVALID_VALUE";
        break;
      case GL_INVALID_OPERATION:
        errorString = "GL_INVALID_OPERATION";
        break;
      case GL_OUT_OF_MEMORY:
        errorString = "GL_OUT_OF_MEMORY";
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;
      default:
        errorString = "UNKNOWN_ERROR";
        break;
    }

    LOGE("OpenGL error in %s: %s (0x%x)",
         operation.c_str(),
         errorString,
         error);
    return false;
  }
  return true;
}

} // namespace digitalhumans
