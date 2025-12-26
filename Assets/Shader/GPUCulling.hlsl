
// ==========================================
// 结构定义
// ==========================================

// 1. AABB 包围盒定义 (对应 C++ 结构体)
struct AABB
{
    float3 Min; // AABB 最小点
    float3 Max; // AABB 最大点
    float2 Padding;
};

// 2. 常量缓冲：视锥体平面和物体总数
// register(b0) -> 对应 C++ 的 CBV
cbuffer CullingParams : register(b0)
{
    // 视锥体的6个面：Left, Right, Bottom, Top, Near, Far
    // 这里的 float4 存储平面方程: Ax + By + Cz + D = 0
    // xyz 为法线(指向视锥体内侧), w 为距离 D
    float4 g_FrustumPlanes[6]; 
    
    // 需要剔除的实例总数
    uint g_TotalInstanceCount; 
};

// ==========================================
// 资源绑定
// ==========================================

// 输入：所有物体的 AABB 数据 (SRV)
StructuredBuffer<AABB> g_InputAABBs : register(t0);

// 输出：可见物体的索引列表 (UAV)
// 我们将可见的 Instance ID 存入这个 AppendBuffer
RWStructuredBuffer<uint> g_VisibleInstanceIndices : register(u0);
RWByteAddressBuffer g_CounterBuffer : register(u1); // 手动传入一个计数器 buffer

// ==========================================
// 辅助函数：AABB vs Frustum 测试
// ==========================================
// 如果 AABB 在视锥体内(或相交)，返回 true
bool IsVisible(AABB box)
{
    // 遍历 6 个视锥体平面
    for (int i = 0; i < 6; ++i)
    {
        float4 plane = g_FrustumPlanes[i];
        float3 normal = plane.xyz;
        float distance = plane.w;

        // 找到 AABB 上 "最可能在平面外侧" 的点 (Positive Vertex / p-vertex)
        // 这个点是沿着法线方向最远的点
        float3 p;
        p.x = (normal.x > 0) ? box.Max.x : box.Min.x;
        p.y = (normal.y > 0) ? box.Max.y : box.Min.y;
        p.z = (normal.z > 0) ? box.Max.z : box.Min.z;

        // 计算点到平面的有向距离
        // dot(N, P) + D
        // 如果距离 < 0，说明这个点在平面的背面（外侧）
        // 如果 AABB 最靠里的点都在背面，说明整个 AABB 都在外面 -> 剔除
        if (dot(normal, p) + distance < 0.0f)
        {
            return false;
        }
    }
    return true;
}

// ==========================================
// 主函数
// ==========================================
// 线程组大小：通常设为 64 或 128
[numthreads(64, 1, 1)]
void CSMain(uint3 dtid : SV_DispatchThreadID)
{
    uint instanceIndex = dtid.x;

    // 1. 越界检查
    //if (instanceIndex >= g_TotalInstanceCount)
    //    return;

    // 2. 读取当前实例的 AABB
    AABB box = g_InputAABBs[instanceIndex];

    // 3. 执行剔除测试
    if (IsVisible(box))
    {
        // 4. 如果可见，将索引追加到输出列表
        // AppendStructuredBuffer 会自动处理原子计数
        //g_VisibleInstanceIndices.Append(instanceIndex);
    }
    uint writeIndex;
    g_CounterBuffer.InterlockedAdd(0, 1, writeIndex); 

    g_VisibleInstanceIndices[writeIndex] = 1;
}
