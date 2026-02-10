//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_GLTFOBJECT_H
#define LIGHTDIGITALHUMAN_GLTFOBJECT_H
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <any>
#include <typeinfo>

namespace digitalhumans {

class GltfOpenGLContext;
class Gltf;

/**
 * @brief glTF对象基类 - 所有glTF对象的基类
 * 提供动画属性管理和基础功能
 */
class GltfObject {
 public:
  /**
   * @brief 构造函数
   * 初始化动画属性对象
   */
  GltfObject();

  /**
   * @brief 虚析构函数
   */
  virtual ~GltfObject() = default;


  /**
   * @brief 验证数据有效性
   * @return true如果数据有效
*/
  virtual bool validate() const { return true; }


  /**
   * @brief 调试输出
   * @return 描述字符串
   */
  virtual std::string toString() const { return ""; };

};

} // namespace digitalhumans



#endif //LIGHTDIGITALHUMAN_GLTFOBJECT_H
