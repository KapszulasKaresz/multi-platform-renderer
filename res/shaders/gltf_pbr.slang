struct VSInput {
    float3 inPosition : POSITION;
    float3 inNormal   : NORMAL;
    float4 inTangent  : TANGENT;
    float2 inTexCoord : TEXCOORD0;
    float3 inColor    : COLOR0;
};

struct VSOutput
{
    float4 pos          : SV_Position;
    float3 worldPos     : POSITION0;
    float3 normal       : NORMAL;
    float4 tangent      : TANGENT;
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
    float4 emission;
};

struct UniformBufferLightCount
{
    int lightCount;
};

struct UniformBufferLightArray
{
    int lightCount;
    UniformBufferLight lights[3];
    float4 sunlightDirection;
    float4 sunlightEmission;
};

[vk::binding(0, 0)]
ConstantBuffer<UniformBufferCamera> camera : register(b0, space0);

[vk::binding(0, 1)]
ConstantBuffer<UniformBufferObject> object : register(b1, space0);

[vk::binding(0, 2)]
ConstantBuffer<UniformBufferLightArray> lightArray : register(b2, space0);

[vk::binding(1, 1)]
Sampler2D texture : register(t0, space0);

[vk::binding(2, 1)]
Sampler2D textureMetallicRoughness : register(t1, space0);

[vk::binding(3, 1)]
Sampler2D textureNormal : register(t2, space0);


[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    
    float4 worldPosition = mul(object.model, float4(input.inPosition, 1.0));
    output.worldPos = worldPosition.xyz;
    
    output.pos = mul(camera.proj, mul(camera.view, worldPosition));
    
    output.normal = normalize(mul((float3x3)object.model, input.inNormal));
    
    output.tangent.xyz = normalize(mul((float3x3)object.model, input.inTangent.xyz));
    
    output.tangent.w = input.inTangent.w;
    
    output.color = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    
    return output;
}


[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {

    float3 baseColor = texture.Sample(vertIn.fragTexCoord).rgb;

    float3 N = normalize(vertIn.normal);
    float3 T = normalize(vertIn.tangent.xyz);
    
    T = normalize(T - dot(T, N) * N);
    
    float3 B = cross(N, T) * vertIn.tangent.w;
    
    float3x3 TBN = float3x3(T, B, N);
    
    float3 sampledNormal = textureNormal.Sample(vertIn.fragTexCoord).rgb;
    sampledNormal = sampledNormal * 2.0 - 1.0;
    
    float3 finalNormal = normalize(mul(sampledNormal, TBN));

    float3 mrSample = textureMetallicRoughness.Sample(vertIn.fragTexCoord).rgb;
    float roughness = mrSample.g; 
    float metallic = mrSample.b;  

    float3x3 viewRot = (float3x3)camera.view;
    float3 viewTrans = float3(camera.view[0][3], camera.view[1][3], camera.view[2][3]);
    float3 camPos = -mul(transpose(viewRot), viewTrans);
    float3 viewDir = normalize(camPos - vertIn.worldPos);

    float3 diffuseColor = baseColor * (1.0 - metallic);
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), baseColor, metallic);

    float3 ambientLight = float3(0.1, 0.1, 0.1) * baseColor; 
    float3 diffuseLight = float3(0.0, 0.0, 0.0);
    float3 specularLight = float3(0.0, 0.0, 0.0);
    const float specPower = exp2(10.0 * (1.0 - roughness) + 1.0);
    
    const int activeLightCount = min(lightArray.lightCount, 3);
    for (int i = 0; i < activeLightCount; ++i) {
        UniformBufferLight light = lightArray.lights[i];
        
        float3 lightDir;
        float attenuation = 1.0;
        
        if (light.position.w < 0.5) {
            // Directional Light: light.position.xyz is direction towards the light source
            lightDir = normalize(light.position.xyz);
        } else {
            // Point Light: vector to light source and inverse-square distance attenuation
            float3 lightVec = light.position.xyz - vertIn.worldPos;
            float dist = length(lightVec);
            
            lightDir = lightVec / max(dist, 0.0001); // Prevent divide-by-zero
            attenuation = 1.0 / (dist * dist);
        }
        
        float diff = max(dot(finalNormal, lightDir), 0.0);
        
        float3 halfDir = normalize(lightDir + viewDir);
        float NdotH = max(dot(finalNormal, halfDir), 0.0);
        
        float spec = pow(NdotH, specPower);
        
        diffuseLight += diff * light.emission.rgb * diffuseColor * attenuation;
        specularLight += spec * light.emission.rgb * F0 * diff * attenuation;
    }

    const float3 sunlightDirection =
        normalize(-lightArray.sunlightDirection.xyz);
    const float sunlightDiffuse = max(dot(finalNormal, sunlightDirection), 0.0);
    const float3 sunlightHalfDir = normalize(sunlightDirection + viewDir);
    const float sunlightNdotH =
        max(dot(finalNormal, sunlightHalfDir), 0.0);
    const float sunlightSpec =
        pow(sunlightNdotH, specPower);
    diffuseLight += sunlightDiffuse
        * lightArray.sunlightEmission.rgb * diffuseColor;
    specularLight += sunlightSpec
        * lightArray.sunlightEmission.rgb * F0 * sunlightDiffuse;
    
    float3 finalColor = ambientLight + diffuseLight + specularLight;
    
    return float4(finalColor, 1.0);
}