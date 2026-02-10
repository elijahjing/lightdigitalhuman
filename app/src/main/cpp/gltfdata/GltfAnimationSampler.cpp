//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfAnimationSampler.h"

#include "../utils/utils.h"
#include <unordered_map>
#include <sstream>

#include "../utils/LogUtils.h"
#include "Gltf.h"

#include "GltfAccessor.h"


namespace digitalhumans {

// ===== InterpolationModeUtils实现 =====

    const std::string InterpolationModeUtils::LINEAR_STR = "LINEAR";
    const std::string InterpolationModeUtils::STEP_STR = "STEP";
    const std::string InterpolationModeUtils::CUBICSPLINE_STR = "CUBICSPLINE";

// 字符串到枚举的映射表
    static const std::unordered_map<std::string, InterpolationMode> stringToMode = {
            {InterpolationModeUtils::LINEAR_STR, InterpolationMode::LINEAR},
            {InterpolationModeUtils::STEP_STR, InterpolationMode::STEP},
            {InterpolationModeUtils::CUBICSPLINE_STR, InterpolationMode::CUBICSPLINE}
    };

// 枚举到字符串的映射表
    static const std::unordered_map<InterpolationMode, std::string> modeToString = {
            {InterpolationMode::LINEAR, InterpolationModeUtils::LINEAR_STR},
            {InterpolationMode::STEP, InterpolationModeUtils::STEP_STR},
            {InterpolationMode::CUBICSPLINE, InterpolationModeUtils::CUBICSPLINE_STR}
    };

    InterpolationMode InterpolationModeUtils::fromString(const std::string& modeString)
    {
        auto it = stringToMode.find(modeString);
        return (it != stringToMode.end()) ? it->second : InterpolationMode::UNKNOWN;
    }

    std::string InterpolationModeUtils::toString(InterpolationMode mode)
    {
        auto it = modeToString.find(mode);
        return (it != modeToString.end()) ? it->second : "";
    }

    bool InterpolationModeUtils::isValid(InterpolationMode mode)
    {
        return mode != InterpolationMode::UNKNOWN;
    }

    std::vector<std::string> InterpolationModeUtils::getAllModeStrings()
    {
        return {LINEAR_STR, STEP_STR, CUBICSPLINE_STR};
    }

// ===== GltfAnimationSampler实现 =====

    GltfAnimationSampler::GltfAnimationSampler()
            : GltfObject()
            , input()
            , interpolation(InterpolationMode::LINEAR)  // 默认为线性插值
            , output()
    {
    }


    std::shared_ptr<GltfAnimationSampler> GltfAnimationSampler::clone() const
    {
        auto cloned = std::make_shared<GltfAnimationSampler>();
        cloned->input = input;
        cloned->interpolation = interpolation;
        cloned->output = output;
        return cloned;
    }

    bool GltfAnimationSampler::validate(std::shared_ptr<Gltf> gltf) const
    {
        if (!gltf) {
            LOGE("Invalid glTF object for validation");
            return false;
        }

        // 检查是否有输入访问器
        if (!input.has_value()) {
            LOGE("Animation sampler missing input accessor");
            return false;
        }

        // 检查是否有输出访问器
        if (!output.has_value()) {
            LOGE("Animation sampler missing output accessor");
            return false;
        }

        // 验证输入访问器
        if (!validateAccessor(gltf, input.value(), "input")) {
            return false;
        }

        // 验证输出访问器
        if (!validateAccessor(gltf, output.value(), "output")) {
            return false;
        }

        // 检查插值模式是否有效
        if (!InterpolationModeUtils::isValid(interpolation)) {
            LOGE("Invalid interpolation mode");
            return false;
        }

        // 检查输入和输出访问器的兼容性
        if (!areAccessorsCompatible(gltf)) {
            LOGE("Input and output accessors are not compatible");
            return false;
        }

        return true;
    }

    void GltfAnimationSampler::reset()
    {
        input.reset();
        interpolation = InterpolationMode::LINEAR;
        output.reset();
    }

    bool GltfAnimationSampler::isComplete() const
    {
        return input.has_value() && output.has_value() &&
               InterpolationModeUtils::isValid(interpolation);
    }

    bool GltfAnimationSampler::getTimeRange(std::shared_ptr<Gltf> gltf, float& outMinTime, float& outMaxTime) const
    {
        if (!gltf || !input.has_value()) {
            return false;
        }

        const auto& accessors = gltf->getAccessors();
        if (input.value() < 0 || input.value() >= static_cast<int>(accessors.size())) {
            return false;
        }

        auto inputAccessor = accessors[input.value()];
        if (!inputAccessor) {
            return false;
        }

        const auto& minValues = inputAccessor->getMin();
        const auto& maxValues = inputAccessor->getMax();

        if (minValues.empty() || maxValues.empty()) {
            return false;
        }

        outMinTime = minValues[0];
        outMaxTime = maxValues[0];
        return true;
    }

    size_t GltfAnimationSampler::getKeyFrameCount(std::shared_ptr<Gltf> gltf) const
    {
        if (!gltf || !input.has_value()) {
            return 0;
        }

        const auto& accessors = gltf->getAccessors();
        if (input.value() < 0 || input.value() >= static_cast<int>(accessors.size())) {
            return 0;
        }

        auto inputAccessor = accessors[input.value()];
        if (!inputAccessor) {
            return 0;
        }

        return inputAccessor->getCount().value();
    }

    bool GltfAnimationSampler::areAccessorsCompatible(std::shared_ptr<Gltf> gltf) const
    {
        if (!gltf || !input.has_value() || !output.has_value()) {
            return false;
        }

        const auto& accessors = gltf->getAccessors();

        // 检查输入访问器
        if (input.value() < 0 || input.value() >= static_cast<int>(accessors.size())) {
            return false;
        }

        // 检查输出访问器
        if (output.value() < 0 || output.value() >= static_cast<int>(accessors.size())) {
            return false;
        }

        auto inputAccessor = accessors[input.value()];
        auto outputAccessor = accessors[output.value()];

        if (!inputAccessor || !outputAccessor) {
            return false;
        }

        // 输入访问器应该是SCALAR（时间值）
        if (inputAccessor->getType() != "SCALAR") {
            LOGE("Input accessor must be SCALAR type");
            return false;
        }

        // 对于三次样条插值，输出访问器的数量应该是输入的3倍
        if (interpolation == InterpolationMode::CUBICSPLINE) {
            if (outputAccessor->getCount() != inputAccessor->getCount().value() * 3) {
                LOGE("For CUBICSPLINE interpolation, output count must be 3x input count");
                return false;
            }
        } else {
            // 对于线性和阶跃插值，输入和输出的数量应该相同
            if (outputAccessor->getCount() != inputAccessor->getCount()) {
                LOGE("For LINEAR/STEP interpolation, input and output counts must match");
                return false;
            }
        }

        return true;
    }

    void GltfAnimationSampler::setInterpolationFromString(const std::string& modeString)
    {
        interpolation = InterpolationModeUtils::fromString(modeString);
        if (interpolation == InterpolationMode::UNKNOWN) {
            LOGW("Unknown interpolation mode: %s, using LINEAR", modeString.c_str());
            interpolation = InterpolationMode::LINEAR;
        }
    }

    std::string GltfAnimationSampler::getInterpolationString() const
    {
        return InterpolationModeUtils::toString(interpolation);
    }

    std::string GltfAnimationSampler::getDescription() const
    {
        std::ostringstream oss;

        oss << "Animation Sampler:\n";

        if (input.has_value()) {
            oss << "  Input Accessor: " << input.value() << "\n";
        } else {
            oss << "  Input Accessor: Not set\n";
        }

        oss << "  Interpolation: " << getInterpolationString() << "\n";

        if (output.has_value()) {
            oss << "  Output Accessor: " << output.value() << "\n";
        } else {
            oss << "  Output Accessor: Not set\n";
        }

        oss << "  Complete: " << (isComplete() ? "Yes" : "No");

        return oss.str();
    }

// === 静态工厂方法 ===

    std::shared_ptr<GltfAnimationSampler> GltfAnimationSampler::createLinear(int inputAccessor, int outputAccessor)
    {
        auto sampler = std::make_shared<GltfAnimationSampler>();
        sampler->setInput(inputAccessor);
        sampler->setOutput(outputAccessor);
        sampler->setInterpolation(InterpolationMode::LINEAR);
        return sampler;
    }

    std::shared_ptr<GltfAnimationSampler> GltfAnimationSampler::createStep(int inputAccessor, int outputAccessor)
    {
        auto sampler = std::make_shared<GltfAnimationSampler>();
        sampler->setInput(inputAccessor);
        sampler->setOutput(outputAccessor);
        sampler->setInterpolation(InterpolationMode::STEP);
        return sampler;
    }

    std::shared_ptr<GltfAnimationSampler> GltfAnimationSampler::createCubicSpline(int inputAccessor, int outputAccessor)
    {
        auto sampler = std::make_shared<GltfAnimationSampler>();
        sampler->setInput(inputAccessor);
        sampler->setOutput(outputAccessor);
        sampler->setInterpolation(InterpolationMode::CUBICSPLINE);
        return sampler;
    }

// === 私有方法实现 ===

    bool GltfAnimationSampler::validateAccessor(std::shared_ptr<Gltf> gltf, int accessorIndex, const std::string& accessorName) const
    {
        const auto& accessors = gltf->getAccessors();

        if (accessorIndex < 0 || accessorIndex >= static_cast<int>(accessors.size())) {
            LOGE("Invalid %s accessor index: %d", accessorName.c_str(), accessorIndex);
            return false;
        }

        auto accessor = accessors[accessorIndex];
        if (!accessor) {
            LOGE("%s accessor is null", accessorName.c_str());
            return false;
        }

        return true;
    }

} // namespace digitalhumans