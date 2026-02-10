//
// Created by vincentsyan on 2025/8/18.
//

#ifndef LIGHTDIGITALHUMAN_ENGINE_H
#define LIGHTDIGITALHUMAN_ENGINE_H

#include <memory>
#include <vector>

namespace digitalhumans {
class GltfRenderer;

class GltfState;

class GltfOpenGLContext;

class UserCamera;

class HDRImage;

class Engine {
 public:

  explicit Engine();

  ~Engine() = default;

  void renderFrame(int width, int height);

  void setUserCamera(std::shared_ptr<UserCamera> userCamera) const;
  std::shared_ptr<GltfRenderer> renderer;
  std::shared_ptr<GltfState> state;
  std::shared_ptr<GltfOpenGLContext> context;
  std::vector<std::string> getAnimationAllName() const;

  bool processEnvironmentMap(const HDRImage &hdrImage) const;

  const std::shared_ptr<GltfState> &getState() const;

  void setState(const std::shared_ptr<GltfState> &state);

  void playAnimation(std::string name, int time) const;


  void stopAnimation(const std::string &name) const;

  void setIbL(bool use) const;

 private:
  /**
   * @brief 动画更新
   * 内部方法，用于更新动画状态
   * @param state GltfState对象
   */
  void animate(std::shared_ptr<GltfState> state);
  bool init = false;

};

} // namespace digitalhumans

#endif //LIGHTDIGITALHUMAN_ENGINE_H
