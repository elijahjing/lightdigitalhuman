// KHR_lights_punctual extension.
// see https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_lights_punctual
struct Light
{
    vec3 direction;
    float range;

    vec3 color;
    float intensity;

    vec3 position;
    float innerConeCos;

    float outerConeCos;
    int type;
};


const int LightType_Directional = 0;
const int LightType_Point = 1;
const int LightType_Spot = 2;


#ifdef USE_PUNCTUAL
uniform Light u_Lights[LIGHT_COUNT + 1]; //Array [0] is not allowed
#endif

#if defined(HAS_NORMAL_VEC3) && !defined(HAS_TANGENT_VEC4)
// === 环境光 ===
uniform vec3 u_GroundColor;
uniform vec3 u_SkyColor;
uniform float u_AmbientIntensity;
/**
 * @brief 计算环境光贡献
 */
/**
 * @brief 简单但视觉效果好的环境光
 */
vec3 calculateImprovedAmbient(vec3 baseColor) {
    // === 1. 双色调环境光 ===
    vec3 upColor = u_SkyColor;      // 天空色
    vec3 downColor = u_GroundColor; // 地面色

    // 根据法线Y分量进行插值
    float lerpFactor = dot(v_Normal, vec3(0, 1, 0)) * 0.5 + 0.5;
    vec3 ambientColor = mix(downColor, upColor, lerpFactor);

    // === 2. 避免颜色冲刷 ===
    // 使用叠加混合而不是简单相乘
    vec3 result = baseColor;

    // 计算环境光对亮度的贡献
    float ambientLuminance = dot(ambientColor, vec3(0.299, 0.587, 0.114));
    float baseLuminance = dot(baseColor, vec3(0.299, 0.587, 0.114));

    // 只增加亮度，不改变色相
    float luminanceBoost = ambientLuminance * u_AmbientIntensity;
    result = baseColor * (1.0 + luminanceBoost);

    // 轻微的色彩影响
    result = mix(result, result * ambientColor, 0.1);

    return result;
}
// 菲涅尔反射计算
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

/**
 * @brief 物理准确的环境光计算
 */
vec3 calculatePhysicalAmbient(vec3 baseColor, vec3 normal, float metallic, float roughness,vec3 viewDir) {
    // === 1. 方向性环境光（天空半球光照）===

    // 天空颜色 (上方)
    vec3 skyColor = u_SkyColor;
    // 地面颜色 (下方)
    vec3 groundColor = u_GroundColor;

    // 根据法线方向混合天空和地面颜色
    float hemisphereBlend = dot(normal, vec3(0, 1, 0)) * 0.5 + 0.5; // 0=地面, 1=天空
    vec3 environmentColor = mix(groundColor, skyColor, hemisphereBlend);

    // === 2. 基于材质属性的响应 ===

    // 金属材质对环境光的响应不同
    vec3 F0 = mix(vec3(0.04), baseColor, metallic);

    // 菲涅尔反射（视角相关）
    float cosTheta = max(dot(normal, viewDir), 0.0);
    vec3 F = fresnelSchlick(cosTheta, F0);

    // 漫反射部分（非金属材质主要贡献）
    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - metallic; // 金属没有漫反射

    // 最终环境光
    vec3 diffuse = environmentColor * baseColor * kD;
    vec3 specular = environmentColor * F * (1.0 - roughness); // 粗糙表面反射较少

    return (diffuse + specular) * u_AmbientIntensity;
}
#endif


// https://github.com/KhronosGroup/glTF/blob/master/extensions/2.0/Khronos/KHR_lights_punctual/README.md#range-property
float getRangeAttenuation(float range, float distance)
{
    if (range <= 0.0)
    {
        // negative range means unlimited
        return 1.0 / pow(distance, 2.0);
    }
    return max(min(1.0 - pow(distance / range, 4.0), 1.0), 0.0) / pow(distance, 2.0);
}


// https://github.com/KhronosGroup/glTF/blob/master/extensions/2.0/Khronos/KHR_lights_punctual/README.md#inner-and-outer-cone-angles
float getSpotAttenuation(vec3 pointToLight, vec3 spotDirection, float outerConeCos, float innerConeCos)
{
    float actualCos = dot(normalize(spotDirection), normalize(-pointToLight));
    if (actualCos > outerConeCos)
    {
        if (actualCos < innerConeCos)
        {
            float angularAttenuation = (actualCos - outerConeCos) / (innerConeCos - outerConeCos);
            return angularAttenuation * angularAttenuation;
        }
        return 1.0;
    }
    return 0.0;
}


vec3 getLighIntensity(Light light, vec3 pointToLight)
{
    float rangeAttenuation = 1.0;
    float spotAttenuation = 1.0;

    if (light.type != LightType_Directional)
    {
        rangeAttenuation = getRangeAttenuation(light.range, length(pointToLight));
    }
    if (light.type == LightType_Spot)
    {
        spotAttenuation = getSpotAttenuation(pointToLight, light.direction, light.outerConeCos, light.innerConeCos);
    }

    return rangeAttenuation * spotAttenuation * light.intensity * light.color;
}


vec3 getPunctualRadianceTransmission(vec3 normal, vec3 view, vec3 pointToLight, float alphaRoughness,
    vec3 baseColor, float ior)
{
    float transmissionRougness = applyIorToRoughness(alphaRoughness, ior);

    vec3 n = normalize(normal);           // Outward direction of surface point
    vec3 v = normalize(view);             // Direction from surface point to view
    vec3 l = normalize(pointToLight);
    vec3 l_mirror = normalize(l + 2.0*n*dot(-l, n));     // Mirror light reflection vector on surface
    vec3 h = normalize(l_mirror + v);            // Halfway vector between transmission light vector and v

    float D = D_GGX(clamp(dot(n, h), 0.0, 1.0), transmissionRougness);
    float Vis = V_GGX(clamp(dot(n, l_mirror), 0.0, 1.0), clamp(dot(n, v), 0.0, 1.0), transmissionRougness);

    // Transmission BTDF
    return baseColor * D * Vis;
}


vec3 getPunctualRadianceClearCoat(vec3 clearcoatNormal, vec3 v, vec3 l, vec3 h, float VdotH, vec3 f0, vec3 f90, float clearcoatRoughness)
{
    float NdotL = clampedDot(clearcoatNormal, l);
    float NdotV = clampedDot(clearcoatNormal, v);
    float NdotH = clampedDot(clearcoatNormal, h);
    return NdotL * BRDF_specularGGX(clearcoatRoughness * clearcoatRoughness, NdotL, NdotV, NdotH);
}


vec3 getPunctualRadianceSheen(vec3 sheenColor, float sheenRoughness, float NdotL, float NdotV, float NdotH)
{
    return NdotL * BRDF_specularSheen(sheenColor, sheenRoughness, NdotL, NdotV, NdotH);
}


// Compute attenuated light as it travels through a volume.
vec3 applyVolumeAttenuation(vec3 radiance, float transmissionDistance, vec3 attenuationColor, float attenuationDistance)
{
    if (attenuationDistance == 0.0)
    {
        // Attenuation distance is +∞ (which we indicate by zero), i.e. the transmitted color is not attenuated at all.
        return radiance;
    }
    else
    {
        // Compute light attenuation using Beer's law.
        vec3 transmittance = pow(attenuationColor, vec3(transmissionDistance / attenuationDistance));
        return transmittance * radiance;
    }
}


vec3 getVolumeTransmissionRay(vec3 n, vec3 v, float thickness, float ior, mat4 modelMatrix)
{
    // Direction of refracted light.
    vec3 refractionVector = refract(-v, normalize(n), 1.0 / ior);

    // Compute rotation-independant scaling of the model matrix.
    vec3 modelScale;
    modelScale.x = length(vec3(modelMatrix[0].xyz));
    modelScale.y = length(vec3(modelMatrix[1].xyz));
    modelScale.z = length(vec3(modelMatrix[2].xyz));

    // The thickness is specified in local space.
    return normalize(refractionVector) * thickness * modelScale;
}
