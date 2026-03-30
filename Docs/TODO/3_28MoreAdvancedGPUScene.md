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



//。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。
第二步：  优化Drawcall， 只issue实际的drawcall（稀疏drawcall dence化）， 
记录每个pass的 count 和 offset， 一次性issue。
我们之前的优化， 相当于把所有的Drawcall都优化成了一个Drawcall就能绘制一批物体， 
但是这还是意味着我们要issue这么多次， 并且中间还有一些visibility false的没办法剔除。
为了解决这个问题， 我们需要一个dence的 indirctdrawcall buffer， 并且还需要尽量减少前后端通信， 后者我们会用一个countbuffer来记录， 这个就是ExecuteIndirect里面的参数而已， 就是我们在GPU上算出来每个不同的PSOBatch/ pass，会有几个Drawcall， 然后位置在哪里。
这些Drawcall我们已经在GPU端生成好了， 这个地方只要告诉GPU一次性读这部分内容，并且设置好这个Batch的状态， GPU就能一次性执行所有的Drawcall了。
这一步的主要做法就是在我们之前的处理模式之后， 生成CompactIndirectCommandBuffer+ countBUffer， 然后执行这个就行。
我们现在可以先把Group撇了， 后续再去想这个问题， 先跑通流程。

  1. 先按旧逻辑做 culling
      - 写“原始/稀疏”的 IndirectDrawArgs
      - 写 VisibilityBuffer
      - 每个 DrawTemplate 仍然有自己固定的 visibility 段
      - InstanceCount 也还是先累计到这张原始表里
  2. 再做一次 command compaction
      - 扫描这张原始 IndirectDrawArgs
      - 找出 InstanceCount > 0 的项
      - 按 groupID / PSOGroup 把它们写入 CompactIndirectCommandBuffer
      - 同时 groupCountBuffer[groupID]++
  3. 最后按 group 执行
      - CPU 只管按 group 绑定 PSO / RootSignature
      - 每个 group 一次 ExecuteIndirect
      - count 由 groupCountBuffer 控制

  也就是：

  Pass 1: Cull
  SparseIndirectArgsBuffer + VisibilityBuffer

  Pass 2: Compact
  SparseIndirectArgsBuffer -> CompactIndirectCommandBuffer
                      -> GroupCountBuffer

  Pass 3: Render
  for each group:
      SetPSO(...)
      SetRootSignature(...)
      ExecuteIndirect(...)

  ———

  这样做的好处是：

  - 不需要一开始就把 visibility 也改成全局紧密
  - 不需要维护 PerObject -> CompactIndex
  - 你现有 culling 逻辑几乎可以保留
  - 新增复杂度主要集中在第二步 Compact Pass

  你现在可以把它理解成：

  - 第一步解决“哪些 draw 有内容”
  - 第二步解决“把这些 draw 整理成适合 ExecuteIndirect 批量提交的格式”
  - 第三步才是真正提交

  这个思路是对的。
  下一步你真正要设计清楚的，就是第二步 compact pass 的输入输出结构。
