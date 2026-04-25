struct VSInput {
    float3 inPosition : POSITION;
    float3 inNormal   : NORMAL;
    float2 inTexCoord : TEXCOORD0;
    float3 inColor    : COLOR0;
};

struct VSOutput
{
    float4 pos          : SV_Position;
    float3 normal       : NORMAL;
    float3 color        : COLOR0;
    float2 fragTexCoord : TEXCOORD0;
};

struct UniformBufferCamera
{
    float4x4 view;
    float4x4 proj;
};

struct UniformBufferObject
{
    float4x4 model;
};

[vk::binding(0, 0)]
ConstantBuffer<UniformBufferCamera> camera : register(b0, space0);

[vk::binding(0, 1)]
ConstantBuffer<UniformBufferObject> object : register(b1, space0);

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    output.pos = mul(camera.proj, mul(camera.view, mul(object.model, float4(input.inPosition, 1.0))));
    output.normal = input.inNormal;
    output.color = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    return output;
}

[vk::binding(1, 1)]
Sampler2D texture : register(t0, space0);

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    return texture.Sample(vertIn.fragTexCoord);
}