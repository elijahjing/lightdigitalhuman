//
// Created by vincentsyan on 2025/8/19.
//

#ifndef LIGHTDIGITALHUMAN_GLTFBUFFER_H
#define LIGHTDIGITALHUMAN_GLTFBUFFER_H


// GltfBuffer.h

#include "GltfObject.h"
#include <vector>
#include <string>
#include <optional>
#include <cstdint>

namespace digitalhumans {

/**
 * @brief glTF缓冲区类
 * 表示二进制数据的缓冲区
 */
class GltfBuffer: public GltfObject {
 public:
  /**
   * @brief 默认构造函数
   */
  GltfBuffer();

  /**
   * @brief 虚析构函数
   */
  ~GltfBuffer() override = default;

  // === Getter方法 ===
  const std::optional<std::string> &getUri() const { return uri; }
  const std::optional<size_t> &getByteLength() const { return byteLength; }
  const std::optional<std::string> &getName() const { return name; }
  const std::vector<uint8_t> &getBuffer() const { return buffer; }

  // === Setter方法 ===
  void setUri(const std::string &uri) { this->uri = uri; }
  void setByteLength(size_t byteLength) { this->byteLength = byteLength; }
  void setName(const std::string &name) { this->name = name; }
  void setBuffer(const std::vector<unsigned char> &buffer) {
    this->buffer = buffer;
    this->byteLength = buffer.size();
  }
  void setBuffer(std::vector<unsigned char> &&buffer) {
    this->buffer = std::move(buffer);
    this->byteLength = this->buffer.size();
  }

  // === 工具方法 ===
  /**
   * @brief 检查缓冲区是否有效
   */
  /**
* @brief 验证数据有效性
* @return true如果数据有效
*/
  bool validate() const override;

  /**
   * @brief 清空缓冲区数据
   */
  void clear();

  /**
   * @brief 获取实际数据大小
   */
  size_t getActualSize() const { return buffer.size(); }


 public:
  // === glTF标准属性 ===
  std::optional<std::string> uri;         ///< 缓冲区URI（外部文件路径或data URI）
  std::optional<size_t> byteLength;       ///< 缓冲区字节长度
  std::optional<std::string> name;        ///< 缓冲区名称

  // === 非glTF标准属性 ===
  std::vector<uint8_t> buffer;            ///< 原始二进制数据


};

} // namespace digitalhumans


#endif //LIGHTDIGITALHUMAN_GLTFBUFFER_H
