//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_MATH_UTILS_H
#define LIGHTDIGITALHUMAN_MATH_UTILS_H



#include <cstdint>
#include <type_traits>
#include <cmath>  // 包含数学函数
namespace digitalhumans {
namespace math_utils {

/**
 * @brief 检查一个数是否是2的幂
 * @tparam T 整数类型
 * @param n 要检查的数
 * @return 如果n是2的幂则返回true，否则返回false
 */
template<typename T>
constexpr bool isPowerOf2(T n) noexcept {
  static_assert(std::is_integral_v<T>, "T must be an integral type");
  return n > 0 && (n & (n - 1)) == 0;
}

/**
 * @brief 找到最接近给定数的2的幂
 * @tparam T 数值类型
 * @param n 输入数值
 * @return 最接近n的2的幂
 */
template<typename T>
constexpr T nearestPowerOf2(T n) noexcept {
  static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");

  if (n <= 0) {
    return 1;
  }

  if constexpr (std::is_integral_v<T>) {
    if (isPowerOf2(n)) {
      return n;
    }
  }

  // 使用位操作进行快速计算
  if constexpr (std::is_integral_v<T>) {
    // 对于整数类型，使用位操作
    if (n == 1) return 1;

    // 找到最高位
    T power = 1;
    while (power < n) {
      power <<= 1;
    }

    // 检查哪个更接近：power 还是 power/2
    T prevPower = power >> 1;
    return (n - prevPower) < (power - n) ? prevPower : power;
  } else {
    // 对于浮点类型，使用数学函数
    return static_cast<T>(std::pow(2.0,
                                   std::round(std::log(n) / std::log(2.0))));
  }
}

/**
 * @brief 将数值调整为偶数
 * @tparam T 整数类型
 * @param n 输入数值
 * @return 如果n是奇数则返回n+1，否则返回n
 */
template<typename T>
constexpr T makeEven(T n) noexcept {
  static_assert(std::is_integral_v<T>, "T must be an integral type");
  return (n & 1) ? n + 1 : n;
}

/**
 * @brief 将数值调整为奇数
 * @tparam T 整数类型
 * @param n 输入数值
 * @return 如果n是偶数则返回n+1，否则返回n
 */
template<typename T>
constexpr T makeOdd(T n) noexcept {
  static_assert(std::is_integral_v<T>, "T must be an integral type");
  return (n & 1) ? n : n + 1;
}

/**
 * @brief 向上取整到最近的2的幂
 * @tparam T 整数类型
 * @param n 输入数值
 * @return 大于等于n的最小2的幂
 */
template<typename T>
constexpr T ceilPowerOf2(T n) noexcept {
  static_assert(std::is_integral_v<T>, "T must be an integral type");

  if (n <= 0) {
    return 1;
  }

  if (isPowerOf2(n)) {
    return n;
  }

  T power = 1;
  while (power < n) {
    power <<= 1;
  }
  return power;
}

/**
 * @brief 向下取整到最近的2的幂
 * @tparam T 整数类型
 * @param n 输入数值
 * @return 小于等于n的最大2的幂
 */
template<typename T>
constexpr T floorPowerOf2(T n) noexcept {
  static_assert(std::is_integral_v<T>, "T must be an integral type");

  if (n <= 0) {
    return 1;
  }

  if (isPowerOf2(n)) {
    return n;
  }

  T power = 1;
  while (power <= n) {
    power <<= 1;
  }
  return power >> 1;
}

/**
 * @brief 计算log2的整数值
 * @tparam T 整数类型
 * @param n 输入数值（必须是2的幂）
 * @return log2(n)的值，如果n不是2的幂则返回-1
 */
template<typename T>
constexpr int log2Int(T n) noexcept {
  static_assert(std::is_integral_v<T>, "T must be an integral type");

  if (!isPowerOf2(n)) {
    return -1;
  }

  int result = 0;
  while (n > 1) {
    n >>= 1;
    result++;
  }
  return result;
}

/**
 * @brief 检查数值是否在指定范围内
 * @tparam T 数值类型
 * @param value 要检查的值
 * @param min 最小值（包含）
 * @param max 最大值（包含）
 * @return 如果value在[min, max]范围内则返回true
 */
template<typename T>
constexpr bool inRange(T value, T min, T max) noexcept {
  return value >= min && value <= max;
}

/**
 * @brief 将数值限制在指定范围内
 * @tparam T 数值类型
 * @param value 要限制的值
 * @param min 最小值
 * @param max 最大值
 * @return 限制在[min, max]范围内的值
 */
template<typename T>
constexpr T clamp(T value, T min, T max) noexcept {
  return value < min ? min : (value > max ? max : value);
}

/**
 * @brief 线性插值
 * @tparam T 数值类型
 * @param a 起始值
 * @param b 结束值
 * @param t 插值参数 [0, 1]
 * @return 插值结果
 */
template<typename T>
constexpr T lerp(T a, T b, T t) noexcept {
  return a + t * (b - a);
}

} // namespace math_utils
} // namespace digitalhumans

// 包含必要的头文件用于模板实现
#include <cmath>


#endif //LIGHTDIGITALHUMAN_MATH_UTILS_H
