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
    payload.color = float4(0.8, 0.8, 0.8, 1.0);
}
