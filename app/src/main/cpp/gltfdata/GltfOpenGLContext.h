//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFOPENGLCONTEXT_H
#define LIGHTDIGITALHUMAN_GLTFOPENGLCONTEXT_H

#include <memory>
#include <string>
#include <vector>

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

namespace digitalhumans {
class GltfTexture;

class GltfSampler;

class Gltf;

class GltfTextureInfo;

class GltfAccessor;

class GltfImage;

/**
* @brief Android OpenGL ES上下文封装类
* 提供glTF渲染所需的OpenGL ES功能封装
*/
class GltfOpenGLContext {
 public:
  /**
   * @brief 构造函数
   */
  GltfOpenGLContext();

  /**
   * @brief 析构函数
   */
  ~GltfOpenGLContext() = default;

  /**
   * @brief 初始化OpenGL ES上下文
   * @return 是否初始化成功
   */
  bool initialize();

  /**
   * @brief 加载OpenGL ES扩展
   */
  void loadOpenGLExtensions();

  /**
   * @brief 设置纹理到指定的纹理槽
   * @param uniformLocation uniform位置
   * @param gltf glTF对象
   * @param textureInfo 纹理信息
   * @param texSlot 纹理槽索引
   * @return 是否设置成功
   */
  bool setTexture(GLint uniformLocation,
                  std::shared_ptr<Gltf> gltf,
                  std::shared_ptr<GltfTextureInfo> textureInfo,
                  GLint texSlot);

  /**
   * @brief 设置索引缓冲区
   * @param gltf glTF对象
   * @param accessorIndex 访问器索引
   * @return 是否设置成功
   */
  bool setIndices(std::shared_ptr<Gltf> gltf, int accessorIndex);

  /**
   * @brief 启用顶点属性
   * @param gltf glTF对象
   * @param attributeLocation 属性位置
   * @param accessor 访问器对象
   * @return 是否启用成功
   */
  bool enableAttribute(std::shared_ptr<Gltf> gltf,
                       GLint attributeLocation,
                       std::shared_ptr<GltfAccessor> accessor);

  /**
   * @brief 编译着色器
   * @param shaderIdentifier 着色器标识符
   * @param isVertex 是否为顶点着色器
   * @param shaderSource 着色器源代码
   * @return 编译后的着色器对象
   */
  GLuint compileShader(const std::string &shaderIdentifier,
                       bool isVertex,
                       const std::string &shaderSource);

  /**
   * @brief 链接着色器程序
   * @param vertexShader 顶点着色器
   * @param fragmentShader 片段着色器
   * @return 链接后的程序对象
   */
  GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);

  /**
   * @brief 设置采样器参数
   * @param samplerObj glTF采样器对象
   * @param textureType 纹理类型
   * @param generateMipmaps 是否生成mipmap
   */
  void setSampler(std::shared_ptr<GltfSampler> samplerObj,
                  GLenum textureType,
                  bool generateMipmaps);

  // === 状态查询方法 ===
  bool
  supportsAnisotropicFiltering() const { return supportsEXTTextureFilterAnisotropic; }

  GLfloat getMaxAnisotropy() const { return maxAnisotropy; }

  // === OpenGL ES包装方法 ===
  GLenum createTexture();

  GLuint createBuffer();

  void deleteTexture(GLuint texture);

  void deleteBuffer(GLuint buffer);


  // 绑定纹理
  void bindTexture(GLenum target, GLuint texture);

  // 设置纹理参数
  void texParameteri(GLenum target, GLenum pname, GLint param);

  void texParameterf(GLenum target, GLenum pname, GLfloat param);

  void checkGLError(const char *operation);

  // 纹理数据上传方法
  void texImage2D(GLenum target, GLint level, GLint internalformat,
                  GLsizei width, GLsizei height, GLint border,
                  GLenum format, GLenum type, const void *data);

  // 重载版本，支持不同数据类型
  void texImage2D(GLenum target, GLint level, GLint internalformat,
                  GLsizei width, GLsizei height, GLint border,
                  GLenum format, GLenum type, const std::vector<float> &data);

  void texImage2D(GLenum target, GLint level, GLint internalformat,
                  GLsizei width, GLsizei height, GLint border,
                  GLenum format, GLenum type, const std::vector<uint8_t> &data);


 private:
  std::string parseShaderError(const std::string &shaderIdentifier,
                               const std::string &shaderSource,
                               const std::string &infoLog);

  void uploadImageToTexture(std::shared_ptr<GltfTexture> gltfTexture,
                            std::shared_ptr<GltfImage> image);

 private:
  bool supportsEXTTextureFilterAnisotropic;
  GLenum anisotropyParameter;
  GLfloat maxAnisotropy;
  bool initialized;
};

} // namespace digitalhumans


#endif //LIGHTDIGITALHUMAN_GLTFOPENGLCONTEXT_H
