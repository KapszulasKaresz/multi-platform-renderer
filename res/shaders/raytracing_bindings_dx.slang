ConstantBuffer<UniformBufferCamera> camera : register(b0, space0);
ConstantBuffer<UniformBufferLightArray> lightArray : register(b1, space0);
ByteAddressBuffer geometry : register(t0, space2);
RaytracingAccelerationStructure scene : register(t0, space3);
Texture2D<float4> albedoTextures[128] : register(t0, space4);
Texture2D<float4> normalTextures[128] : register(t0, space5);
Texture2D<float4> metallicRoughnessTextures[128] : register(t0, space6);
SamplerState albedoSamplers[128] : register(s0, space4);
SamplerState normalSamplers[128] : register(s0, space5);
SamplerState metallicRoughnessSamplers[128] : register(s0, space6);
RWTexture2D<float4> outputImage : register(u0, space7);

float2 rayTracingNdc(float2 pixel)
{
    return float2(pixel.x * 2.0 - 1.0, 1.0 - pixel.y * 2.0);
}

float4 sampleAlbedo(uint index, float2 uv, float lod)
{
    return albedoTextures[index].SampleLevel(albedoSamplers[index], uv, lod);
}

float4 sampleNormal(uint index, float2 uv, float lod)
{
    return normalTextures[index].SampleLevel(normalSamplers[index], uv, lod);
}

float4 sampleMetallicRoughness(uint index, float2 uv, float lod)
{
    return metallicRoughnessTextures[index].SampleLevel(
        metallicRoughnessSamplers[index], uv, lod
    );
}
