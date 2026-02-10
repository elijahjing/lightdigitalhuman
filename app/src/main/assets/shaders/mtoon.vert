#version 300 es
precision highp float;

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec4 aTangent;
layout (location = 4) in float aVertexID;
layout (location = 5) in ivec4 aJointIndices;
layout (location = 6) in vec4 aJointWeights;

//  变换矩阵
uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat3 uNormalMatrix;

//  骨骼动画
uniform mat4 uJointMatrices[64];  // 最多支持64个骨骼
uniform int uHasSkeleton;         // 是否启用骨骼动画

//  纹理化morph系统
layout(std140) uniform MorphWeights {
    vec4 uMorphWeights[64];  // 最多支持256个morph targets (64 * 4)
};

uniform int uMorphCount;
uniform int uTextureWidth;
uniform int uTextureHeight;
uniform sampler2D uMorphPositionTexture;
uniform sampler2D uMorphNormalTexture;

//  输出变量
out vec3 vWorldPosition;
out vec3 vViewPosition;
out vec3 vWorldNormal;
out vec3 vViewNormal;
out vec2 vUV;
out vec3 vTangent;
out vec3 vBitangent;
out float vMorphDebug;

//  轮廓线渲染
uniform int isOutlinePass;
uniform float outlineWidthFactor;

//  获取morph权重的函数
float getMorphWeight(int morphIndex) {
    if (morphIndex >= 256) return 0.0;  // 超出最大支持数量

    int vec4Index = morphIndex / 4;
    int componentIndex = morphIndex % 4;

    if (vec4Index >= 64) return 0.0;  // 超出UBO大小

    vec4 weightVec = uMorphWeights[vec4Index];

    if (componentIndex == 0) return weightVec.x;
    else if (componentIndex == 1) return weightVec.y;
    else if (componentIndex == 2) return weightVec.z;
    else return weightVec.w;
}

//  计算纹理坐标的函数
vec2 getTextureCoord(int vertexIndex, int morphIndex) {
    int pixelIndex = vertexIndex * uMorphCount + morphIndex;
    int x = pixelIndex % uTextureWidth;
    int y = pixelIndex / uTextureWidth;

    return vec2(
    (float(x) + 0.5) / float(uTextureWidth),
    (float(y) + 0.5) / float(uTextureHeight)
    );
}

void main() {
    int vertexIndex = int(aVertexID);

    //  初始化为基础几何体
    vec3 morphedPosition = aPosition;
    vec3 morphedNormal = aNormal;
    float totalMorphWeight = 0.0;

    //  应用纹理化morph targets
    for (int i = 0; i < uMorphCount && i < 256; ++i) {
        float weight = getMorphWeight(i);

        // 只处理有效权重，避免不必要的纹理采样
        if (abs(weight) > 0.001) {
            vec2 texCoord = getTextureCoord(vertexIndex, i);

            //  采样morph偏移数据
            vec3 positionDelta = texture(uMorphPositionTexture, texCoord).xyz;
            vec3 normalDelta = texture(uMorphNormalTexture, texCoord).xyz;

            //  应用权重
            morphedPosition += positionDelta * weight;
            morphedNormal += normalDelta * weight;

            totalMorphWeight += abs(weight);
        }
    }

    //  轮廓线处理
    vec3 finalPosition = morphedPosition;
    vec3 finalNormal = normalize(morphedNormal);

    //  骨骼动画变换
    if (uHasSkeleton == 1) {
        // 计算骨骼变换矩阵
        mat4 skinMatrix =
            aJointWeights.x * uJointMatrices[aJointIndices.x] +
            aJointWeights.y * uJointMatrices[aJointIndices.y] +
            aJointWeights.z * uJointMatrices[aJointIndices.z] +
            aJointWeights.w * uJointMatrices[aJointIndices.w];

        // 应用骨骼变换到位置
        vec4 skinnedPosition = skinMatrix * vec4(finalPosition, 1.0);
        finalPosition = skinnedPosition.xyz;

        // 应用骨骼变换到法线 (只使用旋转部分)
        mat3 skinNormalMatrix = mat3(skinMatrix);
        finalNormal = normalize(skinNormalMatrix * finalNormal);
    }

    if (isOutlinePass == 1) {
        finalPosition += finalNormal * outlineWidthFactor;
    }

    //  变换计算
    vec4 worldPosition = uModelMatrix * vec4(finalPosition, 1.0);
    vec4 viewPosition = uViewMatrix * worldPosition;

    vWorldPosition = worldPosition.xyz;
    vViewPosition = viewPosition.xyz;
    vWorldNormal = normalize(uNormalMatrix * finalNormal);
    vViewNormal = normalize((mat3(uViewMatrix) * mat3(uModelMatrix)) * finalNormal);

    vUV = aUV;

    //  切线空间计算
    vTangent = normalize(uNormalMatrix * aTangent.xyz);
    vBitangent = normalize(cross(vWorldNormal, vTangent) * aTangent.w);

    vMorphDebug = totalMorphWeight;

    gl_Position = uProjectionMatrix * viewPosition;
}