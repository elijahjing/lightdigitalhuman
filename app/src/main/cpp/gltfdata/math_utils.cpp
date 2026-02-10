//
// Created by vincentsyan on 2025/8/18.
//

#include "math_utils.h"
#include "math_utils.h"
#include <cmath>
#include <limits>

namespace digitalhumans {
namespace math_utils {

// 显式实例化常用类型
template bool isPowerOf2<int>(int n) noexcept;
template bool isPowerOf2<uint32_t>(uint32_t n) noexcept;
template bool isPowerOf2<uint64_t>(uint64_t n) noexcept;

template int nearestPowerOf2<int>(int n) noexcept;
template uint32_t nearestPowerOf2<uint32_t>(uint32_t n) noexcept;
template float nearestPowerOf2<float>(float n) noexcept;
template double nearestPowerOf2<double>(double n) noexcept;

template int makeEven<int>(int n) noexcept;
template uint32_t makeEven<uint32_t>(uint32_t n) noexcept;

template int makeOdd<int>(int n) noexcept;
template uint32_t makeOdd<uint32_t>(uint32_t n) noexcept;

template int ceilPowerOf2<int>(int n) noexcept;
template uint32_t ceilPowerOf2<uint32_t>(uint32_t n) noexcept;

template int floorPowerOf2<int>(int n) noexcept;
template uint32_t floorPowerOf2<uint32_t>(uint32_t n) noexcept;

template int log2Int<int>(int n) noexcept;
template int log2Int<uint32_t>(uint32_t n) noexcept;

} // namespace math_utils
} // namespace digitalhumans