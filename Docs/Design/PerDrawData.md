
// GrassRenderPass.cpp - 草地实例化渲染示例
void GrassRenderPass::Execute(const RenderContext& context)
{
    // 假设有 10000 株草要渲染
    const uint32_t grassCount = 10000;
    Material* grassMat = GetGrassMaterial();
    ModelData* grassModel = GetGrassModel();
    
    // 每个实例的 PerDraw 数据大小（例如：WorldMatrix = 64 bytes）
    uint32_t perInstanceSize = 64;  // Matrix4x4
    uint32_t totalSize = grassCount * perInstanceSize;
    
    // 一次性分配连续的内存块
    PerDrawHandle handle = mPerDrawAllocator->Allocate(totalSize);
    uint8_t* basePtr = mPerDrawAllocator->GetCPUAddress(handle);
    
    // 填充每个实例的数据
    for (uint32_t i = 0; i < grassCount; ++i) {
        Matrix4x4 worldMatrix = CalculateGrassWorldMatrix(i);
        memcpy(basePtr + i * perInstanceSize, &worldMatrix, perInstanceSize);
    }
    
    // 创建 DrawRecord（instanceCount = 10000）
    mRenderPassInfo.drawRecordList.emplace_back(
        grassMat,
        grassModel,
        handle,
        grassCount  // 关键：实例化数量
    );
}