//
// Created by vincentsyan on 2025/8/20.
//

#ifndef LIGHTDIGITALHUMAN_UNIFORMTYPES_H
#define LIGHTDIGITALHUMAN_UNIFORMTYPES_H

#include <vector>
#include <variant>
#include <memory>
#include "glm.hpp"
#include "../utils/utils.h"

namespace digitalhumans {

class UniformLight;
class UniformStruct;
/**
* @brief uniform值的类型变体
*/
using UniformValue = std::variant<
    float,                          ///< 单精度浮点数
    int,                           ///< 整数
    bool,                          ///< 布尔值
    double,                        ///< 双精度浮点数
    glm::vec2,                     ///< 2D向量
    glm::vec3,                     ///< 3D向量
    glm::vec4,                     ///< 4D向量
    glm::ivec2,                    ///< 2D整数向量
    glm::ivec3,                    ///< 3D整数向量
    glm::ivec4,                    ///< 4D整数向量
    glm::mat2,                     ///< 2x2矩阵
    glm::mat3,                     ///< 3x3矩阵
    glm::mat4,                     ///< 4x4矩阵
    std::vector<float>,            ///< 浮点数数组
    std::vector<double>,           ///< 双精度浮点数数组
    std::vector<int>,              ///< 整数数组
    std::vector<glm::vec2>,        ///< 2D向量数组
    std::vector<glm::vec3>,        ///< 3D向量数组
    std::vector<glm::vec4>,        ///< 4D向量数组
    std::vector<glm::mat3>,        ///< 3x3矩阵数组
    std::vector<glm::mat4>,        ///< 4x4矩阵数组
    std::shared_ptr<UniformLight>, ///< 结构体
    std::shared_ptr<std::vector<UniformLight>>,   // ✅ 确保有这一行

    std::shared_ptr<UniformStruct>  ///< 结构体
>;

} // namespace digitalhumans

#endif //LIGHTDIGITALHUMAN_UNIFORMTYPES_H
