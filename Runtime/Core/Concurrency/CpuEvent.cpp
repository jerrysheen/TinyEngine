#include "PreCompiledHeader.h"
#include "CpuEvent.h"

namespace EngineCore
{
    //CpuEvent &CpuEvent::RenderThreadSubmited()
    //{
    //    static CpuEvent renderThreadSubmitedEvent(true);
    //    return renderThreadSubmitedEvent;
    //}
    //
    //CpuEvent &CpuEvent::MainThreadSubmited()
    //{
    //    static CpuEvent mainThreadSubmitedEvent(false);
    //    return mainThreadSubmitedEvent;
    //}

    CpuEvent &CpuEvent::GUIDataConsumed()
    {
        // 初始为 true：引擎启动第一帧不需要等待上一帧的GUI数据被消费
        static CpuEvent guiDataConsumedEvent(true);
        return guiDataConsumedEvent;
    }
};

