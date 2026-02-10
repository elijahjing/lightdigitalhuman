//
// Created by vincentsyan on 2025/8/19.
//

#include "GltfBufferView.h"
namespace digitalhumans {

// 静态成员定义
const std::vector<std::string> GltfBufferView::animatedProperties = {};

GltfBufferView::GltfBufferView()
    : GltfObject(), buffer(std::nullopt), byteOffset(0),
      byteLength(std::nullopt), byteStride(0), target(std::nullopt),
      name(std::nullopt) {
  // 构造函数体为空，所有初始化在初始化列表中完成
}

bool GltfBufferView::isValid() const {
  // 必须有有效的缓冲区索引
  if (!buffer.has_value() || buffer.value() < 0) {
    return false;
  }

  // 必须有字节长度
  if (!byteLength.has_value() || byteLength.value() == 0) {
    return false;
  }

  return true;
}

void GltfBufferView::clear() {
  buffer.reset();
  byteOffset = 0;
  byteLength.reset();
  byteStride = 0;
  target.reset();
  name.reset();
}

} // namespace digitalhumans