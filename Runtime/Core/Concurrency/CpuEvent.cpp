#include "PreCompiledHeader.h"
#include "CpuEvent.h"

namespace EngineCore
{
    CpuEvent &CpuEvent::RenderThreadSubmited()
    {
        static CpuEvent renderThreadSubmitedEvent(true);
        return renderThreadSubmitedEvent;
    }

    CpuEvent &CpuEvent::MainThreadSubmited()
    {
        static CpuEvent mainThreadSubmitedEvent(false);
        return mainThreadSubmitedEvent;
    }
};

