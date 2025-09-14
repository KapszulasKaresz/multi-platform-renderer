struct VSInput {
    float3 inPosition;
    float3 inNormal;
    float2 inTexCoord;
    float3 inColor;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 normal : NORMAL;
    float3 color : COLOR;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    output.pos = float4(input.inPosition, 1.0);
    output.normal = input.inNormal;
    output.color = input.inColor;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    return float4(vertIn.color, 1.0);
}