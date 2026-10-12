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

[vk::binding(0, 1)]
RaytracingAccelerationStructure scene;

[vk::binding(0, 0)]
ConstantBuffer<UniformBufferCamera> camera : register(b0, space0);

[vk::binding(0, 2)]
RWTexture2D<float4> outputImage;

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
void closestHitMain(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes)
{
    const uint materialId = InstanceID();
    const float3 color = float3(
        frac(float(materialId) * 0.731),
        frac(float(materialId) * 0.379 + 0.21),
        frac(float(materialId) * 0.527 + 0.43)
    );
    payload.color = float4(0.25 + 0.75 * color, 1.0);
}
