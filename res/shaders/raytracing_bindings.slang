[vk::binding(0, 1)]
ConstantBuffer<UniformBufferLightArray> lightArray : register(b1, space0);

[vk::binding(0, 3)]
RaytracingAccelerationStructure scene;

[vk::binding(0, 0)]
ConstantBuffer<UniformBufferCamera> camera : register(b0, space0);

[vk::binding(0, 7)]
RWTexture2D<float4> outputImage;

[vk::binding(0, 4)]
Sampler2D<float4> albedoTextures[128];

[vk::binding(0, 5)]
Sampler2D<float4> normalTextures[128];

[vk::binding(0, 6)]
Sampler2D<float4> metallicRoughnessTextures[128];

[vk::binding(0, 2)]
ByteAddressBuffer geometry;

float2 rayTracingNdc(float2 pixel)
{
    return pixel * 2.0 - 1.0;
}

float4 sampleAlbedo(uint index, float2 uv, float lod)
{
    return albedoTextures[index].SampleLevel(uv, lod);
}

float4 sampleNormal(uint index, float2 uv, float lod)
{
    return normalTextures[index].SampleLevel(uv, lod);
}

float4 sampleMetallicRoughness(uint index, float2 uv, float lod)
{
    return metallicRoughnessTextures[index].SampleLevel(uv, lod);
}
