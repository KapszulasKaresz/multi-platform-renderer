[[vk::binding(0, 0)]]
RWStructuredBuffer<float3> dataBuffer : register(u0, space0);

[numthreads(4, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x < 4)
    {
        dataBuffer[dispatchThreadID.x].x += 2.0f;
    }
}