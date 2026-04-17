# D: RenderDoc回放除错

## 这次问题的价值

这次排查最值得记录的，不是单独修了哪一行代码，而是我们对问题模型的理解经历了几次明显修正。

最开始很多判断都带着“运行时直觉”：

- shader 看起来最可疑
- depth format / resource state 看起来最可疑
- `Present` 之后这帧应该就结束了
- legacy 既然不走 GPUScene compute，可能不会踩到同样的问题

后面顺着代码一层层对下来，发现这些直觉里有些是对的，有些只对了一半，有些则需要完全改写。

## 思路演进

### 1. 第一阶段：先怀疑 shader、depth SRV/DSV、barrier

最开始 Hi-Z blit 回放异常时，最自然的怀疑方向是：

- `BlitCSShader` 采样路径是否有问题
- depth attachment 的 SRV / DSV format 是否不匹配
- `UAV barrier` / resource state 是否缺失

这些方向都不是空穴来风：

- depth read 的状态语义确实需要更准确
- `UAV` 写后读链路补 barrier 是合理的
- `compute root signature` 后来也确实找出了一个真实 bug

但是这些点都解释不了一个核心现象：

- 程序运行大多数时候稳定
- RenderDoc replay 更容易炸
- 改 shader 采样方式以后也没有根治

这说明问题更像“生命周期 / 复用时序”，而不是单点 shader 错误。

### 2. 第二阶段：发现 `compute root signature` 生命周期 bug，但它不是唯一根因

在 [D3D12RootSignature.cpp](/D:/GitHubST/TinyEngine/Runtime/Platforms/D3D12/D3D12RootSignature.cpp) 里，compute root signature 之前把局部 `CD3DX12_DESCRIPTOR_RANGE` 的地址传给了 `InitAsDescriptorTable(...)`。

这确实是个真 bug，因为：

- `InitAsDescriptorTable(...)` 保存的是指针
- 真正 `D3D12SerializeRootSignature(...)` 发生在后面
- 局部变量早就析构了

这个问题后来修掉了，但验证结果也很明确：

- 它必须修
- 但它不是这次 replay 问题的唯一根因

这一步带来的认知修正是：

- “找到一个真 bug”不等于“已经找到主因”

### 3. 第三个关键修正：`Present` 不是 frame descriptor 复用边界

这次讨论里最大的认知修正，就是把下面两件事彻底分开了：

- CPU 执行到了 `Present`
- GPU 已经安全用完这帧绑定过的 descriptor

一开始很容易顺着直觉去想：

- 既然顺序执行到了 `Present`
- 那前面这帧的事情应该都“结束了”
- 所以 `Present` 之后 reset per-frame allocator 看起来也合理

但顺着代码看清楚之后，这个判断需要修正：

- [D3D12RenderAPI.cpp](/D:/GitHubST/TinyEngine/Runtime/Platforms/D3D12/D3D12RenderAPI.cpp:1376) 的 `RenderAPISubmit()` 先 `ExecuteCommandLists(...)`
- 然后才 `SignalFence(mFrameFence)`
- [D3D12RenderAPI.cpp](/D:/GitHubST/TinyEngine/Runtime/Platforms/D3D12/D3D12RenderAPI.cpp:1402) 的 `RenderAPIPresentFrame()` 只是发布当前 frame 的 fence 并调用 `Present`
- 真正“这个 frame slot 可复用”是在 [RenderEngine.cpp](/D:/GitHubST/TinyEngine/Runtime/Renderer/RenderEngine.cpp:81) `WaitForFrameAvaliable()` 通过之后

所以更准确的理解应该是：

- `Present` 是 CPU 提交流程的一部分
- 它不是 descriptor 生命周期的可靠结束点
- frame 资源真正的复用依据仍然是 `FrameTicket + fence`

这是这次排查里最重要的一次思路修正。

### 4. 第四个关键修正：问题不是“reset 后空泡期”，而是“后续覆盖”

后面还有一个判断被纠正：

- 一开始容易把 `ResetFrameAllocator()` 想成“把这帧的 per-frame CBV/SRV/UAV 清掉了”
- 然后脑子里自然形成“中间有一段空泡期”的模型

但实现上并不是这样：

- [D3D12DescManager.cpp](/D:/GitHubST/TinyEngine/Runtime/Platforms/D3D12/D3D12DescManager.cpp:89) 的 `ResetFrameAllocator(frameIndex)`
- [D3D12DescAllocator.cpp](/D:/GitHubST/TinyEngine/Runtime/Platforms/D3D12/D3D12DescAllocator.cpp:200) 的 `ResetDynamicSpace(frameIndex)`

它们做的事情只是：

- 把动态分配 offset 挪回当前 frame 段起点

并没有做：

- 清空 descriptor heap
- 写 null descriptor
- 让原来的 root table 立刻失效

所以问题模型必须改写成：

- old descriptor table 还在
- old descriptor 内容也还在
- 真正危险的不是 reset 那一下“空了”
- 真正危险的是后续新的分配又写回同一段槽位，把旧 descriptor 覆盖掉了

这一步理解清楚以后，很多现象就顺了：

- 为什么程序经常不挂
- 为什么 RenderDoc replay 更容易出问题
- 为什么问题不像“稳定必现”，而像时序型未定义行为

### 5. 第五个关键修正：`LagacyRenderPipeline` 也会走动态 descriptor table

这一步是我们在讨论里专门纠正过的。

一开始容易误以为：

- legacy 不走 GPUScene compute 主链
- 那它可能不会踩到 dynamic descriptor allocator 这条线

但实际代码链路不是这样：

- [LagacyRenderPipeline.cpp](/D:/GitHubST/TinyEngine/Runtime/Renderer/RenderPath/LagacyRenderPipeline.cpp:24) 调 pass 的 `Execute()`
- [OpaqueRenderPass.cpp](/D:/GitHubST/TinyEngine/Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp:50) 调 `IssueRenderCommandCommon(...)`
- [RenderPass.cpp](/D:/GitHubST/TinyEngine/Runtime/Renderer/RenderPipeLine/RenderPass.cpp:16) 明确会执行 `SetMaterialData(record.mat)`
- [D3D12RenderAPI.cpp](/D:/GitHubST/TinyEngine/Runtime/Platforms/D3D12/D3D12RenderAPI.cpp:1103) 的 `RenderAPISetMaterial` 会调用 `GetFrameCbvSrvUavAllocator(...)`
- 然后 `CopyDescriptorsSimple(...)` 到这段临时 table，再 `SetGraphicsRootDescriptorTable(...)`

所以 legacy 的真实情况是：

- 它确实会用 bindless heap
- 但它用的是 bindless heap 的动态区临时表
- 不是直接把 root table 绑到 bindless heap 起点

真正“直接绑 bindless heap 起点”的是：

- [D3D12RenderAPI.cpp](/D:/GitHubST/TinyEngine/Runtime/Platforms/D3D12/D3D12RenderAPI.cpp:1131) `RenderAPISetBindlessMat`

这一步的思路修正很重要，因为它直接推翻了“legacy 没走到这条线”的假设。

### 6. 第六个关键修正：legacy 以前没明显炸，不代表它没踩到

既然 legacy 也会走动态 descriptor table，就必须回答另一个问题：

- 为什么它以前没有像 Hi-Z 这次这样明显暴露问题

这里最后比较合理的解释是：

- 它踩到了
- 但它更容易把 bug 掩盖掉

因为 legacy 常见绑定内容更稳定：

- 材质贴图 SRV 往往长期不变
- camera 不动时，很多 draw 的纹理组合也基本不变

这样就会出现一种很迷惑的情况：

- descriptor 槽位虽然被过早复用
- 但新写回去的内容和旧内容经常接近，甚至完全一样
- GPU 即使读到了覆盖后的 descriptor，视觉结果也未必立刻异常

而 Hi-Z / depth / UAV 这类路径更敏感：

- descriptor 内容更动态
- 更依赖当前 frame / 当前 pass / 当前 mip
- 一旦覆盖错了，更容易直接变成 replay error 或 crash

这一步也修正了一个容易犯的错：

- “以前没出错”不等于“逻辑是安全的”

## 这次过程中比较典型的认知错误

为了后面少踩同样的坑，这里把这次最典型的几种误判直接列出来：

- 把 `Present` 当成 GPU 已消费完 frame 资源的边界
- 把 `ResetFrameAllocator()` 想成“把 descriptor 清空了”
- 误以为 legacy 绕开了 dynamic descriptor 路径
- 看到运行时稳定，就过早降低对生命周期 bug 的怀疑
- 找到一个真实 bug 后，过早把它当成唯一根因

这些误判都很自然，但组合起来会把排查带偏很久。

## 顺手排查过、目前没看到同等级问题的几条线

除了 dynamic descriptor 这条主线，这次还顺手看了几块多帧复用对象，目前没有看到同等级问题：

- direct command allocator： [D3D12RenderAPI.cpp](/D:/GitHubST/TinyEngine/Runtime/Platforms/D3D12/D3D12RenderAPI.cpp:964) 在 reset 前会先等 fence
- staged upload page pool： [UploadPagePool.cpp](/D:/GitHubST/TinyEngine/Runtime/Renderer/UploadPagePool.cpp:46) 按 `FrameTicket` 的 fence 值回收
- editor GUI allocator： [D3D12EditorGUIManager.cpp](/D:/GitHubST/TinyEngine/Editor/D3D12/D3D12EditorGUIManager.cpp:97) 有自己单独的 fence 闭环
- compute binding allocator： [RenderBackend.cpp](/D:/GitHubST/TinyEngine/Runtime/Renderer/RenderBackend.cpp:237) reset 与 [RenderBackend.cpp](/D:/GitHubST/TinyEngine/Runtime/Renderer/RenderBackend.cpp:257) allocate 都在主线程录制阶段，线程 ownership 一致

另外两个点暂时没有坐实，但值得以后继续留意：

- [GPUScene.cpp](/D:/GitHubST/TinyEngine/Runtime/Scene/GPUScene.cpp:320) 的 `visibilityBuffer[frameID % 3]->Reset()` 逻辑上依赖外层 `WaitForFrameAvaliable()` 间接保证安全，这条链成立，但表达比较隐式
- [D3D12DescAllocator.cpp](/D:/GitHubST/TinyEngine/Runtime/Platforms/D3D12/D3D12DescAllocator.cpp:184) 的 `CleanPerFrameData()` 目前更像死代码，后续不要再把它误用成“全局动态区 reset”

## 最后留下的结论

这次真正应该记住的，不是某个单点实现，而是这套判断顺序：

1. 不要把 `Present` 当成 descriptor 动态区复用边界。
2. `ResetFrameAllocator()` 不是清空 descriptor，而是允许后续从同一段重新分配。
3. 真正的危险是 overwrite，不是 reset 当下的空泡。
4. `LagacyRenderPipeline` 也会走动态 descriptor table，不能用“它没走这条线”解释历史现象。
5. 历史上“没出错”很多时候只是内容稳定，把 bug 掩盖住了，不等于逻辑真的安全。

## 晚点再看时的提醒

如果后面再遇到类似的 RenderDoc / replay / 多帧复用问题，优先按这个顺序想：

- 先问“资源什么时候才算真的可复用”
- 再问“reset 到底是清空还是只是回卷分配指针”
- 再问“旧绑定是不是还在引用同一段内存/descriptor”
- 最后再去怀疑 shader、格式、工具本身

这样大概率会比这次少绕很多弯路。
