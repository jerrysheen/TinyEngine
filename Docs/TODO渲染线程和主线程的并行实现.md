明白，你是想要带 fence 的两帧流水线版本，而不是继续现在的锁步。下面给你一个「最小但有效」的改造方案 + TODO + 示例代码，尽量贴合你现有结构（Renderer + RenderThread + D3D12RenderAPI）。

一、目标模型（简述）

保留现有：主线程推 DrawCommand → 渲染线程消费 → D3D12RenderAPI 录制 + 提交。

改造为：

GPU 与渲染线程：异步，渲染线程只 SignalFence，不 WaitForFence。
主线程与 GPU/渲染线程：通过 每帧一个 fenceValue + 双缓冲 FrameSlot 来控制何时可以重用 per‑frame 资源（PerDraw 大 buffer 等）。
简化：只做 2 帧 in‑flight（双缓冲），不用搞很复杂的队列。

二、概念：FrameSlot / FrameContext

先定义一个“帧槽位”（只在 CPU 侧用）：

struct FrameSlot
{
    uint64_t fenceValue = 0;  // 这一槽位上次提交到 GPU 的 fence 值
    // 如果你以后想把 RenderContext 双缓冲，也可以放在这里：
    // RenderContext renderContext;
};
Renderer 里：

static const int kFrameSlotCount = 2;

FrameSlot mFrameSlots[kFrameSlotCount];
uint64_t  mFrameIndex = 0;          // 主线程帧编号
int       mCurrentFrameSlot = 0;    // 主线程当前使用哪个槽
int       mSubmitFrameSlot  = 0;    // 渲染线程当前提交的是哪个槽（简单版主/渲染线程用同样的切换规则就行）
三、D3D12RenderAPI：Fence 改造

TODO 1：让 fence 支持“只 Signal，不 Wait”并能返回 fence 值

在 D3D12RenderAPI.h 里加一个方法：

uint64_t SignalFrameFence();                // 返回当前提交对应的 fence 值
void     WaitForFenceValue(uint64_t value); // 主线程用来等待
D3D12RenderAPI.cpp：

uint64_t D3D12RenderAPI::SignalFrameFence()
{
    // mFrameFence->mCurrentFence 自增并 Signal
    mFrameFence->mCurrentFence++;
    uint64_t value = mFrameFence->mCurrentFence;

    ThrowIfFailed(
        mCommandQueue->Signal(mFrameFence->mFence.Get(), value)
    );
    return value;
}

void D3D12RenderAPI::WaitForFenceValue(uint64_t value)
{
    if (mFrameFence->mFence->GetCompletedValue() < value)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
        ThrowIfFailed(
            mFrameFence->mFence->SetEventOnCompletion(value, eventHandle)
        );
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}
TODO 2：修改 RenderAPISubmit，只 Signal，不 Wait

现在你的 RenderAPISubmit() 最后是：

// swap the back and front buffers
SignalFence(mFrameFence);
WaitForFence(mFrameFence);
改为：

uint64_t D3D12RenderAPI::RenderAPISubmit()
{
    ...
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // 不再这里 Wait
    uint64_t fenceValue = SignalFrameFence();
    return fenceValue;
}
记得在 D3D12RenderAPI.h 里把签名改成：

virtual uint64_t RenderAPISubmit() override;
四、Renderer 渲染线程：记录每帧的 fenceValue

Renderer::RenderLoop() 里处理 kEndFrame 的地方现在是：

if(cmd.op == RenderOp::kEndFrame)
{
    currState = RenderOp::kEndFrame;
    RenderAPI::GetInstance()->RenderAPISubmit();
    ...
    RenderAPI::GetInstance()->RenderAPIPresentFrame();
    ...
}
TODO 3：接收这个 fenceValue，写进当前的 FrameSlot

if(cmd.op == RenderOp::kEndFrame)
{
    currState = RenderOp::kEndFrame;

    // 1. 提交命令并拿到 fence 值
    uint64_t fenceValue = RenderAPI::GetInstance()->RenderAPISubmit();

    // 2. 记录到当前 submit 槽位
    mFrameSlots[mSubmitFrameSlot].fenceValue = fenceValue;

    // 3. Present + ResetFrameAllocator
    RenderAPI::GetInstance()->RenderAPIPresentFrame();

    // 4. 切到下一槽位（简单轮询 0/1）
    mSubmitFrameSlot = (mSubmitFrameSlot + 1) % kFrameSlotCount;
}
注意：这里 mFrameSlots 和 mSubmitFrameSlot 要在 Renderer 里定义成 mutable 的共享状态（可能需要加 std::atomic<uint64_t> 或简单的互斥保护，但因为只有渲染线程写、主线程读/等，我们可以用“一帧之后再读”的弱同步，先不复杂化）。

五、Renderer 主线程：帧开始前等对应 FrameSlot 的 fence

现在主线程的 Renderer::Render 是这样：

void Renderer::Render(const RenderContext& context)
{
    BeginFrame();
    FlushPerFrameData();
    ...
    EndFrame();
    ...
}
TODO 4：在 BeginFrame 前，根据 FrameIndex 选择槽位，并等待该槽上一次使用结束

void Renderer::Render(const RenderContext& context)
{
    // 1. 选出当前要用的 FrameSlot
    uint64_t frameIndex = ++mFrameIndex;
    mCurrentFrameSlot = frameIndex % kFrameSlotCount;
    FrameSlot& slot = mFrameSlots[mCurrentFrameSlot];

    // 2. 如果这个槽位之前有 fenceValue，就等待 GPU 完成那次提交
    if (slot.fenceValue != 0)
    {
        RenderAPI::GetInstance()->WaitForFenceValue(slot.fenceValue);
        slot.fenceValue = 0; // 已经消费完，可以视为“空槽”
    }

    // 3. 正常走这一帧的 Begin/Render/End
    BeginFrame();
    FlushPerFrameData();

    for (auto& pass : context.camera->mRenderPassAsset.renderPasses)
    {
        pass->Configure(context);
        pass->Filter(context);
        pass->Execute(context);
        Submit(pass->GetRenderPassInfo());
    }

    EndFrame();
}
这样就实现了：

同一个 FrameSlot（你可以把它和 PerDraw 大 buffer / FrameAllocator 绑定理解）只有在 上一轮使用它的 GPU 完全跑完之后 才会被主线程再次使用；
渲染线程不阻塞 GPU，只在 RenderAPISubmit() 里 Signal，一切等待都在主线程“重用槽位”时进行；
你仍然可以在 RenderAPIPresentFrame 里 Reset per‑frame allocator，因为此时上一帧 GPU 已经结束，且下一帧不会重用旧槽。
六、PerDraw / PerPass buffer 要不要两份？

在这个“最小有效版”里，可以继续只用一份 D3D12PerDrawAllocator，因为：

你在 RenderAPIPresentFrame 里 Reset 的前提，是上一帧 GPU 已经完成（多 thanks 于 WaitForFenceValue）。
主线程每帧开始前等待的是它要重用的那个 FrameSlot 对应的 fence。
只要你保持「每帧只使用一个 PerDrawAllocator，并在 Present 后 Reset」，就不会出现 “A 帧还在用这块 buffer，B 帧已经重新写” 的情况。
也就是说：双缓冲只用在 FrameSlot（CPU 管控）层，不用立刻把 PerDrawAllocator 也做双份，这是你要的「保留最简单，但有效果」的版本。

七、最终 TODO 汇总

D3D12RenderAPI

 新增 SignalFrameFence / WaitForFenceValue，按上面的代码实现。
 修改 RenderAPISubmit()：返回 uint64_t fenceValue，不再在内部 Wait。
 RenderAPIPresentFrame() 保持现状（Present + Reset desc frame allocator）。
Renderer（渲染线程部分）

 在 Renderer 中新增 FrameSlot mFrameSlots[2]、uint64_t mFrameIndex、int mCurrentFrameSlot、int mSubmitFrameSlot。
 在 RenderLoop 处理 kEndFrame 时代码改为：fenceValue = RenderAPISubmit() → 写入 mFrameSlots[mSubmitFrameSlot].fenceValue → RenderAPIPresentFrame() → mSubmitFrameSlot = (mSubmitFrameSlot + 1) % 2;。
Renderer（主线程 Render）

 在 Render 开头，增加 FrameSlot 选择和 WaitForFenceValue 逻辑（如上示例）。
 确保所有 PerDrawHandle 分配和写入都发生在该帧的 BeginFrame 与 EndFrame 之间。
文档 / 约定

 在 RenderContext / RenderPassInfo / DrawRecord 注释中写明：一帧内构建完后，只读，不能在 EndFrame 后再改。
 在 PerDrawAllocator 相关注释里说明：Reset 的时机依赖于 fence，同一帧所有 PerDraw 分配必须在 Begin/EndFrame 间完成。
这样改完之后：

GPU 与渲染线程可以真正异步跑；
主线程仅在“复用某个帧槽”前等待对应的 fence，得到与 Unity 类似的「提交后数据就冻结、两帧 in‑flight」效果，同时整体结构仍然非常接近你现在的代码。