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

PixelOutput main(PixelInput pixelInput)
{
    float3 inColor = pixelInput.color;
    PixelOutput output;
    output.attachment0 = float4(inColor, 1.0f);
    return output;
}