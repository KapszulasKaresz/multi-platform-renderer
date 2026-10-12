struct RayPayload
{
    float4 color;
};

struct UniformBufferCamera
{
    float4x4 view;
    float4x4 proj;
    float4x4 inverseView;
    float4x4 inverseProj;
};

[vk::binding(0, 2)]
RaytracingAccelerationStructure scene;

[vk::binding(0, 0)]
ConstantBuffer<UniformBufferCamera> camera : register(b0, space0);

[vk::binding(0, 6)]
RWTexture2D<float4> outputImage;

[vk::binding(0, 3)]
Sampler2D<float4> albedoTextures[];

[vk::binding(0, 4)]
Sampler2D<float4> normalTextures[];

[vk::binding(0, 5)]
Sampler2D<float4> metallicRoughnessTextures[];

[vk::binding(0, 1)]
ByteAddressBuffer geometry;

[shader("raygeneration")]
void raygenMain()
{
    RayPayload payload;
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
    outputImage[DispatchRaysIndex().xy] = payload.color;
}

[shader("miss")]
void missMain(inout RayPayload payload)
{
    payload.color = float4(0.1, 0.2, 0.4, 1.0);
}

[shader("closesthit")]
void closestHitMain(
    inout RayPayload payload,
    in BuiltInTriangleIntersectionAttributes attributes
)
{
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
    const float4 albedo = albedoTextures[materialId].SampleLevel(uv, 0.0);
    const float3 normal = normalTextures[materialId].SampleLevel(uv, 0.0).xyz * 2.0 - 1.0;
    const float4 metallicRoughness =
        metallicRoughnessTextures[materialId].SampleLevel(uv, 0.0);
    const float lighting = 0.5 + 0.5 * abs(normal.z);
    const float metallic = metallicRoughness.b;
    const float roughness = metallicRoughness.g;
    payload.color = float4(
        albedo.rgb * lighting * (1.0 - 0.25 * metallic) + normal * 0.05,
        albedo.a * (0.75 + 0.25 * (1.0 - roughness))
    );
}
