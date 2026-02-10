#version 300 es
precision mediump float;

#define PI 3.141592653589793
#define RECIPROCAL_PI 0.3183098861837907

in vec3 vWorldPosition;
in vec3 vViewPosition;
in vec3 vWorldNormal;
in vec2 vUV;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vViewNormal;

out vec4 FragColor;

//  基础材质参数
uniform vec3 litFactor;
uniform vec3 shadeColorFactor;
uniform float shadingShiftFactor;
uniform float shadingToonyFactor;
uniform float lightColorAttenuationFactor;
uniform float indirectLightIntensityFactor;

//  边缘光参数
uniform vec3 rimColorFactor;
uniform float rimLightingMixFactor;
uniform float rimFresnelPowerFactor;
uniform float rimLiftFactor;

//  发光参数
uniform vec3 emissiveFactor;
uniform float emissiveIntensity;

//  光照参数
uniform vec3 directionalLightDirection;
uniform vec3 directionalLightColor;
uniform vec3 ambientLightColor;

//  纹理
uniform sampler2D map;
uniform sampler2D shadeMultiplyTexture;
uniform sampler2D shadingShiftTexture;
uniform sampler2D rimMultiplyTexture;
uniform sampler2D aoMap;
uniform sampler2D emissiveTexture;
uniform sampler2D matcapTexture;
uniform sampler2D normalTexture;

//  纹理标志
uniform int hasMap;
uniform int hasShadeMultiplyTexture;
uniform int hasShadingShiftTexture;
uniform int hasRimMultiplyTexture;
uniform int hasAoMap;
uniform int hasEmissiveTexture;
uniform int hasMatcapTexture;
uniform int hasNormalTexture;

//  在片段着色器中：
uniform int isOutlinePass;
uniform vec3 outlineColorFactor;

uniform float alphaCutoff;
#define ALPHA_MODE_OPAQUE 0
#define ALPHA_MODE_MASK 1
#define ALPHA_MODE_BLEND 2
uniform int alphaMode;

vec4 sRGBTransferOETF(in vec4 value) {
    return vec4(mix(pow(value.rgb, vec3(0.41666)) * 1.055 - vec3(0.055),
    value.rgb * 12.92,
    vec3(lessThanEqual(value.rgb, vec3(0.0031308)))), value.a);
}

vec4 linearToOutputTexel(vec4 value) {
    return sRGBTransferOETF(value);
}

//  MToon特有的linearstep函数
float linearstep(float a, float b, float t) {
    return clamp((t - a) / (b - a), 0.0, 1.0);
}

//  Lambert BRDF
vec3 BRDF_Lambert(const in vec3 diffuseColor) {
    return RECIPROCAL_PI * diffuseColor;
}
//  获取法线（支持法线贴图）
vec3 getNormal() {
    vec3 normal = normalize(vWorldNormal);

    if (hasNormalTexture == 1) {
        vec3 normalMap = texture(normalTexture, vUV).rgb * 2.0 - 1.0;
        mat3 TBN = mat3(
        normalize(vTangent),
        normalize(vBitangent),
        normal
        );
        normal = normalize(TBN * normalMap);
    }

    return normal;
}


//  获取基础颜色
vec3 getBaseColor() {
    vec3 baseColor = litFactor;

    if (hasMap == 1) {
        vec4 texColor = texture(map, vUV);
        baseColor = texColor.rgb * litFactor;
    }

    return baseColor;
}

//  获取阴影颜色
vec3 getShadeColor(vec3 baseColor) {
    vec3 shadeColor = baseColor * shadeColorFactor;

    if (hasShadeMultiplyTexture == 1) {
        vec3 shadeMultiply = texture(shadeMultiplyTexture, vUV).rgb;
        shadeColor *= shadeMultiply;
    }

    return shadeColor;
}

//  获取阴影偏移
float getShadingShift() {
    float shift = shadingShiftFactor;

    if (hasShadingShiftTexture == 1) {
        float shiftTex = texture(shadingShiftTexture, vUV).r;
        shift += (shiftTex - 0.5) * 2.0;
    }

    return shift;
}

//  计算主光照
vec3 calculateMainLighting(vec3 baseColor, vec3 shadeColor, vec3 normal) {
    vec3 lightDir = normalize(-directionalLightDirection);
    float NdotL = dot(normal, lightDir);

    // 应用阴影偏移
    float shadingShift = getShadingShift();
    float shiftedNdotL = NdotL + shadingShift;

    // 卡通化处理
    float toonSmoothness = (1.0 - shadingToonyFactor) * 0.5;
    float toonShading = smoothstep(-toonSmoothness, toonSmoothness, shiftedNdotL);

    // 混合明暗颜色
    vec3 litColor = baseColor * directionalLightColor * lightColorAttenuationFactor;
    vec3 shadedColor = mix(shadeColor, litColor, toonShading);

    return shadedColor;
}

float getShading(
const in float dotNL,
const in float shadow,
const in float shadingShift
) {
    float shading = dotNL;
    shading = shading + shadingShift;
    shading = linearstep(-1.0 + shadingToonyFactor, 1.0 - shadingToonyFactor, shading);
    shading *= shadow;
    return shading;
}

vec3 getDiffuse(
vec3 baseColor,
vec3 shadeColor,
const in float shading,
in vec3 lightColor
) {

    vec3 col = lightColor * BRDF_Lambert(mix(shadeColorFactor, litFactor, shading));
    return col;
}

//  计算主光照（MToon方式）
vec3 calculateMainLighting2(vec3 baseColor, vec3 shadeColor, vec3 normal) {
    vec3 lightDir = normalize(-directionalLightDirection);
    float dotNL = clamp(dot(normal, lightDir), -1.0, 1.0);

    // 简化的阴影，实际项目中应该传入阴影贴图
    float shadow = 1.0;
    float shadingShift = getShadingShift();

    float shading = getShading(dotNL, shadow, shadingShift);

    return getDiffuse(baseColor, shadeColor, shading, directionalLightColor);
}

//  计算环境光遮蔽
float getAmbientOcclusion() {
    float ao = 1.0;

    if (hasAoMap == 1) {
        ao = texture(aoMap, vUV).r;
    }

    return ao;
}

// 计算间接光照
vec3 calculateIndirectLighting(vec3 baseColor) {
    vec3 indirectLighting = ambientLightColor * baseColor * indirectLightIntensityFactor;

    // 应用AO
    float ao = getAmbientOcclusion();
    indirectLighting *= ao;

    return indirectLighting;
}

//  计算边缘光
vec3 calculateRimLighting(vec3 normal) {
    vec3 viewDir = normalize(-vViewPosition);
    float NdotV = dot(normal, viewDir);

    // 菲涅尔计算
    float rim = pow(1.0 - max(NdotV, 0.0), rimFresnelPowerFactor);
    rim = max(rim - rimLiftFactor, 0.0);

    // 应用边缘光纹理
    if (hasRimMultiplyTexture == 1) {
        float rimTex = texture(rimMultiplyTexture, vUV).r;
        rim *= rimTex;
    }

    // 边缘光颜色
    vec3 rimColor = rimColorFactor * rim;

    // 光照混合
    if (rimLightingMixFactor > 0.0) {
        vec3 lightDir = normalize(-directionalLightDirection);
        float NdotL = max(dot(normal, lightDir), 0.0);
        vec3 rimLighting = rimColor * directionalLightColor * NdotL;
        rimColor = mix(rimColor, rimLighting, rimLightingMixFactor);
    }

    return rimColor;
}

//  计算Matcap
vec3 calculateMatcap(vec3 normal) {
    if (hasMatcapTexture != 1) {
        return vec3(0.0);
    }

    // 将法线转换到视图空间
    vec3 viewNormal = normalize(vViewNormal);

    // 计算Matcap UV
    vec2 matcapUV = viewNormal.xy * 0.5 + 0.5;

    // 采样Matcap纹理
    vec3 matcapColor = texture(matcapTexture, matcapUV).rgb;

    return matcapColor;
}

//  计算发光
vec3 calculateEmissive() {
    vec3 emissive = emissiveFactor;

    if (hasEmissiveTexture == 1) {
        vec3 emissiveTex = texture(emissiveTexture, vUV).rgb;
        emissive *= emissiveTex;
    }

    return emissive * emissiveIntensity;
}

//  计算基于几何的环境遮蔽
float calculateGeometricAO(vec3 normal) {
    //  基于法线方向的遮蔽
    vec3 upVector = vec3(0.0, 1.0, 0.0);
    vec3 downVector = vec3(0.0, -1.0, 0.0);
    vec3 viewDir = normalize(-vViewPosition);

    //  向下的面（如眼眶上方）更暗
    float downFacing = max(dot(normal, downVector), 0.0);
    float upFacing = max(dot(normal, upVector), 0.0);

    //  背向观察者的面更暗
    float viewFacing = max(dot(normal, viewDir), 0.0);

    // 组合遮蔽
    float geometricAO = 1.0;
    geometricAO -= downFacing * 0.4;// 向下的面暗40%
    geometricAO -= (1.0 - viewFacing) * 0.3;// 背向的面暗30%

    return max(geometricAO, 0.2);// 最暗不超过20%
}

void main() {
    //  1. 首先获取基础纹理和alpha（在所有计算之前）
    float materialAlpha = 1.0;
    if (hasMap == 1) {
        vec4 baseTexture = texture(map, vUV);
        materialAlpha = baseTexture.a;

        // 立即进行alpha测试
        if (alphaMode == 1 && materialAlpha < alphaCutoff) { // ALPHA_MODE_MASK
            discard;
        }
    }
    // 获取基础参数

    vec3 normal = getNormal();
    vec3 baseColor = getBaseColor();
    vec3 shadeColor = getShadeColor(baseColor);

    // 计算主光照
    vec3 mainLighting = calculateMainLighting(baseColor, shadeColor, normal);

    // 计算间接光照
    vec3 indirectLighting = calculateIndirectLighting(baseColor);

    // 计算边缘光
    vec3 rimLighting = calculateRimLighting(normal);

    // 计算Matcap
    vec3 matcapColor = calculateMatcap(normal);

    // 计算发光
    vec3 emissive = calculateEmissive();

    //  最终颜色合成
    vec3 finalColor = mainLighting + indirectLighting + rimLighting + matcapColor + emissive;

    // 获取透明度
    float alpha = 1.0;
    if (hasMap == 1) {
        alpha = texture(map, vUV).a;
    }
    FragColor = vec4(finalColor, alpha);
}