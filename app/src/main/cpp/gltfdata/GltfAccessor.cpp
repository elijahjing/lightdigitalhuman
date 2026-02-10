//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfAccessor.h"
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <GLES3/gl3.h>
#include "../utils/LogUtils.h"
#include "Gltf.h"
#include "GltfBufferView.h"

#include "GltfBuffer.h"


namespace digitalhumans {

const std::unordered_map<std::string, AccessorType>
    AccessorTypeUtils::stringToEnum = {
    {"SCALAR", AccessorType::SCALAR},
    {"VEC2", AccessorType::VEC2},
    {"VEC3", AccessorType::VEC3},
    {"VEC4", AccessorType::VEC4},
    {"MAT2", AccessorType::MAT2},
    {"MAT3", AccessorType::MAT3},
    {"MAT4", AccessorType::MAT4}
};

const std::unordered_map<AccessorType, std::string>
    AccessorTypeUtils::enumToString = {
    {AccessorType::SCALAR, "SCALAR"},
    {AccessorType::VEC2, "VEC2"},
    {AccessorType::VEC3, "VEC3"},
    {AccessorType::VEC4, "VEC4"},
    {AccessorType::MAT2, "MAT2"},
    {AccessorType::MAT3, "MAT3"},
    {AccessorType::MAT4, "MAT4"}
};

const std::unordered_map<AccessorType, int>
    AccessorTypeUtils::componentCounts = {
    {AccessorType::SCALAR, 1},
    {AccessorType::VEC2, 2},
    {AccessorType::VEC3, 3},
    {AccessorType::VEC4, 4},
    {AccessorType::MAT2, 4},
    {AccessorType::MAT3, 9},
    {AccessorType::MAT4, 16}
};

AccessorType AccessorTypeUtils::fromString(const std::string &typeStr) {
  auto it = stringToEnum.find(typeStr);
  if (it != stringToEnum.end()) {
    return it->second;
  }
  throw std::invalid_argument("Invalid accessor type: " + typeStr);
}

std::string AccessorTypeUtils::toString(AccessorType type) {
  auto it = enumToString.find(type);
  if (it != enumToString.end()) {
    return it->second;
  }
  throw std::invalid_argument("Invalid accessor type enum");
}

int AccessorTypeUtils::getComponentCount(AccessorType type) {
  auto it = componentCounts.find(type);
  if (it != componentCounts.end()) {
    return it->second;
  }
  return 0;
}

int AccessorTypeUtils::getComponentCount(const std::string &typeStr) {
  try {
    return getComponentCount(fromString(typeStr));
  } catch (const std::exception &) {
    return 0;
  }
}

bool AccessorTypeUtils::isValidType(const std::string &typeStr) {
  return stringToEnum.find(typeStr) != stringToEnum.end();
}

const std::vector<std::string> GltfAccessor::animatedProperties = {};

GltfAccessor::GltfAccessor()
    : GltfObject(), bufferView(std::nullopt), byteOffset(0),
      componentType(std::nullopt), normalized(false),
      count(std::nullopt), type(std::nullopt), sparse(std::nullopt),
      name(std::nullopt), glBuffer(0) {
}

GltfAccessor::GltfAccessor(int bufferViewIndex,
                           int componentType,
                           int count,
                           const std::string &type)
    : GltfObject(), bufferView(bufferViewIndex), byteOffset(0),
      componentType(componentType), normalized(false),
      count(count), type(type), sparse(std::nullopt), name(std::nullopt),
      glBuffer(0) {
  if (!AccessorTypeUtils::isValidType(type)) {
    throw std::invalid_argument("Invalid accessor type: " + type);
  }
  if (!isValidComponentType(componentType)) {
    throw std::invalid_argument(
        "Invalid component type: " + std::to_string(componentType));
  }
}

GltfAccessor::~GltfAccessor() {
  destroy();
}

GltfAccessor::GltfAccessor(GltfAccessor &&other) noexcept
    : GltfObject(std::move(other)), bufferView(other.bufferView),
      byteOffset(other.byteOffset),
      componentType(other.componentType), normalized(other.normalized),
      count(other.count),
      type(std::move(other.type)), max(std::move(other.max)),
      min(std::move(other.min)),
      sparse(std::move(other.sparse)), name(std::move(other.name)),
      glBuffer(other.glBuffer),
      typedView(std::move(other.typedView)),
      filteredView(std::move(other.filteredView)),
      normalizedTypedView(std::move(other.normalizedTypedView)),
      normalizedFilteredView(std::move(other.normalizedFilteredView)),
      typedViewValid(other.typedViewValid),
      filteredViewValid(other.filteredViewValid),
      normalizedTypedViewValid(other.normalizedTypedViewValid),
      normalizedFilteredViewValid(other.normalizedFilteredViewValid) {
  other.glBuffer = 0;
  other.clearCachedViews();
}

GltfAccessor &GltfAccessor::operator=(GltfAccessor &&other) noexcept {
  if (this != &other) {
    destroy();
    GltfObject::operator=(std::move(other));
    bufferView = other.bufferView;
    byteOffset = other.byteOffset;
    componentType = other.componentType;
    normalized = other.normalized;
    count = other.count;
    type = std::move(other.type);
    max = std::move(other.max);
    min = std::move(other.min);
    sparse = std::move(other.sparse);
    name = std::move(other.name);
    glBuffer = other.glBuffer;
    typedView = std::move(other.typedView);
    filteredView = std::move(other.filteredView);
    normalizedTypedView = std::move(other.normalizedTypedView);
    normalizedFilteredView = std::move(other.normalizedFilteredView);
    typedViewValid = other.typedViewValid;
    filteredViewValid = other.filteredViewValid;
    normalizedTypedViewValid = other.normalizedTypedViewValid;
    normalizedFilteredViewValid = other.normalizedFilteredViewValid;
    other.glBuffer = 0;
    other.clearCachedViews();
  }
  return *this;
}


bool GltfAccessor::validate() const {
  if (!GltfObject::validate()) {
    return false;
  }

  return validateAccessor() && validateBounds();
}

void GltfAccessor::setAccessorType(const std::string &accessorType) {
  if (AccessorTypeUtils::isValidType(accessorType)) {
    type = accessorType;
    clearCachedViews();
  } else {
    throw std::invalid_argument("Invalid accessor type: " + accessorType);
  }
}

void GltfAccessor::setAccessorType(AccessorType accessorType) {
  setAccessorType(AccessorTypeUtils::toString(accessorType));
}

std::pair<const void *, size_t> GltfAccessor::getTypedView(const Gltf &gltf) {
  if (typedViewValid) {
    return {typedView.data(), typedView.size()};
  }

  if (bufferView.has_value()) {
    const auto &bufferViews = gltf.getBufferViews();
    const auto &buffers = gltf.getBuffers();

    if (bufferView.value() >= static_cast<int>(bufferViews.size())) {
      LOGE("Invalid bufferView index: %d", bufferView.value());
      return {nullptr, 0};
    }

    const auto &bv = bufferViews[bufferView.value()];
    if (!bv || !bv->getBuffer().has_value()) {
      LOGE("Invalid buffer view");
      return {nullptr, 0};
    }

    int bufferIndex = bv->getBuffer().value();
    if (bufferIndex >= static_cast<int>(buffers.size())) {
      LOGE("Invalid buffer index: %d", bufferIndex);
      return {nullptr, 0};
    }

    const auto &buffer = buffers[bufferIndex];
    if (!buffer) {
      LOGE("Invalid buffer");
      return {nullptr, 0};
    }

    const auto &bufferData = buffer->buffer;
    int totalByteOffset = byteOffset + bv->getByteOffset();

    if (totalByteOffset >= static_cast<int>(bufferData.size())) {
      LOGE("Byte offset exceeds buffer size");
      return {nullptr, 0};
    }

    int componentSize = getComponentSize();
    int componentCount = getComponentCount();
    int byteStride = getByteStride(gltf);

    size_t arrayLength = 0;
    if (byteStride != 0) {
      if (componentSize != 0 && count.has_value()) {
        arrayLength =
            byteStride / componentSize * (count.value() - 1) + componentCount;
      } else {
        LOGW("Invalid component type in accessor '%s'",
             name.value_or("").c_str());
      }
    } else if (count.has_value()) {
      arrayLength = count.value() * componentCount;
    }
    size_t maxLength = (bufferData.size() - totalByteOffset) / componentSize;
    if (arrayLength > maxLength) {
      arrayLength = maxLength;
      LOGW("Count in accessor '%s' is too large",
           name.value_or("").c_str());
    }

    size_t totalBytes = arrayLength * componentSize;
    typedView.resize(totalBytes);
    std::memcpy(typedView.data(),
                bufferData.data() + totalByteOffset,
                totalBytes);

  } else {
    typedView = createEmptyView();
  }

  // 应用稀疏数据
  if (sparse.has_value() && sparse->isValid()) {
    applySparse(gltf, typedView.data(), getComponentSize());
  }

  typedViewValid = true;
  return {typedView.data(), typedView.size()};
}

std::vector<float> GltfAccessor::getNormalizedTypedView(const Gltf &gltf) {
  if (normalizedTypedViewValid) {
    return normalizedTypedView;
  }

  auto [data, size] = getTypedView(gltf);
  if (data && componentType.has_value()) {
    if (normalized) {
      normalizedTypedView = dequantize(data, size, componentType.value());
    } else {
      int componentCount = getComponentCount();
      int elementCount = count.value_or(0);
      normalizedTypedView.resize(elementCount * componentCount);

      switch (componentType.value()) {
        case GL_FLOAT:
          std::memcpy(normalizedTypedView.data(), data, size);
          break;
        case GL_BYTE: {
          const int8_t *src = static_cast<const int8_t *>(data);
          for (size_t i = 0; i < normalizedTypedView.size(); ++i) {
            normalizedTypedView[i] = static_cast<float>(src[i]);
          }
        }
          break;
        case GL_UNSIGNED_BYTE: {
          const uint8_t *src = static_cast<const uint8_t *>(data);
          for (size_t i = 0; i < normalizedTypedView.size(); ++i) {
            normalizedTypedView[i] = static_cast<float>(src[i]);
          }
        }
          break;
        case GL_SHORT: {
          const int16_t *src = static_cast<const int16_t *>(data);
          for (size_t i = 0; i < normalizedTypedView.size(); ++i) {
            normalizedTypedView[i] = static_cast<float>(src[i]);
          }
        }
          break;
        case GL_UNSIGNED_SHORT: {
          const uint16_t *src = static_cast<const uint16_t *>(data);
          for (size_t i = 0; i < normalizedTypedView.size(); ++i) {
            normalizedTypedView[i] = static_cast<float>(src[i]);
          }
        }
          break;
        case GL_UNSIGNED_INT: {
          const uint32_t *src = static_cast<const uint32_t *>(data);
          for (size_t i = 0; i < normalizedTypedView.size(); ++i) {
            normalizedTypedView[i] = static_cast<float>(src[i]);
          }
        }
          break;
      }
    }
  }

  normalizedTypedViewValid = true;
  return normalizedTypedView;
}

std::pair<const void *, size_t>
GltfAccessor::getDeinterlacedView(const Gltf &gltf) {
  if (filteredViewValid) {
    return {filteredView.data(), filteredView.size()};
  }

  if (!bufferView.has_value() || !componentType.has_value() ||
      !count.has_value() || !type.has_value()) {
    filteredView = createEmptyView();
    filteredViewValid = true;
    return {filteredView.data(), filteredView.size()};
  }

  if (!bufferView.has_value() || bufferView.value() < 0) {
    if (!sparse.has_value() || !sparse->isValid()) {
      filteredView = createEmptyView();
      filteredViewValid = true;
      return {filteredView.data(), filteredView.size()};
    }
    int componentSize = getComponentSize();
    int componentCount = getComponentCount();

    if (componentSize <= 0 || componentCount <= 0 || count.value() <= 0) {
      filteredView = createEmptyView();
      filteredViewValid = true;
      return {filteredView.data(), filteredView.size()};
    }

    size_t arrayLength = count.value() * componentCount;
    filteredView.resize(arrayLength * componentSize, 0); // 初始化为 0

    // 应用稀疏数据
    applySparse(gltf, filteredView.data(), componentSize);

    filteredViewValid = true;
    return {filteredView.data(), filteredView.size()};
  }

  const auto &bufferViews = gltf.getBufferViews();
  const auto &buffers = gltf.getBuffers();

  if (bufferView.value() >= static_cast<int>(bufferViews.size())) {
    filteredView = createEmptyView();
    filteredViewValid = true;
    return {filteredView.data(), filteredView.size()};
  }

  const auto &bv = bufferViews[bufferView.value()];
  if (!bv || !bv->getBuffer().has_value()) {
    filteredView = createEmptyView();
    filteredViewValid = true;
    return {filteredView.data(), filteredView.size()};
  }

  int bufferIndex = bv->getBuffer().value();
  if (bufferIndex >= static_cast<int>(buffers.size())) {
    filteredView = createEmptyView();
    filteredViewValid = true;
    return {filteredView.data(), filteredView.size()};
  }

  const auto &buffer = buffers[bufferIndex];
  if (!buffer) {
    filteredView = createEmptyView();
    filteredViewValid = true;
    return {filteredView.data(), filteredView.size()};
  }

  const auto &bufferData = buffer->buffer;
  int componentSize = getComponentSize();
  int componentCount = getComponentCount();
  size_t arrayLength = count.value() * componentCount;

  int stride = bv->getByteStride();
  if (stride == 0) {
    stride = componentCount * componentSize;
  }

  // 分配过滤视图
  filteredView.resize(arrayLength * componentSize);

  // 从buffer中提取数据
  const uint8_t *srcData = bufferData.data() + bv->getByteOffset();
  uint8_t *dstData = filteredView.data();

  for (int i = 0; i < count.value(); ++i) {
    int srcOffset = i * stride + byteOffset;
    int dstOffset = i * componentCount * componentSize;

    if (srcOffset + componentCount * componentSize <=
        static_cast<int>(bufferData.size()) - bv->getByteOffset()) {
      std::memcpy(dstData + dstOffset,
                  srcData + srcOffset,
                  componentCount * componentSize);
    }
  }

  // 应用稀疏数据
  if (sparse.has_value() && sparse->isValid()) {
    applySparse(gltf, filteredView.data(), componentSize);
  }

  filteredViewValid = true;
  return {filteredView.data(), filteredView.size()};
}

std::vector<float>
GltfAccessor::getNormalizedDeinterlacedView(const Gltf &gltf) {
  if (normalizedFilteredViewValid) {
    return normalizedFilteredView;
  }

  auto [data, size] = getDeinterlacedView(gltf);
  if (data && componentType.has_value()) {
    if (normalized) {
      normalizedFilteredView = dequantize(data, size, componentType.value());
    } else {
      // 直接转换逻辑同 getNormalizedTypedView
      int componentCount = getComponentCount();
      int elementCount = count.value_or(0);
      normalizedFilteredView.resize(elementCount * componentCount);

      switch (componentType.value()) {
        case GL_FLOAT:
          std::memcpy(normalizedFilteredView.data(), data, size);
          break;
        case GL_BYTE: {
          const int8_t *src = static_cast<const int8_t *>(data);
          for (size_t i = 0; i < normalizedFilteredView.size(); ++i) {
            normalizedFilteredView[i] = static_cast<float>(src[i]);
          }
        }
          break;
        case GL_UNSIGNED_BYTE: {
          const uint8_t *src = static_cast<const uint8_t *>(data);
          for (size_t i = 0; i < normalizedFilteredView.size(); ++i) {
            normalizedFilteredView[i] = static_cast<float>(src[i]);
          }
        }
          break;
        case GL_SHORT: {
          const int16_t *src = static_cast<const int16_t *>(data);
          for (size_t i = 0; i < normalizedFilteredView.size(); ++i) {
            normalizedFilteredView[i] = static_cast<float>(src[i]);
          }
        }
          break;
        case GL_UNSIGNED_SHORT: {
          const uint16_t *src = static_cast<const uint16_t *>(data);
          for (size_t i = 0; i < normalizedFilteredView.size(); ++i) {
            normalizedFilteredView[i] = static_cast<float>(src[i]);
          }
        }
          break;
        case GL_UNSIGNED_INT: {
          const uint32_t *src = static_cast<const uint32_t *>(data);
          for (size_t i = 0; i < normalizedFilteredView.size(); ++i) {
            normalizedFilteredView[i] = static_cast<float>(src[i]);
          }
        }
          break;
      }

    }
  }

  normalizedFilteredViewValid = true;
  return normalizedFilteredView;
}

int GltfAccessor::getByteStride(const Gltf &gltf) const {
  if (bufferView.has_value()) {
    const auto &bufferViews = gltf.getBufferViews();
    if (bufferView.value() < static_cast<int>(bufferViews.size())) {
      const auto &bv = bufferViews[bufferView.value()];
      if (bv) {
        return bv->getByteStride();
      }
    }
  }

  if (sparse.has_value() && sparse->values.bufferView.has_value()) {
    const auto &bufferViews = gltf.getBufferViews();
    int sparseBufferView = sparse->values.bufferView.value();
    if (sparseBufferView < static_cast<int>(bufferViews.size())) {
      const auto &bv = bufferViews[sparseBufferView];
      if (bv) {
        return bv->getByteStride();
      }
    }
  }

  return 0;
}

int GltfAccessor::getComponentCount() const {
  if (type.has_value()) {
    return AccessorTypeUtils::getComponentCount(type.value());
  }
  return 0;
}

int GltfAccessor::getComponentSize() const {
  if (!componentType.has_value()) {
    return 0;
  }

  switch (componentType.value()) {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
      return 1;
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
      return 2;
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
      return 4;
    default:
      return 0;
  }
}

int GltfAccessor::getElementSize() const {
  return getComponentSize() * getComponentCount();
}

size_t GltfAccessor::getDataSize() const {
  if (count.has_value()) {
    return static_cast<size_t>(count.value()) * getElementSize();
  }
  return 0;
}

std::vector<float>
GltfAccessor::dequantize(const void *data, size_t dataSize, int componentType) {
  std::vector<float> result;
  if (!data || dataSize == 0) {
    return result;
  }

  switch (componentType) {
    case GL_BYTE: {
      size_t elementCount = dataSize / sizeof(int8_t);
      result.reserve(elementCount);
      const int8_t *src = static_cast<const int8_t *>(data);
      for (size_t i = 0; i < elementCount; ++i) {
        result.push_back(std::max(static_cast<float>(src[i]) / 127.0f, -1.0f));
      }
    }
      break;
    case GL_UNSIGNED_BYTE: {
      size_t elementCount = dataSize / sizeof(uint8_t);
      result.reserve(elementCount);
      const uint8_t *src = static_cast<const uint8_t *>(data);
      for (size_t i = 0; i < elementCount; ++i) {
        result.push_back(static_cast<float>(src[i]) / 255.0f);
      }
    }
      break;
    case GL_SHORT: {
      size_t elementCount = dataSize / sizeof(int16_t);
      result.reserve(elementCount);
      const int16_t *src = static_cast<const int16_t *>(data);
      for (size_t i = 0; i < elementCount; ++i) {
        result.push_back(std::max(static_cast<float>(src[i]) / 32767.0f,
                                  -1.0f));
      }
    }
      break;
    case GL_UNSIGNED_SHORT: {
      size_t elementCount = dataSize / sizeof(uint16_t);
      result.reserve(elementCount);
      const uint16_t *src = static_cast<const uint16_t *>(data);
      for (size_t i = 0; i < elementCount; ++i) {
        result.push_back(static_cast<float>(src[i]) / 65535.0f);
      }
    }
      break;
    case GL_FLOAT: {
      size_t elementCount = dataSize / sizeof(float);
      result.reserve(elementCount);
      const float *src = static_cast<const float *>(data);
      result.assign(src, src + elementCount);
    }
      break;
    case GL_UNSIGNED_INT: {
      size_t elementCount = dataSize / sizeof(uint32_t);
      result.reserve(elementCount);
      const uint32_t *src = static_cast<const uint32_t *>(data);
      for (size_t i = 0; i < elementCount; ++i) {
        result.push_back(static_cast<float>(src[i]));
      }
    }
      break;
    default:
      LOGW("Unsupported component type for dequantization: %d", componentType);
      break;
  }

  return result;
}

template<typename T>
std::vector<float>
GltfAccessor::dequantize(const std::vector<T> &typedArray, int componentType) {
  return dequantize(typedArray.data(),
                    typedArray.size() * sizeof(T),
                    componentType);
}

bool GltfAccessor::isValidComponentType(int componentType) {
  return componentType == GL_BYTE ||
      componentType == GL_UNSIGNED_BYTE ||
      componentType == GL_SHORT ||
      componentType == GL_UNSIGNED_SHORT ||
      componentType == GL_UNSIGNED_INT ||
      componentType == GL_FLOAT;
}

void GltfAccessor::destroy() {
  if (glBuffer != 0) {
    glDeleteBuffers(1, &glBuffer);
    glBuffer = 0;
  }
  clearCachedViews();
}

void GltfAccessor::reset() {
  bufferView = std::nullopt;
  byteOffset = 0;
  componentType = std::nullopt;
  normalized = false;
  count = std::nullopt;
  type = std::nullopt;
  max.clear();
  min.clear();
  sparse = std::nullopt;
  name = std::nullopt;

  destroy(); // 清理OpenGL资源
  clearCachedViews();
}


std::unique_ptr<GltfAccessor>
GltfAccessor::create(int bufferViewIndex, int componentType,
                     int count, const std::string &type) {
  return std::make_unique<GltfAccessor>(bufferViewIndex,
                                        componentType,
                                        count,
                                        type);
}

// 私有方法实现
std::vector<uint8_t> GltfAccessor::createEmptyView() const {
  if (!count.has_value() || !componentType.has_value()) {
    return std::vector<uint8_t>();
  }

  size_t elementSize = getElementSize();
  size_t totalSize = count.value() * elementSize;

  return std::vector<uint8_t>(totalSize, 0);
}

void GltfAccessor::applySparse(const Gltf &gltf,
                               void *view,
                               size_t elementSize) const {
  if (!sparse.has_value() || !sparse->isValid() || !view) {
    return;
  }

  const auto &bufferViews = gltf.getBufferViews();
  const auto &buffers = gltf.getBuffers();

  // 获取索引数据
  int indicesBufferViewIndex = sparse->indices.bufferView.value();
  if (indicesBufferViewIndex >= static_cast<int>(bufferViews.size())) {
    LOGE("Invalid indices bufferView index: %d", indicesBufferViewIndex);
    return;
  }

  const auto &indicesBufferView = bufferViews[indicesBufferViewIndex];
  if (!indicesBufferView || !indicesBufferView->getBuffer().has_value()) {
    LOGE("Invalid indices buffer view");
    return;
  }

  int indicesBufferIndex = indicesBufferView->getBuffer().value();
  if (indicesBufferIndex >= static_cast<int>(buffers.size())) {
    LOGE("Invalid indices buffer index: %d", indicesBufferIndex);
    return;
  }

  const auto &indicesBuffer = buffers[indicesBufferIndex];
  if (!indicesBuffer) {
    LOGE("Invalid indices buffer");
    return;
  }

  // 获取值数据
  int valuesBufferViewIndex = sparse->values.bufferView.value();
  if (valuesBufferViewIndex >= static_cast<int>(bufferViews.size())) {
    LOGE("Invalid values bufferView index: %d", valuesBufferViewIndex);
    return;
  }

  const auto &valuesBufferView = bufferViews[valuesBufferViewIndex];
  if (!valuesBufferView || !valuesBufferView->getBuffer().has_value()) {
    LOGE("Invalid values buffer view");
    return;
  }

  int valuesBufferIndex = valuesBufferView->getBuffer().value();
  if (valuesBufferIndex >= static_cast<int>(buffers.size())) {
    LOGE("Invalid values buffer index: %d", valuesBufferIndex);
    return;
  }

  const auto &valuesBuffer = buffers[valuesBufferIndex];
  if (!valuesBuffer) {
    LOGE("Invalid values buffer");
    return;
  }

  // 创建索引类型化数组
  const auto &indicesBufferData = indicesBuffer->buffer;
  int indicesByteOffset =
      sparse->indices.byteOffset + indicesBufferView->getByteOffset();

  std::vector<uint32_t> indices;
  indices.reserve(sparse->count);

  switch (sparse->indices.componentType) {
    case GL_UNSIGNED_BYTE: {
      const uint8_t *src =
          reinterpret_cast<const uint8_t *>(indicesBufferData.data()
              + indicesByteOffset);
      for (int i = 0; i < sparse->count; ++i) {
        indices.push_back(static_cast<uint32_t>(src[i]));
      }
    }
      break;
    case GL_UNSIGNED_SHORT: {
      const uint16_t *src =
          reinterpret_cast<const uint16_t *>(indicesBufferData.data()
              + indicesByteOffset);
      for (int i = 0; i < sparse->count; ++i) {
        indices.push_back(static_cast<uint32_t>(src[i]));
      }
    }
      break;
    case GL_UNSIGNED_INT: {
      const uint32_t *src =
          reinterpret_cast<const uint32_t *>(indicesBufferData.data()
              + indicesByteOffset);
      for (int i = 0; i < sparse->count; ++i) {
        indices.push_back(src[i]);
      }
    }
      break;
    default:
      LOGE("Unsupported sparse indices component type: %d",
           sparse->indices.componentType);
      return;
  }

  // 获取值数据
  const auto &valuesBufferData = valuesBuffer->buffer;
  int valuesByteOffset =
      sparse->values.byteOffset + valuesBufferView->getByteOffset();
  const uint8_t *valuesData = valuesBufferData.data() + valuesByteOffset;

  // 应用稀疏值
  uint8_t *viewData = static_cast<uint8_t *>(view);
  for (int i = 0; i < sparse->count; ++i) {
    uint32_t index = indices[i];
    size_t srcOffset = i * elementSize;
    size_t dstOffset = index * elementSize;

    std::memcpy(viewData + dstOffset, valuesData + srcOffset, elementSize);
  }
}


void GltfAccessor::clearCachedViews() const {
  typedView.clear();
  filteredView.clear();
  normalizedTypedView.clear();
  normalizedFilteredView.clear();
  typedViewValid = false;
  filteredViewValid = false;
  normalizedTypedViewValid = false;
  normalizedFilteredViewValid = false;
}

bool GltfAccessor::validateAccessor() const {
  // count 和 componentType 是必需的
  if (!count.has_value() || !componentType.has_value()) {
    LOGE("Accessor missing required fields: count or componentType");
    return false;
  }

  if (count.value() <= 0) {
    LOGE("Invalid accessor count: %d", count.value());
    return false;
  }

  if (!isValidComponentType(componentType.value())) {
    LOGE("Invalid component type: %d", componentType.value());
    return false;
  }

  if (type.has_value() && !AccessorTypeUtils::isValidType(type.value())) {
    LOGE("Invalid accessor type: %s", type.value().c_str());
    return false;
  }

  if (byteOffset < 0) {
    LOGE("Invalid byte offset: %d", byteOffset);
    return false;
  }

  // 验证稀疏数据
  if (sparse.has_value() && !sparse->isValid()) {
    LOGE("Invalid sparse data");
    return false;
  }

  return true;
}

bool GltfAccessor::validateBounds() const {
  if (!type.has_value()) {
    return true; // 如果没有类型，跳过边界验证
  }

  int expectedComponents = AccessorTypeUtils::getComponentCount(type.value());

  if (!max.empty() && static_cast<int>(max.size()) != expectedComponents) {
    LOGW("Max array size (%zu) doesn't match expected components (%d)",
         max.size(), expectedComponents);
    return false;
  }

  if (!min.empty() && static_cast<int>(min.size()) != expectedComponents) {
    LOGW("Min array size (%zu) doesn't match expected components (%d)",
         min.size(), expectedComponents);
    return false;
  }

  // 验证 min <= max
  if (!max.empty() && !min.empty()) {
    for (size_t i = 0; i < max.size(); ++i) {
      if (min[i] > max[i]) {
        LOGW("Min value (%f) greater than max value (%f) at index %zu",
             min[i], max[i], i);
        return false;
      }
    }
  }

  return true;
}


void GltfAccessor::setType(const std::optional<std::string> &type) {
  GltfAccessor::type = type;
}

const std::optional<std::string> &GltfAccessor::getType() const {
  return type;
}


template std::vector<float>
GltfAccessor::dequantize(const std::vector<int8_t> &, int);

template std::vector<float>
GltfAccessor::dequantize(const std::vector<uint8_t> &, int);

template std::vector<float>
GltfAccessor::dequantize(const std::vector<int16_t> &, int);

template std::vector<float>
GltfAccessor::dequantize(const std::vector<uint16_t> &, int);

template std::vector<float>
GltfAccessor::dequantize(const std::vector<uint32_t> &, int);

template std::vector<float>
GltfAccessor::dequantize(const std::vector<float> &, int);

} // namespace digitalhumans