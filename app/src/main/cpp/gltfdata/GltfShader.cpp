//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfShader.h"

#include <android/log.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "gtc/type_ptr.hpp"
#include <utility>
#include <utility>
#include"../utils/LogUtils.h"
#include "UniformStruct.h"

#include <variant>
#include "GltfLight.h"


namespace digitalhumans {

GltfShader::GltfShader(GLuint shaderProgram,
                       std::string shaderHash,
                       std::shared_ptr<GltfOpenGLContext> webgl)
    : program(shaderProgram), hash(std::move(shaderHash)), uniforms(),
      attributes(), unknownUniforms(),
      unknownAttributes(), reportedUnknownUniforms(),
      reportedUnknownAttributes(), gl(std::move(webgl)),
      uniformUpdateCount(0), attributeQueryCount(0) {
  if (program != 0 && gl) {
    initializeUniforms();
    initializeAttributes();
  } else {
    LOGE("Invalid shader program or WebGL context");
  }
}

GltfShader::~GltfShader() {
  destroy();
}

GltfShader::GltfShader(GltfShader &&other) noexcept
    : program(other.program), hash(std::move(other.hash)),
      uniforms(std::move(other.uniforms)),
      attributes(std::move(other.attributes)),
      unknownUniforms(std::move(other.unknownUniforms)),
      unknownAttributes(std::move(other.unknownAttributes)),
      reportedUnknownUniforms(std::move(other.reportedUnknownUniforms)),
      reportedUnknownAttributes(std::move(other.reportedUnknownAttributes)),
      gl(std::move(other.gl)),
      uniformUpdateCount(other.uniformUpdateCount),
      attributeQueryCount(other.attributeQueryCount) {
  other.program = 0;
  other.uniformUpdateCount = 0;
  other.attributeQueryCount = 0;
}

GltfShader &GltfShader::operator=(GltfShader &&other) noexcept {
  if (this != &other) {
    destroy();

    program = other.program;
    hash = std::move(other.hash);
    uniforms = std::move(other.uniforms);
    attributes = std::move(other.attributes);
    unknownUniforms = std::move(other.unknownUniforms);
    unknownAttributes = std::move(other.unknownAttributes);
    reportedUnknownUniforms = std::move(other.reportedUnknownUniforms);
    reportedUnknownAttributes = std::move(other.reportedUnknownAttributes);
    gl = std::move(other.gl);
    uniformUpdateCount = other.uniformUpdateCount;
    attributeQueryCount = other.attributeQueryCount;

    other.program = 0;
    other.uniformUpdateCount = 0;
    other.attributeQueryCount = 0;
  }
  return *this;
}

void GltfShader::destroy() {
  if (program != 0) {
    if (gl) {
      glDeleteProgram(program);
      checkGLError("delete shader program");
    }
    program = 0;
  }
}

void GltfShader::use() const {
  if (program != 0) {
    glUseProgram(program);
    checkGLError("use shader program");
  }
}

GLint GltfShader::getAttributeLocation(const std::string &name) {
  attributeQueryCount++;

  auto it = attributes.find(name);
  if (it == attributes.end()) {
    recordUnknownAttribute(name);
    return -1;
  }
  return it->second;
}

GLint GltfShader::getUniformLocation(const std::string &name) {
  auto it = uniforms.find(name);
  if (it == uniforms.end()) {
    GLint tt = glGetUniformLocation(getProgram(), name.c_str());
    if (tt == -1) {
      recordUnknownUniform(name);
      return -1;
    }

    return tt;
  }
  return it->second.location;
}

void GltfShader::updateUniform(const std::string &objectName,
                               const UniformValue &object,
                               bool log) {
  try {
    std::visit([this, &objectName, log](const auto &value) {
      using T = std::decay_t<decltype(value)>;

      if constexpr (std::is_same_v<T, std::shared_ptr<UniformStruct>>) {
        if (value) {
          updateUniformStruct(objectName, value, log);
        } else if (log) {
          LOGI("update uniform: object undefined");
        }
      } else if constexpr (std::is_same_v<T,
                                          std::shared_ptr<std::vector<
                                              UniformLight>>>) {
        if (value && !value->empty()) {
          updateLightArray(objectName, value, log);
        } else if (log) {
          LOGI("update uniform: light array undefined or empty");
        }
      } else if constexpr (std::is_same_v<T, std::vector<UniformValue>>) {
        updateUniformArray(objectName, value, log);
      } else {
        updateUniformValue(objectName, UniformValue(value), log);
      }
    }, object);
  } catch (const std::exception &e) {
    LOGE("Exception in updateUniform for %s: %s", objectName.c_str(), e.what());
  }
}

void GltfShader::updateLightArray(const std::string &objectName,
                                  const std::shared_ptr<std::vector<UniformLight>> &lights,
                                  bool log) {
  if (!lights || lights->empty()) {
    if (log) LOGI("Light array is empty");
    return;
  }



  // 设置每个光源的uniform
  for (size_t i = 0; i < lights->size(); i++) {
    const UniformLight &light = (*lights)[i];
    std::string prefix = objectName + "[" + std::to_string(i) + "].";

    updateUniformValue(prefix + "direction", light.direction);
    updateUniformValue(prefix + "range", light.range);           // ✅ 新增
    updateUniformValue(prefix + "color", light.color);
    updateUniformValue(prefix + "intensity", light.intensity);
    updateUniformValue(prefix + "position", light.position);
    updateUniformValue(prefix + "innerConeCos", light.innerConeCos);
    updateUniformValue(prefix + "outerConeCos", light.outerConeCos);
    updateUniformValue(prefix + "type", 0);

  }
}

void
GltfShader::updateUniformArray(const std::string &arrayName,
                               const std::vector<UniformValue> &array,
                               bool log) {
  if (array.empty()) {
    if (log) {
      LOGW("Empty array for uniform: %s", arrayName.c_str());
    }
    return;
  }

  try {
    // 检查第一个元素的类型
    std::visit([this, &arrayName, &array, log](const auto &firstValue) {
      using T = std::decay_t<decltype(firstValue)>;

      if constexpr (std::is_same_v<T, std::shared_ptr<UniformStruct>>) {
        // 结构体数组
        for (size_t i = 0; i < array.size(); ++i) {
          std::string uniformName = arrayName + "[" + std::to_string(i) + "]";
          updateUniform(uniformName, array[i], log);
        }
      } else {
        // 基本类型数组
        std::string uniformName = arrayName + "[0]";
        auto flat = flattenArray(array);

        if (flat.empty()) {
          LOGE("Failed to flatten uniform array %s", uniformName.c_str());
          return;
        }

        updateUniformValue(uniformName, std::vector<float>(flat), log);
      }
    }, array[0]);
  } catch (const std::exception &e) {
    LOGE("Exception in updateUniformArray for %s: %s",
         arrayName.c_str(),
         e.what());
  }
}

void
GltfShader::updateUniformStruct(const std::string &structName,
                                std::shared_ptr<UniformStruct> object,
                                bool log) {
  if (!object) {
    if (log) {
      LOGW("Null UniformStruct for: %s", structName.c_str());
    }
    return;
  }

  try {
    const auto &memberNames = object->getMemberNames();
    for (const auto &memberName: memberNames) {
      std::string uniformName = structName + "." + memberName;
      auto memberValue = object->getMemberValue(memberName);
      updateUniform(uniformName, memberValue, log);
    }
  } catch (const std::exception &e) {
    LOGE("Exception in updateUniformStruct for %s: %s",
         structName.c_str(),
         e.what());
  }
}

void GltfShader::updateUniformValue(const std::string &uniformName,
                                    const UniformValue &value,
                                    bool log) {
  auto it = uniforms.find(uniformName);
  if (it == uniforms.end()) {
    LOGE("Unknown uniform: %s", uniformName.c_str());
    recordUnknownUniform(uniformName);
    return;
  }

  const UniformInfo &uniform = it->second;
  uniformUpdateCount++;

  try {
    std::visit([this, &uniform, &uniformName](const auto &val) {
      using T = std::decay_t<decltype(val)>;

      switch (uniform.type) {
        case GL_FLOAT:
          if constexpr (std::is_same_v<T, float>) {
            glUniform1f(uniform.location, val);
          } else if constexpr (std::is_same_v<T, std::vector<float>>) {
            glUniform1fv(uniform.location,
                         static_cast<GLsizei>(val.size()),
                         val.data());
          } else if constexpr (std::is_same_v<T, int>) {
            glUniform1f(uniform.location, static_cast<float>(val));
          }
          break;

        case GL_FLOAT_VEC2:
          if constexpr (std::is_same_v<T, glm::vec2>) {
            glUniform2fv(uniform.location, 1, glm::value_ptr(val));
          } else if constexpr (std::is_same_v<T, std::vector<float>>) {
            if (val.size() >= 2) {
              glUniform2fv(uniform.location, 1, val.data());
            }
          }
          break;

        case GL_FLOAT_VEC3:
          if constexpr (std::is_same_v<T, glm::vec3>) {
            glUniform3fv(uniform.location, 1, glm::value_ptr(val));
          } else if constexpr (std::is_same_v<T, std::vector<float>>) {
            if (val.size() >= 3) {
              glUniform3fv(uniform.location, 1, val.data());
            }
          }
          break;

        case GL_FLOAT_VEC4:
          if constexpr (std::is_same_v<T, glm::vec4>) {
            glUniform4fv(uniform.location, 1, glm::value_ptr(val));
          } else if constexpr (std::is_same_v<T, std::vector<float>>) {
            if (val.size() >= 4) {
              glUniform4fv(uniform.location, 1, val.data());
            }
          }
          break;

        case GL_INT:
          if constexpr (std::is_same_v<T, int>) {
            glUniform1i(uniform.location, val);
          } else if constexpr (std::is_same_v<T, bool>) {
            glUniform1i(uniform.location, val ? 1 : 0);
          } else if constexpr (std::is_same_v<T, std::vector<int>>) {
            glUniform1iv(uniform.location,
                         static_cast<GLsizei>(val.size()),
                         val.data());
          }
          break;

        case GL_INT_VEC2:
          if constexpr (std::is_same_v<T, glm::ivec2>) {
            glUniform2iv(uniform.location, 1, glm::value_ptr(val));
          } else if constexpr (std::is_same_v<T, std::vector<int>>) {
            if (val.size() >= 2) {
              glUniform2iv(uniform.location, 1, val.data());
            }
          }
          break;

        case GL_INT_VEC3:
          if constexpr (std::is_same_v<T, glm::ivec3>) {
            glUniform3iv(uniform.location, 1, glm::value_ptr(val));
          } else if constexpr (std::is_same_v<T, std::vector<int>>) {
            if (val.size() >= 3) {
              glUniform3iv(uniform.location, 1, val.data());
            }
          }
          break;

        case GL_INT_VEC4:
          if constexpr (std::is_same_v<T, glm::ivec4>) {
            glUniform4iv(uniform.location, 1, glm::value_ptr(val));
          } else if constexpr (std::is_same_v<T, std::vector<int>>) {
            if (val.size() >= 4) {
              glUniform4iv(uniform.location, 1, val.data());
            }
          }
          break;

        case GL_FLOAT_MAT2:
          if constexpr (std::is_same_v<T, glm::mat2>) {
            glUniformMatrix2fv(uniform.location,
                               1,
                               GL_FALSE,
                               glm::value_ptr(val));
          } else if constexpr (std::is_same_v<T, std::vector<float>>) {
            if (val.size() >= 4) {
              glUniformMatrix2fv(uniform.location, 1, GL_FALSE, val.data());
            }
          }
          break;

        case GL_FLOAT_MAT3:
          if constexpr (std::is_same_v<T, glm::mat3>) {
            glUniformMatrix3fv(uniform.location,
                               1,
                               GL_FALSE,
                               glm::value_ptr(val));
          } else if constexpr (std::is_same_v<T, std::vector<float>>) {
            if (val.size() >= 9) {
              glUniformMatrix3fv(uniform.location, 1, GL_FALSE, val.data());
            }
          } else if constexpr (std::is_same_v<T, std::vector<glm::mat3>>) {
            if (!val.empty()) {
              glUniformMatrix3fv(uniform.location,
                                 static_cast<GLsizei>(val.size()),
                                 GL_FALSE,
                                 glm::value_ptr(val[0]));
            }
          }
          break;

        case GL_FLOAT_MAT4:
          if constexpr (std::is_same_v<T, glm::mat4>) {
            glUniformMatrix4fv(uniform.location,
                               1,
                               GL_FALSE,
                               glm::value_ptr(val));
          } else if constexpr (std::is_same_v<T, std::vector<float>>) {
            if (val.size() >= 16) {
              glUniformMatrix4fv(uniform.location, 1, GL_FALSE, val.data());
            }
          } else if constexpr (std::is_same_v<T, std::vector<glm::mat4>>) {
            if (!val.empty()) {
              glUniformMatrix4fv(uniform.location,
                                 static_cast<GLsizei>(val.size()),
                                 GL_FALSE,
                                 glm::value_ptr(val[0]));
            }
          }
          break;

          // 纹理采样器类型
        case GL_SAMPLER_2D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_2D_ARRAY:
          if constexpr (std::is_same_v<T, int>) {
            glUniform1i(uniform.location, val);
          }
          break;

        default:
          LOGW("Unsupported uniform type: %s for %s",
               getGLTypeName(uniform.type).c_str(), uniformName.c_str());
          break;
      }
    }, value);

    //   checkGLError("updateUniformValue: " + uniformName);

  } catch (const std::exception &e) {
    LOGE("Exception updating uniform %s: %s", uniformName.c_str(), e.what());
  }
}
// === 便利的设置方法实现 ===

void GltfShader::setUniform(const std::string &name, float value) {
  updateUniformValue(name, UniformValue(value));
}

void GltfShader::setUniform(const std::string &name, int value) {
  updateUniformValue(name, UniformValue(value));
}

void GltfShader::setUniform(const std::string &name, bool value) {
  updateUniformValue(name, UniformValue(value ? 1 : 0));
}

void GltfShader::setUniform(const std::string &name, const glm::vec2 &value) {
  updateUniformValue(name, UniformValue(value));
}

void GltfShader::setUniform(const std::string &name, const glm::vec3 &value) {
  updateUniformValue(name, UniformValue(value));
}

void GltfShader::setUniform(const std::string &name, const glm::vec4 &value) {
  updateUniformValue(name, UniformValue(value));
}

void GltfShader::setUniform(const std::string &name, const glm::mat3 &value) {
  updateUniformValue(name, UniformValue(value));
}

void GltfShader::setUniform(const std::string &name, const glm::mat4 &value) {
  updateUniformValue(name, UniformValue(value));
}

void GltfShader::setUniform(const std::string &name,
                            const std::vector<float> &values) {
  updateUniformValue(name, UniformValue(values));
}

void GltfShader::setUniform(const std::string &name,
                            const std::vector<int> &values) {
  updateUniformValue(name, UniformValue(values));
}

// === 其他公共方法实现 ===

void GltfShader::clearUnknownItems() {
  unknownUniforms.clear();
  unknownAttributes.clear();
  reportedUnknownUniforms.clear();
  reportedUnknownAttributes.clear();
}

std::string GltfShader::getInfoString() const {
  std::ostringstream oss;
  oss << "Shader Program " << program << " (Hash: " << hash << "):\n";
  oss << "  Uniforms: " << uniforms.size() << "\n";
  oss << "  Attributes: " << attributes.size() << "\n";
  oss << "  Unknown Uniforms: " << unknownUniforms.size() << "\n";
  oss << "  Unknown Attributes: " << unknownAttributes.size() << "\n";
  oss << "  Update Count: " << uniformUpdateCount << "\n";
  oss << "  Query Count: " << attributeQueryCount;
  return oss.str();
}

bool GltfShader::validate() const {
  if (program == 0) {
    return false;
  }

  glValidateProgram(program);
  GLint validateStatus;
  glGetProgramiv(program, GL_VALIDATE_STATUS, &validateStatus);

  if (validateStatus != GL_TRUE) {
    GLint logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength > 0) {
      std::vector<char> log(logLength);
      glGetProgramInfoLog(program, logLength, nullptr, log.data());
      LOGE("Shader validation error: %s", log.data());
    }
    return false;
  }

  return true;
}

bool GltfShader::checkGLError(const std::string &operation) const {
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

void GltfShader::resetStatistics() {
  uniformUpdateCount = 0;
  attributeQueryCount = 0;
}

// === 私有方法实现 ===

void GltfShader::initializeUniforms() {
  if (!gl) {
    LOGE("Invalid WebGL context in initializeUniforms");
    return;
  }

  GLint uniformCount;
  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);

  for (GLint i = 0; i < uniformCount; ++i) {
    GLchar nameBuffer[256];
    GLsizei nameLength;
    GLint size;
    GLenum type;

    glGetActiveUniform(program,
                       i,
                       sizeof(nameBuffer),
                       &nameLength,
                       &size,
                       &type,
                       nameBuffer);

    std::string name(nameBuffer, nameLength);
    GLint location = glGetUniformLocation(program, name.c_str());

    if (location != -1) {
      uniforms[name] = UniformInfo(type, location);
    }
  }

  LOGI("Initialized %d uniforms for shader %s",
       static_cast<int>(uniforms.size()),
       hash.c_str());
  checkGLError("initialize uniforms");
}

void GltfShader::initializeAttributes() {
  if (!gl) {
    LOGE("Invalid WebGL context in initializeAttributes");
    return;
  }

  GLint attributeCount;
  glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &attributeCount);

  for (GLint i = 0; i < attributeCount; ++i) {
    GLchar nameBuffer[256];
    GLsizei nameLength;
    GLint size;
    GLenum type;

    glGetActiveAttrib(program,
                      i,
                      sizeof(nameBuffer),
                      &nameLength,
                      &size,
                      &type,
                      nameBuffer);

    std::string name(nameBuffer, nameLength);
    GLint location = glGetAttribLocation(program, name.c_str());

    if (location != -1) {
      attributes[name] = location;
    }
  }
  checkGLError("initialize attributes");
}

std::vector<float>
GltfShader::flattenArray(const std::vector<UniformValue> &array) const {
  std::vector<float> flat;

  for (const auto &value: array) {
    std::visit([&flat](const auto &val) {
      using T = std::decay_t<decltype(val)>;

      if constexpr (std::is_same_v<T, float>) {
        flat.push_back(val);
      } else if constexpr (std::is_same_v<T, int>) {
        flat.push_back(static_cast<float>(val));
      } else if constexpr (std::is_same_v<T, glm::vec2>) {
        flat.push_back(val.x);
        flat.push_back(val.y);
      } else if constexpr (std::is_same_v<T, glm::vec3>) {
        flat.push_back(val.x);
        flat.push_back(val.y);
        flat.push_back(val.z);
      } else if constexpr (std::is_same_v<T, glm::vec4>) {
        flat.push_back(val.x);
        flat.push_back(val.y);
        flat.push_back(val.z);
        flat.push_back(val.w);
      } else if constexpr (std::is_same_v<T, glm::mat3>) {
        const float *ptr = glm::value_ptr(val);
        for (int i = 0; i < 9; ++i) {
          flat.push_back(ptr[i]);
        }
      } else if constexpr (std::is_same_v<T, glm::mat4>) {
        const float *ptr = glm::value_ptr(val);
        for (int i = 0; i < 16; ++i) {
          flat.push_back(ptr[i]);
        }
      } else if constexpr (std::is_same_v<T, std::vector<float>>) {
        flat.insert(flat.end(), val.begin(), val.end());
      } else if constexpr (std::is_same_v<T, std::vector<int>>) {
        for (int intVal: val) {
          flat.push_back(static_cast<float>(intVal));
        }
      }
    }, value);
  }

  return flat;
}

void GltfShader::recordUnknownUniform(const std::string &name) {
  if (reportedUnknownUniforms.find(name) == reportedUnknownUniforms.end()) {
    unknownUniforms.push_back(name);
    reportedUnknownUniforms.insert(name);
    LOGE("Uniform '%s' does not exist in shader %s",
         name.c_str(),
         hash.c_str());
  }
}

void GltfShader::recordUnknownAttribute(const std::string &name) {
  if (reportedUnknownAttributes.find(name) == reportedUnknownAttributes.end()) {
    unknownAttributes.push_back(name);
    reportedUnknownAttributes.insert(name);
    LOGE("Attribute '%s' does not exist in shader %s",
         name.c_str(),
         hash.c_str());
  }
}

std::string GltfShader::getGLTypeName(GLenum type) const {
  switch (type) {
    case GL_FLOAT:
      return "GL_FLOAT";
    case GL_FLOAT_VEC2:
      return "GL_FLOAT_VEC2";
    case GL_FLOAT_VEC3:
      return "GL_FLOAT_VEC3";
    case GL_FLOAT_VEC4:
      return "GL_FLOAT_VEC4";
    case GL_INT:
      return "GL_INT";
    case GL_INT_VEC2:
      return "GL_INT_VEC2";
    case GL_INT_VEC3:
      return "GL_INT_VEC3";
    case GL_INT_VEC4:
      return "GL_INT_VEC4";
    case GL_BOOL:
      return "GL_BOOL";
    case GL_BOOL_VEC2:
      return "GL_BOOL_VEC2";
    case GL_BOOL_VEC3:
      return "GL_BOOL_VEC3";
    case GL_BOOL_VEC4:
      return "GL_BOOL_VEC4";
    case GL_FLOAT_MAT2:
      return "GL_FLOAT_MAT2";
    case GL_FLOAT_MAT3:
      return "GL_FLOAT_MAT3";
    case GL_FLOAT_MAT4:
      return "GL_FLOAT_MAT4";
    case GL_SAMPLER_2D:
      return "GL_SAMPLER_2D";
    case GL_SAMPLER_CUBE:
      return "GL_SAMPLER_CUBE";
    case GL_SAMPLER_2D_ARRAY:
      return "GL_SAMPLER_2D_ARRAY";
    default:
      return "UNKNOWN_TYPE";
  }
}

} // namespace digitalhumans