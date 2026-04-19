
// ==========================================
// 结构定义
// ==========================================

struct RenderProxy
{
    uint batchID;
};

// 2. 常量缓冲：视锥体平面和物体总数
// register(b0) -> 对应 C++ 的 CBV
cbuffer CullingParams : register(b0)
{
    // 视锥体的6个面：Left, Right, Bottom, Top, Near, Far
    // 这里的 float4 存储平面方程: Ax + By + Cz + D = 0
    // xyz 为法线(指向视锥体内侧), w 为距离 D
    float4 g_FrustumPlanes[6];
    float4x4 g_ViewProjection;
    // 需要剔除的实例总数
    uint g_TotalInstanceCount;
    uint g_EnableHiZCulling;
    float2 g_CullingPadding;

};

struct IndirectDrawCallArgs
{
    uint StartIndexInVisibilityBuffer;
    uint IndexCountPerInstance;
    uint InstanceCount;
    uint StartIndexLocation;
    uint BaseVertexLocation;
    uint StartInstanceLocation;
    uint PassIndex;
};

// // 1. AABB 包围盒定义 (对应 C++ 结构体)
struct AABB
{
    float3 Min; // AABB 最小点
    float3 Max; // AABB 最大点
    float2 Padding;
};


struct PerObjectData
{
    float4x4 objectToWorld;
    uint matIndex;
    uint renderProxyStartIndex;
    uint renderProxyCount;
    uint padding; // 显式填充 12 字节，确保 C++ (72字节) 与 HLSL 布局严格一致
};

// ==========================================
// 资源绑定
// ==========================================

// 输入：所有物体的 AABB 数据 (SRV)
//StructuredBuffer<AABB> g_InputAABBs : register(t0);
StructuredBuffer<AABB> g_InputAABBs : register(t0, space0);
StructuredBuffer<RenderProxy> g_RenderProxies : register(t1, space0);
StructuredBuffer<PerObjectData> g_InputPerObjectDatas : register(t2, space0);
Texture2D<Float> g_HiZTexture : register(t0, space1);

// 输出：可见物体的索引列表 (UAV)
// 我们将可见的 Instance ID 存入这个 AppendBuffer
RWStructuredBuffer<uint> g_VisibleInstanceIndices : register(u0);
RWStructuredBuffer<IndirectDrawCallArgs> g_IndirectDrawCallArgs : register(u1); // 手动传入一个计数器 buffer

// ==========================================
// 辅助函数：HIZ 判断是否被遮挡
// ==========================================
  static const float2 g_ScreenSize = float2(1920.0, 1080.0);

  float3 GetAABBCorner(AABB box, uint cornerIndex)
  {
      return float3(
          (cornerIndex & 1) ? box.Max.x : box.Min.x,
          (cornerIndex & 2) ? box.Max.y : box.Min.y,
          (cornerIndex & 4) ? box.Max.z : box.Min.z
      );
  }

  bool PassHizCulling(AABB box)
  {
      if (g_EnableHiZCulling == 0)
      {
          return true;
      }

      const float minRectSizePx = 2.0;
      const float depthBias = 1e-3;
      const uint maxConservativeMip = 6;

      float2 ndcMin = float2( 1.0,  1.0);
      float2 ndcMax = float2(-1.0, -1.0);

      // REVERSE_Z 下最近处应取最大 depth
      float nearestDepth = 0.0;

      [unroll]
      for (uint i = 0; i < 8; ++i)
      {
          float3 cornerWS = GetAABBCorner(box, i);
          float4 worldPos = float4(cornerWS, 1.0);
          float4 clip = mul(worldPos, g_ViewProjection);

          // 在近平面后面，第一版直接放弃 Hi-Z 剔除
          if (clip.w <= 0.0)
          {
              return true;
          }

          float3 ndc = clip.xyz / clip.w;

          ndcMin = min(ndcMin, ndc.xy);
          ndcMax = max(ndcMax, ndc.xy);

          nearestDepth = max(nearestDepth, ndc.z);
      }

      // 完全在屏幕外，不交给 Hi-Z，交给 frustum 那边处理
      if (ndcMax.x < -1.0 || ndcMin.x > 1.0 || ndcMax.y < -1.0 || ndcMin.y > 1.0)
      {
          return true;
      }

      float2 uvMin = saturate(ndcMin * 0.5 + 0.5);
      float2 uvMax = saturate(ndcMax * 0.5 + 0.5);

      float2 rectSizePx = (uvMax - uvMin) * g_ScreenSize;
      float maxDim = max(rectSizePx.x, rectSizePx.y);
      if (maxDim <= minRectSizePx)
      {
          return true;
      }

      float mip = max(0.0, ceil(log2(max(maxDim, 1.0) / 2.0)));
      uint mipLevel = (uint)mip;

      uint texWidth, texHeight, mipCount;
      g_HiZTexture.GetDimensions(0, texWidth, texHeight, mipCount);
      mipLevel = min(mipLevel, mipCount - 1);
      mipLevel = min(mipLevel, maxConservativeMip);

      uint mipWidth = max(1u, texWidth >> mipLevel);
      uint mipHeight = max(1u, texHeight >> mipLevel);

      float2 mipMinF = uvMin * float2(mipWidth, mipHeight);
      float2 mipMaxF = uvMax * float2(mipWidth, mipHeight);

      uint2 p0 = uint2(clamp((int)floor(mipMinF.x), 0, (int)mipWidth - 1),
                       clamp((int)floor(mipMinF.y), 0, (int)mipHeight - 1));

      uint2 p1 = uint2(clamp((int)floor(mipMaxF.x), 0, (int)mipWidth - 1),
                       clamp((int)floor(mipMinF.y), 0, (int)mipHeight - 1));

      uint2 p2 = uint2(clamp((int)floor(mipMinF.x), 0, (int)mipWidth - 1),
                       clamp((int)floor(mipMaxF.y), 0, (int)mipHeight - 1));

      uint2 p3 = uint2(clamp((int)floor(mipMaxF.x), 0, (int)mipWidth - 1),
                       clamp((int)floor(mipMaxF.y), 0, (int)mipHeight - 1));

      float h0 = g_HiZTexture.Load(int3(p0, mipLevel));
      float h1 = g_HiZTexture.Load(int3(p1, mipLevel));
      float h2 = g_HiZTexture.Load(int3(p2, mipLevel));
      float h3 = g_HiZTexture.Load(int3(p3, mipLevel));

      // REVERSE_Z + min Hi-Z
      float hizDepth = min(min(h0, h1), min(h2, h3));

      // 物体最近处都比已见深度更远，说明被遮挡
      if (nearestDepth < hizDepth - depthBias)
      {
          return false;
      }

      return true;
  }


// ==========================================
// 辅助函数：AABB vs Frustum 测试
// ==========================================
// 如果 AABB 在视锥体内(或相交)，返回 true
bool IsVisible(AABB box)
{
    // 遍历 6 个视锥体平面
    [loop]
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
    if (instanceIndex >= g_TotalInstanceCount)
        return;

    // 2. 读取当前实例的 AABB
    AABB box = g_InputAABBs[instanceIndex];
    uint proxyOffset = g_InputPerObjectDatas[instanceIndex].renderProxyStartIndex;
    
    uint proxyCount = g_InputPerObjectDatas[instanceIndex].renderProxyCount;
    if(proxyCount == 0) return; 
    uint batchID = g_RenderProxies[proxyOffset].batchID;
    if (!PassHizCulling(box)) return;
    // 3. 执行剔除测试
    if (IsVisible(box))
    {
        uint instanceCount;
        InterlockedAdd(g_IndirectDrawCallArgs[batchID].InstanceCount, 1, instanceCount);
        // 执行后：InstanceCount 原子 +1；instanceCount 是加之前的值

        uint indexStart = g_IndirectDrawCallArgs[batchID].StartInstanceLocation;
        uint currentIndex = indexStart + instanceCount;
        // 4. 如果可见，将索引追加到输出列表
        // AppendStructuredBuffer 会自动处理原子计数
        //g_VisibleInstanceIndices.Append(instanceIndex);
        g_VisibleInstanceIndices[currentIndex] = instanceIndex;
    }

}
