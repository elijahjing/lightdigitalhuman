//
// Created by vincentsyan on 2025/8/19.
//

#ifndef LIGHTDIGITALHUMAN_GLTFBUFFERVIEW_H
#define LIGHTDIGITALHUMAN_GLTFBUFFERVIEW_H


#include "GltfObject.h"
#include <vector>
#include <string>
#include <optional>
#include <cstdint>

namespace digitalhumans {

/**
 * @brief glTF缓冲区视图类
 * 定义如何从缓冲区中读取数据的视图
 */
class GltfBufferView: public GltfObject {
 public:
  /**
   * @brief 默认构造函数
   */
  GltfBufferView();

  /**
   * @brief 虚析构函数
   */
  virtual ~GltfBufferView() = default;

  // === Getter方法 ===
  const std::optional<int> &getBuffer() const { return buffer; }
  size_t getByteOffset() const { return byteOffset; }
  const std::optional<size_t> &getByteLength() const { return byteLength; }
  size_t getByteStride() const { return byteStride; }
  const std::optional<int> &getTarget() const { return target; }
  const std::optional<std::string> &getName() const { return name; }

  // === Setter方法 ===
  void setBuffer(int buffer) { this->buffer = buffer; }
  void setByteOffset(size_t byteOffset) { this->byteOffset = byteOffset; }
  void setByteLength(size_t byteLength) { this->byteLength = byteLength; }
  void setByteStride(size_t byteStride) { this->byteStride = byteStride; }
  void setTarget(int target) { this->target = target; }
  void setName(const std::string &name) { this->name = name; }

  // === 重置方法 ===
  void resetBuffer() { this->buffer.reset(); }
  void resetByteLength() { this->byteLength.reset(); }
  void resetTarget() { this->target.reset(); }
  void resetName() { this->name.reset(); }

  // === 工具方法 ===
  /**
   * @brief 检查缓冲区视图是否有效
   */
  bool isValid() const;

  /**
   * @brief 清空所有数据
   */
  void clear();

  /**
   * @brief 检查是否有目标类型
   */
  bool hasTarget() const { return target.has_value(); }

  /**
   * @brief 检查是否有字节步长
   */
  bool hasByteStride() const { return byteStride > 0; }

  // === 常量定义 ===
  /**
   * @brief glTF缓冲区目标类型常量
   */
  enum Target {
    ARRAY_BUFFER = 34962,         // GL_ARRAY_BUFFER - 顶点数据
    ELEMENT_ARRAY_BUFFER = 34963  // GL_ELEMENT_ARRAY_BUFFER - 索引数据
  };



 public:
  // === glTF标准属性 ===
  std::optional<int> buffer;              ///< 缓冲区索引
  size_t byteOffset;                      ///< 字节偏移量（默认为0）
  std::optional<size_t> byteLength;       ///< 字节长度
  size_t byteStride;                      ///< 字节步长（默认为0，表示紧密排列）
  std::optional<int> target;              ///< 目标类型（GL常量）
  std::optional<std::string> name;        ///< 缓冲区视图名称

 private:
  /// 静态动画属性列表
  static const std::vector<std::string> animatedProperties;
};

} // namespace digitalhumans



#endif //LIGHTDIGITALHUMAN_GLTFBUFFERVIEW_H
