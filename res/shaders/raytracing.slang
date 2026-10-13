struct UniformBufferCamera
{
    float4x4 view;
    float4x4 proj;
    float4x4 inverseView;
    float4x4 inverseProj;
    float accumulationContribution;
    uint accumulationFrame;
};

struct UniformBufferLight
{
    float4 position;
    float4 emission;
};

struct UniformBufferLightArray
{
    int lightCount;
    UniformBufferLight lights[2];
    float4 sunlightDirection;
    float4 sunlightEmission;
};

struct RayPayload
{
    float4 color;
    float3 normal;
    float3 position;
    uint depth;
    uint visible;
    uint shadow;
};

[vk::binding(0, 1)]
ConstantBuffer<UniformBufferLightArray> lightArray : register(b1, space0);

[vk::binding(0, 3)]
RaytracingAccelerationStructure scene;

[vk::binding(0, 0)]
ConstantBuffer<UniformBufferCamera> camera : register(b0, space0);

[vk::binding(0, 7)]
RWTexture2D<float4> outputImage;

[vk::binding(0, 4)]
Sampler2D<float4> albedoTextures[64];

[vk::binding(0, 5)]
Sampler2D<float4> normalTextures[64];

[vk::binding(0, 6)]
Sampler2D<float4> metallicRoughnessTextures[64];

[vk::binding(0, 2)]
ByteAddressBuffer geometry;

// =========================================================================
// Math & PBR Helpers
// =========================================================================

static const float PI = 3.14159265359;
static const float INV_PI = 0.31830988618;

// Fast integer hash for Monte Carlo sampling jitter. Include the accumulation
// frame so every progressive-rendering iteration gets a new sample sequence.
uint initRandom(uint2 pixel, uint depth, uint frame)
{
    uint state = pixel.x * 1973u + pixel.y * 9277u + depth * 26699u
        + frame * 17431u;
    state ^= state >> 16u;
    state *= 2246822519u;
    state ^= state >> 13u;
    state *= 3266489917u;
    state ^= state >> 16u;
    return state == 0u ? 1u : state;
}

float randomFloat(inout uint state)
{
    state = state * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return float((word >> 22u) ^ word) / 4294967295.0;
}

float2 randomFloat2(inout uint state)
{
    return float2(randomFloat(state), randomFloat(state));
}

// Cosine-weighted hemisphere sampling for diffuse GI
float3 sampleCosineHemisphere(float2 u, float3 normal)
{
    float phi = 2.0 * PI * u.x;
    float cosTheta = sqrt(max(0.0, 1.0 - u.y));
    float sinTheta = sqrt(u.y);
    float3 local = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    float3 up = abs(normal.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, normal));
    float3 bitangent = cross(normal, tangent);

    return normalize(tangent * local.x + bitangent * local.y + normal * local.z);
}

// GGX / Trowbridge-Reitz normal distribution
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom + 0.000001);
}

// Smith-Schlick shadowing-masking function
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

// Fresnel-Schlick for direct specular highlights
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

// Fresnel-Schlick with roughness compensation for indirect specular
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

// ACES Filmic Tone Mapping Curve
float3 ACESFilm(float3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// =========================================================================
// Ray Generation Shader
// =========================================================================

[shader("raygeneration")]
void raygenMain()
{
    RayPayload payload;
    payload.depth = 0;
    payload.shadow = 0;
    payload.visible = 0;
    payload.color = float4(0.0, 0.0, 0.0, 1.0);

    const float2 pixel = (float2(DispatchRaysIndex().xy) + 0.5)
        / float2(DispatchRaysDimensions().xy);
    const float2 ndc = pixel * 2.0 - 1.0;
    const float4 origin = mul(camera.inverseView, float4(0.0, 0.0, 0.0, 1.0));
    const float4 target =
        mul(camera.inverseView, mul(camera.inverseProj, float4(ndc, 1.0, 1.0)));

    RayDesc ray;
    ray.Origin = origin.xyz / origin.w;
    ray.Direction = normalize(target.xyz / target.w - ray.Origin);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    TraceRay(
        scene,
        RAY_FLAG_FORCE_OPAQUE,
        0xFF,
        0,
        1,
        0,
        ray,
        payload
    );

    // Tone map HDR radiance and convert to sRGB display space
    float3 hdrColor = payload.color.rgb;
    float3 ldrColor = ACESFilm(hdrColor);
    float3 finalColor = pow(ldrColor, 1.0 / 2.2);

    const uint2 pixelIndex = DispatchRaysIndex().xy;
    const float4 currentColor = float4(finalColor, payload.color.a);
    if (camera.accumulationFrame == 0) {
        outputImage[pixelIndex] = currentColor;
    }
    else {
        const float contribution = saturate(camera.accumulationContribution);
        outputImage[pixelIndex] = lerp(
            outputImage[pixelIndex], currentColor, contribution
        );
    }
}

// =========================================================================
// Miss Shader: Procedural Atmospheric Sky
// =========================================================================

[shader("miss")]
void missMain(inout RayPayload payload)
{
    payload.visible = 1;

    const float3 dir = normalize(WorldRayDirection());
    const float3 sunDir = normalize(-lightArray.sunlightDirection.xyz);

    // Realistic sky gradient: deep zenith blue, warm horizon haze, dark ground
    const float cosTheta = dir.y;
    const float3 zenithColor  = float3(0.08, 0.28, 0.72);
    const float3 horizonColor = float3(0.70, 0.78, 0.88);
    const float3 groundColor  = float3(0.08, 0.07, 0.06);

    float3 sky = (cosTheta >= 0.0)
        ? lerp(horizonColor, zenithColor, pow(saturate(cosTheta), 0.65))
        : lerp(horizonColor, groundColor, pow(saturate(-cosTheta), 0.50));

    // Sun disc and atmospheric Mie glare
    const float sunDot = max(dot(dir, sunDir), 0.0);
    const float sunGlow = pow(sunDot, 16.0) * 0.40;
    const float sunDisc = pow(sunDot, 1024.0) * 6.0;
    const float3 sunRadiance = lightArray.sunlightEmission.rgb;

    payload.color = float4(sky + (sunGlow + sunDisc) * sunRadiance, 1.0);
    payload.normal = -dir;
    payload.position = WorldRayOrigin() + dir * 10000.0;
}

// =========================================================================
// Closest Hit Shader: PBR Lighting, Soft Shadows, Reflections & GI
// =========================================================================

[shader("closesthit")]
void closestHitMain(
    inout RayPayload payload,
    in BuiltInTriangleIntersectionAttributes attributes
)
{
    // Fast path: shadow ray hit an obstacle
    if (payload.shadow != 0) {
        payload.visible = 0;
        return;
    }

    // Geometry unpack
    const uint instanceData = InstanceID();
    const uint meshId = instanceData >> 12;
    const uint materialId = instanceData & 0xFFF;
    const uint meshHeader = 8 + meshId * 16;
    const uint vertexOffset = geometry.Load(meshHeader);
    const uint indexOffset = geometry.Load(meshHeader + 4);
    const uint primitiveOffset = indexOffset + PrimitiveIndex() * 3;
    const uint uvBase = 8 + geometry.Load(0) * 16;
    const uint indexBase = uvBase + geometry.Load(4) * 8;
    const uint index0 = geometry.Load(indexBase + primitiveOffset * 4);
    const uint index1 = geometry.Load(indexBase + (primitiveOffset + 1) * 4);
    const uint index2 = geometry.Load(indexBase + (primitiveOffset + 2) * 4);
    const float2 uv0 = asfloat(geometry.Load2(uvBase + (vertexOffset + index0) * 8));
    const float2 uv1 = asfloat(geometry.Load2(uvBase + (vertexOffset + index1) * 8));
    const float2 uv2 = asfloat(geometry.Load2(uvBase + (vertexOffset + index2) * 8));
    const float2 uv = uv0 * (1.0 - attributes.barycentrics.x - attributes.barycentrics.y)
        + uv1 * attributes.barycentrics.x + uv2 * attributes.barycentrics.y;

    // Textures & Material properties
    const float4 rawAlbedo = albedoTextures[materialId].SampleLevel(uv, 0.0);
    const float3 albedo = pow(rawAlbedo.rgb, 2.2); // Linearize sRGB texture

    float3 normal = normalize(
        normalTextures[materialId].SampleLevel(uv, 0.0).xyz * 2.0 - 1.0
    );

    const float4 metallicRoughness = metallicRoughnessTextures[materialId].SampleLevel(uv, 0.0);
    const float metallic = saturate(metallicRoughness.b);
    const float roughness = clamp(metallicRoughness.g, 0.04, 1.0); // 0.04 avoids GGX divide-by-zero

    const float3 hitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    const float3 V = -normalize(WorldRayDirection());

    // Ensure normal is oriented towards the incoming ray
    if (dot(normal, V) < 0.0) {
        normal = -normal;
    }

    const float NdotV = max(dot(normal, V), 0.0001);
    const float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

    uint seed = initRandom(
        DispatchRaysIndex().xy, payload.depth, camera.accumulationFrame
    );
    float3 directRadiance = float3(0.0, 0.0, 0.0);

    // ---------------------------------------------------------------------
    // 1. Direct Lighting: Point Lights (Inverse-Square + Soft Shadows)
    // ---------------------------------------------------------------------
    const int activeLights = lightArray.lightCount;
    for (int i = 0; i < activeLights; ++i) {
        const float3 toLight = lightArray.lights[i].position.xyz - hitPosition;
        const float distanceToLight = length(toLight);
        if (distanceToLight <= 0.001) continue;

        // Jitter light position for soft penumbra
        const float3 lightPos = lightArray.lights[i].position.xyz
            + (sampleCosineHemisphere(randomFloat2(seed), normal) - normal) * 0.04;
        const float3 lightDirection = normalize(lightPos - hitPosition);
        const float NdotL = max(dot(normal, lightDirection), 0.0);

        if (NdotL > 0.0) {
            RayPayload shadow;
            shadow.shadow = 1;
            shadow.visible = 0;

            RayDesc shadowRay;
            shadowRay.Origin = hitPosition + normal * (0.002 + 0.002 * (1.0 - NdotL));
            shadowRay.Direction = lightDirection;
            shadowRay.TMin = 0.001;
            shadowRay.TMax = distanceToLight - 0.005;

            TraceRay(
                scene,
                RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
                0xFF, 0, 1, 0,
                shadowRay, shadow
            );

            if (shadow.visible != 0) {
                const float attenuation = 1.0 / max(distanceToLight * distanceToLight, 0.01);
                const float3 radiance = lightArray.lights[i].emission.rgb * attenuation;

                // Cook-Torrance BRDF
                const float3 H = normalize(V + lightDirection);
                const float D = DistributionGGX(normal, H, roughness);
                const float G = GeometrySmith(normal, V, lightDirection, roughness);
                const float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

                const float3 specular = (D * G * F) / (4.0 * NdotV * NdotL + 0.0001);
                const float3 kD = (1.0 - F) * (1.0 - metallic);
                const float3 diffuse = kD * albedo * INV_PI;

                directRadiance += (diffuse + specular) * radiance * NdotL;
            }
        }
    }

    // ---------------------------------------------------------------------
    // 2. Direct Lighting: Sunlight (Directional + Penumbra)
    // ---------------------------------------------------------------------
    const float3 sunDirection = normalize(-lightArray.sunlightDirection.xyz);
    const float2 sunJitter = (randomFloat2(seed) - 0.5) * 0.025;
    const float3 sunUp = abs(sunDirection.y) < 0.999 ? float3(0.0, 1.0, 0.0) : float3(1.0, 0.0, 0.0);
    const float3 sunTangent = normalize(cross(sunUp, sunDirection));
    const float3 sunBitangent = cross(sunDirection, sunTangent);
    const float3 jitteredSunDir = normalize(sunDirection + sunTangent * sunJitter.x + sunBitangent * sunJitter.y);

    const float sunNdotL = max(dot(normal, jitteredSunDir), 0.0);
    if (sunNdotL > 0.0) {
        RayPayload sunShadow;
        sunShadow.shadow = 1;
        sunShadow.visible = 0;

        RayDesc sunRay;
        sunRay.Origin = hitPosition + normal * (0.002 + 0.002 * (1.0 - sunNdotL));
        sunRay.Direction = jitteredSunDir;
        sunRay.TMin = 0.001;
        sunRay.TMax = 10000.0;

        TraceRay(
            scene,
            RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
            0xFF, 0, 1, 0,
            sunRay, sunShadow
        );

        if (sunShadow.visible != 0) {
            const float3 sunRadiance = lightArray.sunlightEmission.rgb;
            const float3 H = normalize(V + jitteredSunDir);
            const float D = DistributionGGX(normal, H, roughness);
            const float G = GeometrySmith(normal, V, jitteredSunDir, roughness);
            const float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

            const float3 specular = (D * G * F) / (4.0 * NdotV * sunNdotL + 0.0001);
            const float3 kD = (1.0 - F) * (1.0 - metallic);
            const float3 diffuse = kD * albedo * INV_PI;

            directRadiance += (diffuse + specular) * sunRadiance * sunNdotL;
        }
    }

    // ---------------------------------------------------------------------
    // 3. Indirect Lighting: Specular Reflections & Diffuse GI
    // ---------------------------------------------------------------------
    float3 indirectRadiance = float3(0.0, 0.0, 0.0);

    if (payload.depth == 0) {
        // Specular Reflection (Mirror / Glossy Lobe)
        const float3 R = reflect(-V, normal);
        const float3 jitterDir = sampleCosineHemisphere(randomFloat2(seed), normal);
        const float3 reflDir = normalize(lerp(R, jitterDir, roughness * roughness));

        if (dot(reflDir, normal) > 0.0) {
            RayPayload reflPayload;
            reflPayload.depth = 1;
            reflPayload.shadow = 0;
            reflPayload.visible = 0;
            reflPayload.color = float4(0.0, 0.0, 0.0, 1.0);

            RayDesc reflRay;
            reflRay.Origin = hitPosition + normal * 0.003;
            reflRay.Direction = reflDir;
            reflRay.TMin = 0.001;
            reflRay.TMax = 10000.0;

            TraceRay(scene, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 1, 0, reflRay, reflPayload);

            const float3 F_refl = FresnelSchlickRoughness(NdotV, F0, roughness);
            indirectRadiance += F_refl * reflPayload.color.rgb;
        }

        // Diffuse Indirect GI (Color Bleeding & Contact Ambient Occlusion)
        if (metallic < 0.98) {
            const float3 giDir = sampleCosineHemisphere(randomFloat2(seed), normal);

            RayPayload giPayload;
            giPayload.depth = 1;
            giPayload.shadow = 0;
            giPayload.visible = 0;
            giPayload.color = float4(0.0, 0.0, 0.0, 1.0);

            RayDesc giRay;
            giRay.Origin = hitPosition + normal * 0.003;
            giRay.Direction = giDir;
            giRay.TMin = 0.001;
            giRay.TMax = 10000.0;

            TraceRay(scene, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 1, 0, giRay, giPayload);

            const float3 F_ambient = FresnelSchlickRoughness(NdotV, F0, roughness);
            const float3 kD_ambient = (1.0 - F_ambient) * (1.0 - metallic);
            indirectRadiance += kD_ambient * albedo * giPayload.color.rgb;
        }
    } else {
        // Secondary bounce ambient fallback
        indirectRadiance = albedo * 0.04;
    }

    payload.color = float4(directRadiance + indirectRadiance, rawAlbedo.a);
    payload.normal = normal;
    payload.position = hitPosition;
}