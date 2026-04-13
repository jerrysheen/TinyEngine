struct IndirectDrawDest
{
    uint StartIndexInVisibilityBuffer;
    uint IndexCountPerInstance;
    uint InstanceCount;
    uint StartIndexLocation;
    uint BaseVertexLocation;
    uint StartInstanceLocation;
};


struct IndirectDrawSrouce
{
    uint StartIndexInVisibilityBuffer;
    uint IndexCountPerInstance;
    uint InstanceCount;
    uint StartIndexLocation;
    uint BaseVertexLocation;
    uint StartInstanceLocation;
    //  记录当前这个IndirectDrawSource在哪个pass里面
    uint PassIndex;
};


cbuffer BatchCountBuffer : register(b0)
{
    uint bacthCount; 
};

// ==========================================
// 资源绑定
// ==========================================

// 输入：
RWStructuredBuffer<IndirectDrawSrouce> IndirectDrawSourceBuffer : register(u0); // 手动传入一个计数器 buffer


// 输出：IndirectDrawCall
RWStructuredBuffer<IndirectDrawDest> IndirectDrawDestBuffer : register(u1); // 手动传入一个计数器 buffer
RWStructuredBuffer<uint> IndirectDrawCount : register(u2); // 手动传入一个计数器 buffer

// ===================================  ======
// 主函数
// ==========================================
// 线程组大小：通常设为 64 或 128
[numthreads(64, 1, 1)]
void CSMain(uint3 dtid : SV_DispatchThreadID)
{
    if(dtid.x > bacthCount) return;
    uint instanceIndex = dtid.x;
    uint passIndex = IndirectDrawSourceBuffer[instanceIndex].PassIndex;
    if(IndirectDrawSourceBuffer[instanceIndex].InstanceCount == 0) return;
    uint instanceCount;
    InterlockedAdd(IndirectDrawCount[passIndex], 1, instanceCount);
    instanceCount = instanceCount + passIndex * 1000;
    IndirectDrawDestBuffer[instanceCount].StartIndexInVisibilityBuffer = IndirectDrawSourceBuffer[instanceIndex].StartIndexInVisibilityBuffer;
    IndirectDrawDestBuffer[instanceCount].IndexCountPerInstance = IndirectDrawSourceBuffer[instanceIndex].IndexCountPerInstance;
    IndirectDrawDestBuffer[instanceCount].InstanceCount = IndirectDrawSourceBuffer[instanceIndex].InstanceCount;
    IndirectDrawDestBuffer[instanceCount].StartIndexLocation = IndirectDrawSourceBuffer[instanceIndex].StartIndexLocation;
    IndirectDrawDestBuffer[instanceCount].BaseVertexLocation = IndirectDrawSourceBuffer[instanceIndex].BaseVertexLocation;
    IndirectDrawDestBuffer[instanceCount].StartInstanceLocation = IndirectDrawSourceBuffer[instanceIndex].StartInstanceLocation;

}
