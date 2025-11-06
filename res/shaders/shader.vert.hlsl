cbuffer cb : register(b0)
{
    row_major float4x4 modelMatrix : packoffset(c0);
    row_major float4x4 viewMatrix : packoffset(c4);
    row_major float4x4 projectionMatrix : packoffset(c8);
};

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
    float4 worldPos = mul(float4(input.inPosition, 1.0), modelMatrix);
    float4 viewPos  = mul(worldPos, viewMatrix);
    output.pos      = mul(viewPos, projectionMatrix);
    output.normal   = input.inNormal;
    output.color    = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    return output;
}