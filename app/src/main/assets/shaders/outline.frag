#version 300 es
precision mediump float;

//  VRM轮廓参数
uniform vec3 uOutlineColorFactor;        // 轮廓颜色因子
uniform float uOutlineLightingMixFactor; // 光照混合因子
uniform vec3 uLightDirection;            // 主光源方向
uniform vec3 uLightColor;                // 主光源颜色

// 从顶点着色器传入
in vec3 vWorldPosition;
in vec3 vViewPosition;

out vec4 FragColor;

void main() {
    //  基础轮廓颜色
    vec3 outlineColor = uOutlineColorFactor;

    //  如果启用光照混合
    if (uOutlineLightingMixFactor > 0.0) {
        // 计算表面法线（从世界坐标导数计算）
        vec3 normal = normalize(cross(dFdx(vWorldPosition), dFdy(vWorldPosition)));
        float lightDot = max(0.0, dot(normal, -uLightDirection));

        // 混合光照和基础颜色
        vec3 litColor = outlineColor * uLightColor * lightDot;
        outlineColor = mix(outlineColor, litColor, uOutlineLightingMixFactor);
    }

    //  边缘柔化（抗锯齿）
    float edgeFactor = 1.0;
    vec2 screenPos = gl_FragCoord.xy;
    float gradient = length(vec2(dFdx(screenPos.x), dFdy(screenPos.y)));
    if (gradient > 1.0) {
        edgeFactor = 1.0 / gradient;
    }

    FragColor = vec4(outlineColor, edgeFactor);
}