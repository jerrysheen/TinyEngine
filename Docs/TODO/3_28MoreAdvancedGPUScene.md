做完bindlesstexture + vertexPulling之后， 我们进一步解决问题：
当前的架构下， 比如OpaquePass，只需要设置一次状态即可，这一次状态就是绑定TextureBuffer+ VertexBuffer + perObejctbuffer啥的。
但是我们发射drawcall还是需要多次。这个是一个历史遗留问题：
我们现在的策略很简单， 生成所有的Batch，然后GPUCulling把所有需要渲染的Batch录入Visibilitybuffer， 最后issue所有的IndirectDrawcall，他们通过自己在visibilitybuffer的起始位置去进行对应的绘制。
在这个模式底下，简单说下后端怎么找到对应的内容：
首先IndirectDrawArgsBuffer作为输入和输出，被GPUCulling使用，
GPUCulling扫描场景的PerObjectData，根据每个AABB和Camera判断可见性， 然后通过RenderProxyBuffer找到对应的BatchID，也就是DrawIndirectArgsBuffer的index。
从中读取预设好的在Visibilitybuffer中的起始位置+当前的count， 然后用internalLock ++，写入。
后续我们将这个indirectDrawArgs + 每个Args在可见性列表中的位置做成一个ConstantBuffer，传上去。其中：这个是DrawIndirectArgs， 其中 IndexCountPerInstance、InstanceCount、StartIndexLocation是有用的， 这个就是决定我们当前的mesh的起始index + 长度，
有了这个数据我们直接去对应地方索引数据就行。
  struct DrawIndirectArgs
  {
      uint32_t IndexCountPerInstanc = 0;
      uint32_t InstanceCount = 0;
      uint32_t StartIndexLocation = 0;
      uint32_t BaseVertexLocation = 0;
      uint32_t StartInstanceLocation = 0;
  };

    IndexCountPerInstance 6588
    InstanceCount       1
    StartIndexLocation  771393
    BaseVertexLocation  0
    StartInstanceLocation 124
注意这个地方， 如果我们不用IA传数据的方式， 只用VertexPulling，那么这个地方只有前三个参数有用， 大致代码为：
// 顶点着色器
VertexOutput VSMain(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    VertexOutput output;


    // // 变换到世界空间
    uint index = g_VisibleInstanceIndices[instanceID + g_InstanceBaseOffset];
    PerObjectData data = g_InputPerObjectDatas[index];

    Vertex input = MeshBuffer[vertexID + data.baseVertexLocation];
vertexID就是传进来的， 读了 StartIndexLocation以后的值，比如index是 0 1 2 3 2 1，那么读进来的就是这个值， data.baseVertexLocation 就存在PerObjectData里面

//。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。
当前问题点： 这么做可以发现， 当前的点在于频繁的状态切换， 我们我们现在每一个Draw，不同的只有DrawIndirectArgs的参数，以及每个Drawcall在visiblitybuffer里面的位移，以及这个visibilitybuffer并不是紧密的每一个都要绘制的。
解决这个问题我们要利用API接口的能力      
mCommandList->ExecuteIndirect(
          mCommandSignature.Get(),
          drawIndirect.count,
          static_cast<ID3D12Resource*>(argsBuffer->GetNativeHandle()),
          drawIndirect.startIndex * stride,
          nullptr,
          0
  它的语义是：

  - argsBuffer 里放的是很多条 DrawIndirectArgs
  - drawIndirect.startIndex 指定从第几条开始读
  - drawIndirect.count 指定这次最多执行几条
  - countBuffer = nullptr，说明“执行几条命令”完全由 CPU 的 drawIndirect.count 决定
也就是说首先这个指令是支持群发DrawIndirectArgs的，
现在我们唯一需要解决的是每个Drawcall在Visibility里面的起始位置这个RootConstant，这个也可以优化的。 就是在创建这个IndirectArgsCommand的时候，配套塞入一个RootConstant，然后后续就可以按照这个规则读取了。

另外， 如何紧密排列呢，就是单独做一个输出的IndirectDrawBuffer， 输入是稀疏的，输出是紧密排列的， 这样也解决了前面那个，我们传入的RenderProxy写死的问题





  所以，“生成可见列表”和“GPU 发起 draw”之间的关系是：

  第一档：
  GPU 只生成 VisibleObjectList
  CPU 仍然组织 draw
  这个提升有限。

  第二档：
  GPU 生成 IndirectDrawArgsBuffer
  CPU 仍然逐 batch 调 ExecuteIndirect(count=1)
  这就是你们现在，提升也有限。

  第三档：
  GPU 生成 CommandBuffer + CountBuffer
  CPU 每个大桶只调一次 ExecuteIndirect(..., countBuffer ...)
  这才是明显进入正经 GPU-driven。

  第四档：
  再往后是 mesh shader / cluster culling / work graph 一类，更激进。

  ———

  结合你们当前代码，第一任务我建议这样定

  不是先做“visible batch list”本身，
  而是做：

  按状态桶输出 GPU command list。

  具体目标可以定成：

  1. CPU 只维护稳定的状态桶
     按 PSO + Pass + MaterialPolicy 分桶。
  2. GPU culling 后，向每个桶的 CommandBuffer 追加命令
     命令里至少包含：

  - drawID 或 instanceBaseOffset
  - DrawIndexedArgs 或 DrawArgs

  3. 渲染阶段每个桶只调用一次 ExecuteIndirect
     并使用 CountBuffer 控制实际命令数。

  这样有几个直接收益：

  - 不需要 GPU readback
  - 不需要 CPU 遍历所有 batch
  - 空批次自然消失
  - 真正减少 draw submission 次数
  - 后续接 HiZ 非常自然，因为 HiZ 只是影响“命令生成阶段”的过滤条件

  ———

  HiZ 怎么和这个架构接
  HiZ 最好接在“生成 command list 之前”。

  建议链路是：

  1. Frustum culling
  2. HiZ occlusion culling
  3. surviving object -> append 到对应状态桶 command buffer
  4. ExecuteIndirect per bucket

  这样 HiZ 只是多了一层“决定是否 append command”的逻辑，不需要改渲染阶段。

  ———

  一句话总结
  你现在的正确方向不是“GPU 回读一份 visible list 给 CPU”，而是“GPU 直接写出每个状态桶真正要执行的 indirect command
  buffer，CPU 只发少量 ExecuteIndirect 总控命令”。

  如果你愿意，我下一步可以直接把这件事落成一份更贴近你们工程的设计稿，明确：

  - 现在的 BatchManager 应该保留什么
  - RenderProxy 应该删成什么
  - GPUCulling.hlsl 最终要输出哪两类 buffer
  - GPUSceneRenderPass 最终如何从“遍历 batch”改成“每桶一次 ExecuteIndirect”