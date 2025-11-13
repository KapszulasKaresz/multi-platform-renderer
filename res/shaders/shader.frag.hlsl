struct PixelInput
{
    float4 pos          : SV_Position;
    float3 normal       : NORMAL;
    float3 color        : COLOR0;
    float2 fragTexCoord : TEXCOORD0;
};

struct PixelOutput
{
    float4 attachment0 : SV_Target0;
};

cbuffer cb : register(b0)
{
    column_major float4x4 modelMatrix : packoffset(c0);
    column_major float4x4 viewMatrix : packoffset(c4);
    column_major float4x4 projectionMatrix : packoffset(c8);
};

Texture2D g_Texture : register(t0);

SamplerState TextureSampler : register(s0); 

PixelOutput main(PixelInput pixelInput)
{
    float3 inColor = pixelInput.color;
    PixelOutput output;
    output.attachment0 = g_Texture.Sample(TextureSampler, pixelInput.fragTexCoord);
    return output;
}