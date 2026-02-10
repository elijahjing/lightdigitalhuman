//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFACCESSOR_H
#define LIGHTDIGITALHUMAN_GLTFACCESSOR_H

#include "GltfObject.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>
#include <cstdint>

namespace digitalhumans {

/**
 * @brief 访问器类型枚举
 */
enum class AccessorType {
  SCALAR,
  VEC2,
  VEC3,
  VEC4,
  MAT2,
  MAT3,
  MAT4
};

/**
 * @brief 访问器类型工具类
 */
class AccessorTypeUtils {
 public:
  static AccessorType fromString(const std::string &typeStr);

  static std::string toString(AccessorType type);

  static int getComponentCount(AccessorType type);

  static int getComponentCount(const std::string &typeStr);

  static bool isValidType(const std::string &typeStr);

 private:
  static const std::unordered_map<std::string, AccessorType> stringToEnum;
  static const std::unordered_map<AccessorType, std::string> enumToString;
  static const std::unordered_map<AccessorType, int> componentCounts;
};

/**
 * @brief 稀疏访问器索引
 */
struct GltfSparseIndices {
  std::optional<int> bufferView;
  int byteOffset = 0;
  int componentType = 0;

  bool isValid() const {
    return bufferView.has_value() && componentType != 0;
  }
};

/**
 * @brief 稀疏访问器值
 */
struct GltfSparseValues {
  std::optional<int> bufferView;
  int byteOffset = 0;

  bool isValid() const {
    return bufferView.has_value();
  }
};

/**
 * @brief 稀疏访问器
 */
struct GltfSparse {
  int count = 0;
  GltfSparseIndices indices;
  GltfSparseValues values;

  bool isValid() const {
    return count > 0 && indices.isValid() && values.isValid();
  }
};

/**
 * @brief glTF访问器类
 * 定义如何从缓冲区读取顶点属性数据
 */
class GltfAccessor: public GltfObject {
 public:
  static const std::vector<std::string> animatedProperties;

  /**
   * @brief 构造函数
   */
  GltfAccessor();

  /**
   * @brief 带参数的构造函数
   * @param bufferViewIndex 缓冲区视图索引
   * @param componentType 组件类型
   * @param count 元素数量
   * @param type 访问器类型
   */
  GltfAccessor(int bufferViewIndex,
               int componentType,
               int count,
               const std::string &type);

  // 析构函数
  ~GltfAccessor() override;





  // 删除拷贝构造函数，因为涉及到OpenGL资源
  GltfAccessor(const GltfAccessor &) = delete;

  GltfAccessor &operator=(const GltfAccessor &) = delete;

  // 移动构造函数和赋值操作符
  GltfAccessor(GltfAccessor &&other) noexcept;

  GltfAccessor &operator=(GltfAccessor &&other) noexcept;


  /**
   * @brief 验证数据有效性
   * @return true如果数据有效
   */
  bool validate() const override;


  // 访问器方法
  std::optional<int> getBufferView() const { return bufferView; }

  void setBufferView(int bufferViewIndex) { bufferView = bufferViewIndex; }

  void clearBufferView() { bufferView = std::nullopt; }

  bool hasBufferView() const { return bufferView.has_value(); }

  int getByteOffset() const { return byteOffset; }

  void setByteOffset(int offset) { byteOffset = offset; }

  std::optional<int> getComponentType() const { return componentType; }

  void setComponentType(int type) { componentType = type; }

  bool hasComponentType() const { return componentType.has_value(); }

  bool isNormalized() const { return normalized; }

  void setNormalized(bool norm) { normalized = norm; }

  std::optional<int> getCount() const { return count; }

  void setCount(int elementCount) { count = elementCount; }

  bool hasCount() const { return count.has_value(); }

  const std::optional<std::string> &getAccessorType() const { return type; }

  void setAccessorType(const std::string &accessorType);

  void setAccessorType(AccessorType accessorType);

  bool hasAccessorType() const { return type.has_value(); }

  const std::vector<double> &getMax() const { return max; }

  void setMax(const std::vector<double> &maxValues) { max = maxValues; }

  void clearMax() { max.clear(); }

  bool hasMax() const { return !max.empty(); }

  const std::vector<double> &getMin() const { return min; }

  void setMin(const std::vector<double> &minValues) { min = minValues; }

  void clearMin() { min.clear(); }

  bool hasMin() const { return !min.empty(); }

  const std::optional<GltfSparse> &getSparse() const { return sparse; }

  void setSparse(const GltfSparse &sparseData) { sparse = sparseData; }

  void clearSparse() { sparse = std::nullopt; }

  bool hasSparse() const { return sparse.has_value(); }

  const std::optional<std::string> &getName() const { return name; }

  void setName(const std::string &accessorName) { name = accessorName; }

  void clearName() { name = std::nullopt; }

  bool hasName() const { return name.has_value(); }

  /**
   * @brief 获取类型化视图（直接用于OpenGL）
   * @param gltf glTF根对象
   * @return 类型化数组的指针和大小
   */
  std::pair<const void *, size_t> getTypedView(const Gltf &gltf);

  /**
   * @brief 获取标准化类型化视图（用于动画等）
   * @param gltf glTF根对象
   * @return 标准化后的float数组
   */
  std::vector<float> getNormalizedTypedView(const Gltf &gltf);

  /**
   * @brief 获取去交错视图（去除填充和无关组件）
   * @param gltf glTF根对象
   * @return 去交错后的数组
   */
  std::pair<const void *, size_t> getDeinterlacedView(const Gltf &gltf);

  /**
   * @brief 获取标准化去交错视图
   * @param gltf glTF根对象
   * @return 标准化去交错后的float数组
   */
  std::vector<float> getNormalizedDeinterlacedView(const Gltf &gltf);

  /**
   * @brief 获取字节步长
   * @param gltf glTF根对象
   * @return 字节步长
   */
  int getByteStride(const Gltf &gltf) const;

  /**
   * @brief 获取组件数量
   * @return 组件数量
   */
  int getComponentCount() const;

  /**
   * @brief 获取组件大小（字节）
   * @return 组件大小
   */
  int getComponentSize() const;

  /**
   * @brief 获取元素大小（字节）
   * @return 元素大小
   */
  int getElementSize() const;

  /**
   * @brief 获取总数据大小（字节）
   * @return 数据大小
   */
  size_t getDataSize() const;

  /**
   * @brief 反量化数据
   * @param data 输入数据
   * @param dataSize 数据大小
   * @param componentType 组件类型
   * @return 反量化后的float数组
   */
  static std::vector<float>
  dequantize(const void *data, size_t dataSize, int componentType);

  /**
   * @brief 反量化类型化数组
   * @tparam T 输入类型
   * @param typedArray 输入数组
   * @param componentType 组件类型
   * @return 反量化后的float数组
   */
  template<typename T>
  static std::vector<float>
  dequantize(const std::vector<T> &typedArray, int componentType);

  /**
   * @brief 验证组件类型
   * @param componentType 组件类型
   * @return true如果有效
   */
  static bool isValidComponentType(int componentType);

  /**
   * @brief 获取OpenGL缓冲区对象
   * @return 缓冲区ID
   */
  unsigned int getGLBuffer() const { return glBuffer; }

  /**
   * @brief 设置OpenGL缓冲区对象
   * @param buffer 缓冲区ID
   */
  void setGLBuffer(unsigned int buffer) { glBuffer = buffer; }

  /**
   * @brief 清理OpenGL资源
   */
  void destroy();

  /**
   * @brief 重置到默认状态
   */
  void reset();

  /**
   * @brief 创建简单访问器
   * @param bufferViewIndex 缓冲区视图索引
   * @param componentType 组件类型
   * @param count 元素数量
   * @param type 访问器类型
   * @return 新创建的访问器
   */
  static std::unique_ptr<GltfAccessor>
  create(int bufferViewIndex, int componentType,
         int count, const std::string &type);

  void setType(const std::optional<std::string> &type);

  const std::optional<std::string> &getType() const;

 private:
  // glTF 访问器属性
  std::optional<int> bufferView;      ///< 缓冲区视图索引
  int byteOffset = 0;                 ///< 字节偏移
  std::optional<int> componentType;   ///< 组件类型（GL枚举）
  bool normalized = false;            ///< 是否标准化
  std::optional<int> count;           ///< 元素数量
  std::optional<std::string> type;    ///< 访问器类型
  std::vector<double> max;             ///< 最大值
  std::vector<double> min;             ///< 最小值
  std::optional<GltfSparse> sparse;   ///< 稀疏数据
  std::optional<std::string> name;    ///< 访问器名称

  // 非glTF属性（运行时数据）
  unsigned int glBuffer = 0;                      ///< OpenGL缓冲区对象
  mutable std::vector<uint8_t> typedView;         ///< 缓存的类型化视图
  mutable std::vector<uint8_t> filteredView;      ///< 缓存的过滤视图
  mutable std::vector<float> normalizedTypedView; ///< 缓存的标准化类型化视图
  mutable std::vector<float> normalizedFilteredView; ///< 缓存的标准化过滤视图
  mutable bool typedViewValid = false;            ///< 类型化视图是否有效
  mutable bool filteredViewValid = false;         ///< 过滤视图是否有效
  mutable bool normalizedTypedViewValid = false;  ///< 标准化类型化视图是否有效
  mutable bool normalizedFilteredViewValid = false; ///< 标准化过滤视图是否有效

  /**
   * @brief 创建空视图
   * @return 空的类型化数组
   */
  std::vector<uint8_t> createEmptyView() const;

  /**
   * @brief 应用稀疏数据
   * @param gltf glTF根对象
   * @param view 目标视图
   */
  void applySparse(const Gltf &gltf, void *view, size_t elementSize) const;


  /**
   * @brief 清理缓存的视图
   */
  void clearCachedViews() const;

  /**
   * @brief 验证访问器完整性
   * @return true如果数据完整
   */
  bool validateAccessor() const;

  /**
   * @brief 验证边界值
   * @return true如果边界值有效
   */
  bool validateBounds() const;
};

} // namespace digitalhumans


#endif //LIGHTDIGITALHUMAN_GLTFACCESSOR_H
