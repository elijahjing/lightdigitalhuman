//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfInterpolator.h"

#include <android/log.h>
#include <cmath>
#include <algorithm>

#include "../utils/LogUtils.h"
#include "Gltf.h"
#include "GltfAccessor.h"


namespace digitalhumans {

GltfInterpolator::GltfInterpolator()
    : prevKey(0), prevT(0.0f) {
}

std::array<float, 4> GltfInterpolator::slerpQuat(const std::array<float, 4> &q1,
                                                 const std::array<float, 4> &q2,
                                                 float t) {
  // 归一化输入四元数
  std::array<float, 4> qn1 = normalizeQuat(q1);
  std::array<float, 4> qn2 = normalizeQuat(q2);

  // 计算点积
  float dot = quatDot(qn1, qn2);

  // 如果点积为负，取反其中一个四元数以确保最短路径插值
  if (dot < 0.0f) {
    qn2[0] = -qn2[0];
    qn2[1] = -qn2[1];
    qn2[2] = -qn2[2];
    qn2[3] = -qn2[3];
    dot = -dot;
  }

  // 如果四元数非常接近，使用线性插值避免数值问题
  const float DOT_THRESHOLD = 0.9995f;
  std::array<float, 4> result;

  if (dot > DOT_THRESHOLD) {
    // 线性插值
    result[0] = qn1[0] + t * (qn2[0] - qn1[0]);
    result[1] = qn1[1] + t * (qn2[1] - qn1[1]);
    result[2] = qn1[2] + t * (qn2[2] - qn1[2]);
    result[3] = qn1[3] + t * (qn2[3] - qn1[3]);
  } else {
    // 球面线性插值
    float theta0 = std::acos(std::abs(dot));
    float sinTheta0 = std::sin(theta0);

    float theta = theta0 * t;
    float sinTheta = std::sin(theta);

    float s0 = std::cos(theta) - dot * sinTheta / sinTheta0;
    float s1 = sinTheta / sinTheta0;

    result[0] = s0 * qn1[0] + s1 * qn2[0];
    result[1] = s0 * qn1[1] + s1 * qn2[1];
    result[2] = s0 * qn1[2] + s1 * qn2[2];
    result[3] = s0 * qn1[3] + s1 * qn2[3];
  }

  return normalizeQuat(result);
}

std::vector<float> GltfInterpolator::step(int prevKey,
                                          const std::vector<float> &output,
                                          int stride) {
  std::vector<float> result(stride);

  for (int i = 0; i < stride; ++i) {
    result[i] = output[prevKey * stride + i];
  }

  return result;
}

std::vector<float> GltfInterpolator::linear(int prevKey,
                                            int nextKey,
                                            const std::vector<float> &output,
                                            float t,
                                            int stride) {
  std::vector<float> result(stride);

  for (int i = 0; i < stride; ++i) {
    float prevValue = output[prevKey * stride + i];
    float nextValue = output[nextKey * stride + i];
    result[i] = prevValue * (1.0f - t) + nextValue * t;
  }

  return result;
}

std::vector<float> GltfInterpolator::cubicSpline(int prevKey,
                                                 int nextKey,
                                                 const std::vector<float> &output,
                                                 float keyDelta,
                                                 float t,
                                                 int stride) {
  // stride: 组件数量（例如四元数为4）
  // 乘以3，因为每个输出条目包含两个切线和一个数据点
  const int prevIndex = prevKey * stride * 3;
  const int nextIndex = nextKey * stride * 3;
  const int A = 0;                // 入切线偏移
  const int V = 1 * stride;       // 值偏移
  const int B = 2 * stride;       // 出切线偏移

  std::vector<float> result(stride);
  const float tSq = t * t;
  const float tCub = t * t * t;

  // 假设输出中的组件布局为：入切线、点、出切线
  // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
  for (int i = 0; i < stride; ++i) {
    const float v0 = output[prevIndex + i + V];
    const float a = keyDelta * output[nextIndex + i + A];
    const float b = keyDelta * output[prevIndex + i + B];
    const float v1 = output[nextIndex + i + V];

    result[i] = ((2 * tCub - 3 * tSq + 1) * v0) +
        ((tCub - 2 * tSq + t) * b) +
        ((-2 * tCub + 3 * tSq) * v1) +
        ((tCub - tSq) * a);
  }

  return result;
}

void GltfInterpolator::resetKey() {
  prevKey = 0;
}

std::vector<float> GltfInterpolator::interpolate(std::shared_ptr<Gltf> gltf,
                                                 std::shared_ptr<
                                                     GltfAnimationChannel> channel,
                                                 std::shared_ptr<
                                                     GltfAnimationSampler> sampler,
                                                 float t,
                                                 int stride,
                                                 float maxTime) {
  if (!gltf || !channel || !sampler || maxTime <= 0.0f) {
    return std::vector<float>();
  }

  // 获取输入和输出访问器
  auto inputAccessor = gltf->getAccessors()[sampler->getInput().value()];
  auto outputAccessor = gltf->getAccessors()[sampler->getOutput().value()];

  if (!inputAccessor || !outputAccessor) {
    LOGE("Invalid accessor for animation sampler");
    return std::vector<float>();
  }

  // 获取归一化的数据视图
  std::vector<float>
      input = inputAccessor->getNormalizedDeinterlacedView(*gltf);
  std::vector<float>
      output = outputAccessor->getNormalizedDeinterlacedView(*gltf);

  if (input.empty() || output.empty()) {
    LOGE("Empty input or output data");
    return std::vector<float>();
  }

  // 单关键帧动画不需要插值
  if (static_cast<int>(output.size()) == stride) {
    std::vector<float> result(stride);
    std::copy(output.begin(), output.begin() + stride, result.begin());
    return result;
  }


  // 循环包装t，使动画循环播放
  // 确保t永远不早于第一个关键帧，也不晚于最后一个关键帧
  t = std::fmod(t, maxTime);
  t = clamp(t, input[0], input[input.size() - 1]);

  if (prevT > t) {
    prevKey = 0;
  }
  prevT = t;

  // 找到下一个关键帧: min{ t of input | t > prevKey }
  int nextKey = -1;
  for (int i = prevKey; i < static_cast<int>(input.size()); ++i) {
    if (t <= input[i]) {
      nextKey = clamp(i, 1, static_cast<int>(input.size()) - 1);
      break;
    }
  }

  if (nextKey == -1) {
    // 如果没找到下一个关键帧，使用最后一个
    nextKey = static_cast<int>(input.size()) - 1;
  }

  prevKey = clamp(nextKey - 1, 0, nextKey);

  const float keyDelta = input[nextKey] - input[prevKey];

  // 归一化t: [t0, t1] -> [0, 1]
  float tn = 0.0f;
  if (keyDelta > 0.0f) {
    tn = (t - input[prevKey]) / keyDelta;
  }

  // 处理旋转插值（四元数）
  if (channel->getTarget()->getPath() == InterpolationPath::ROTATION) {

    if (sampler->getInterpolation() == InterpolationMode::CUBICSPLINE) {
      // glTF要求四元数使用三次样条插值
      // https://github.com/KhronosGroup/glTF/issues/1386
      std::vector<float>
          result = cubicSpline(prevKey, nextKey, output, keyDelta, tn, 4);

      // 归一化四元数
      if (result.size() >= 4) {
        std::array<float, 4>
            quat = {result[0], result[1], result[2], result[3]};
        quat = normalizeQuat(quat);
        result[0] = quat[0];
        result[1] = quat[1];
        result[2] = quat[2];
        result[3] = quat[3];
      }
      return result;
    } else if (sampler->getInterpolation() == InterpolationMode::LINEAR) {
      std::array<float, 4> q0 = getQuat(output, prevKey);
      std::array<float, 4> q1 = getQuat(output, nextKey);
      std::array<float, 4> resultQuat = slerpQuat(q0, q1, tn);

      return std::vector<float>{resultQuat[0], resultQuat[1], resultQuat[2],
                                resultQuat[3]};
    } else if (sampler->getInterpolation() == InterpolationMode::STEP) {
      std::array<float, 4> q0 = getQuat(output, prevKey);
      return std::vector<float>{q0[0], q0[1], q0[2], q0[3]};
    }
  }

  // 处理其他类型的插值
  switch (sampler->getInterpolation()) {
    case InterpolationMode::STEP:
      return step(prevKey, output, stride);

    case InterpolationMode::CUBICSPLINE:
      return cubicSpline(prevKey, nextKey, output, keyDelta, tn, stride);

    case InterpolationMode::LINEAR:
    default:
      return linear(prevKey, nextKey, output, tn, stride);
  }
  return std::vector<float>();
}

std::array<float, 4>
GltfInterpolator::getQuat(const std::vector<float> &output, int index) {
  if (static_cast<size_t>(4 * index + 3) >= output.size()) {
    LOGE("Index out of bounds when getting quaternion");
    return {0.0f, 0.0f, 0.0f, 1.0f}; // 返回单位四元数
  }

  const float x = output[4 * index];
  const float y = output[4 * index + 1];
  const float z = output[4 * index + 2];
  const float w = output[4 * index + 3];

  return {x, y, z, w};
}

std::array<float, 4>
GltfInterpolator::normalizeQuat(const std::array<float, 4> &quat) {
  float length = std::sqrt(quat[0] * quat[0] + quat[1] * quat[1] +
      quat[2] * quat[2] + quat[3] * quat[3]);

  if (length < 1e-6f) {
    // 如果四元数长度接近0，返回单位四元数
    return {0.0f, 0.0f, 0.0f, 1.0f};
  }

  const float invLength = 1.0f / length;
  return {
      quat[0] * invLength,
      quat[1] * invLength,
      quat[2] * invLength,
      quat[3] * invLength
  };
}

float GltfInterpolator::quatDot(const std::array<float, 4> &q1,
                                const std::array<float, 4> &q2) {
  return q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3];
}

float GltfInterpolator::clamp(float value, float min, float max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

int GltfInterpolator::clamp(int value, int min, int max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

} // namespace digitalhumans