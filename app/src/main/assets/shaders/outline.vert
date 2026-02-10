#version 300 es
precision mediump float;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat3 uNormalMatrix;

// VRM轮廓参数
uniform float uOutlineWidthFactor;
uniform int uOutlineWidthMode;

// 传递给片段着色器
out vec3 vWorldPosition;
out vec3 vViewPosition;

void main() {
    // 计算基本变换
    vec4 worldPos = uModelMatrix * vec4(aPosition, 1.0);
    vec3 worldNormal = normalize(uNormalMatrix * aNormal);
    vec4 viewPos = uViewMatrix * worldPos;

    vWorldPosition = worldPos.xyz;
    vViewPosition = viewPos.xyz;

    //  根据VRM轮廓模式处理
    if (uOutlineWidthMode == 0) {
        //  模式0：世界坐标模式 - 在视空间中外扩
        vec3 viewNormal = normalize((uViewMatrix * vec4(worldNormal, 0.0)).xyz);
        viewPos.xyz += viewNormal * uOutlineWidthFactor;
        gl_Position = uProjectionMatrix * viewPos;

    } else {
        //  模式1：屏幕坐标模式 - 在屏幕空间中外扩
        vec4 clipPos = uProjectionMatrix * viewPos;
        vec4 clipNormal = uProjectionMatrix * vec4(normalize((uViewMatrix * vec4(worldNormal, 0.0)).xyz), 0.0);

        // 归一化屏幕空间法线
        vec2 screenNormal = normalize(clipNormal.xy);

        // 在屏幕空间中外扩（固定像素宽度）
        float screenSpaceWidth = uOutlineWidthFactor * 0.001; // 调整系数
        clipPos.xy += screenNormal * screenSpaceWidth * clipPos.w;

        gl_Position = clipPos;
    }
}