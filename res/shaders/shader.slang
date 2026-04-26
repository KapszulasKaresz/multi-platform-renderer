struct VSInput {
    float3 inPosition : POSITION;
    float3 inNormal   : NORMAL;
    float2 inTexCoord : TEXCOORD0;
    float3 inColor    : COLOR0;
};

struct VSOutput
{
    float4 pos          : SV_Position;
    float3 worldPos     : POSITION0;
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

struct UniformBufferLight
{
    float4 position;
    float4 color;
};

struct UniformBufferLightCount
{
    int lightCount;
};

[vk::binding(0, 0)]
ConstantBuffer<UniformBufferCamera> camera : register(b0, space0);

[vk::binding(0, 1)]
ConstantBuffer<UniformBufferObject> object : register(b1, space0);

[vk::binding(0, 2)]
ConstantBuffer<UniformBufferLightCount> lightCount : register(b2, space0);

[vk::binding(1, 2)]
ConstantBuffer<UniformBufferLight> lights[10] : register(b2, space1);

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    float4 worldPosition = mul(object.model, float4(input.inPosition, 1.0));
    output.worldPos = worldPosition.xyz;
    
    output.pos = mul(camera.proj, mul(camera.view, worldPosition));
    output.normal = normalize(mul((float3x3)object.model, input.inNormal));
    output.color = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    return output;
}

[vk::binding(1, 1)]
Sampler2D texture : register(t0, space0);

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {

    float3 baseColor = texture.Sample(vertIn.fragTexCoord).rgb;

    float3 normal = normalize(vertIn.normal);
    
    float3 ambientLight = float3(0.1, 0.1, 0.1); 
    float3 diffuseLight = float3(0.0, 0.0, 0.0);
    
    for (int i = 0; i < lightCount.lightCount; ++i) {
        UniformBufferLight light = lights[i];
        
        float3 lightDir = normalize(light.position.xyz - vertIn.worldPos);
        
        float diff = max(dot(normal, lightDir), 0.0);
        
        diffuseLight += diff * light.color.rgb;
    }
    
    float3 finalColor = baseColor * (ambientLight + diffuseLight);
    
    return float4(finalColor, 1.0);
}