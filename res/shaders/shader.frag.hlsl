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
    row_major float4x4 modelMatrix : packoffset(c0);
    row_major float4x4 viewMatrix : packoffset(c4);
    row_major float4x4 projectionMatrix : packoffset(c8);
};

Texture2D g_Texture : register(t0);

SamplerState TextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

PixelOutput main(PixelInput pixelInput)
{
    float3 inColor = pixelInput.color;
    PixelOutput output;
    output.attachment0 = g_Texture.Sample(TextureSampler, pixelInput.fragTexCoord);
    return output;
}