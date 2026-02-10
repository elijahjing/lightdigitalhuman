//
// Created by vincentsyan on 2025/8/19.
//

#include "GltfBuffer.h"
namespace digitalhumans {

GltfBuffer::GltfBuffer()
    : GltfObject(), uri(std::nullopt), byteLength(std::nullopt),
      name(std::nullopt) {
  // 构造函数体为空，所有初始化在初始化列表中完成
}


void GltfBuffer::clear() {
  uri.reset();
  byteLength.reset();
  name.reset();
  buffer.clear();
}

bool GltfBuffer::validate() const {
  // 检查是否有URI或缓冲区数据
  if (uri.has_value() && !uri->empty()) {
    return true;
  }

  if (!buffer.empty()) {
    // 如果有byteLength，检查是否匹配
    if (byteLength.has_value()) {
      return buffer.size() == byteLength.value();
    }
    return true;
  }

  return false;
}

} // namespace digitalhumans