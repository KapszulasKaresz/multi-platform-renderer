struct UniformBufferCamera {
    row_major float4x4 view;
    row_major float4x4 proj;
};

struct UniformBufferObject {
    row_major float4x4 model;
};

ConstantBuffer<UniformBufferCamera> camera : register(b0, space0);
ConstantBuffer<UniformBufferObject> object : register(b1, space0);

struct VertexInput
{
    float3 inPosition : POSITION;
    float3 inNormal : NORMAL;
    float2 inTexCoord : TEXCOORD0;
    float3 inColor : COLOR0;
};

struct VSOutput
{
    float4 pos          : SV_Position;
    float3 normal       : NORMAL;
    float3 color        : COLOR0;
    float2 fragTexCoord : TEXCOORD0;
};

VSOutput main(VertexInput input)
{
    VSOutput output;
    float4 worldPos = mul(float4(input.inPosition, 1.0), object.model);
    float4 viewPos  = mul(worldPos, camera.view);
    output.pos      = mul(viewPos, camera.proj);
    output.normal   = input.inNormal;
    output.color    = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    return output;
}