#pragma once
#include <vector>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <thread>
#include <chrono>
#include <unordered_map>

#if !defined(EDITOR_PROFILER)
#   if defined(EDITOR) || defined(_DEBUG)
#       define ENABLE_PROFILER 1
#   else
#       define ENABLE_PROFILER 0
#   endif
#endif

#if ENABLE_PROFILER
namespace EngineCore
{
    // 函数打点
    struct ProfilerEvent
    {
        const char* name        = nullptr;
        uint32_t    threadID    = 0;
        float       startMs     = 0.0f;
        float       endMs       = 0.0f;
        uint8_t     depth       = 0;
    };

    // 统计数据打点
    struct ProfilerCounter
    {
        const char* name    = nullptr;
        double      value   = 0.0;
    };
    
    struct ProfilerFrame
    {
        float frameStartMs = 0;
        const char* frameTag = nullptr;

        std::vector<ProfilerEvent> events;

        std::unordered_map<const char*, uint32_t> counterIndex;

        void Clear()
        {
            events.clear();
            counterIndex.clear();
            frameTag = nullptr;
            frameStartMs = 0.0f;
        }
    };


    class Profiler
    {
    public:
        static Profiler& Get()
        {
            static Profiler s_Instance;
            return s_Instance;
        }

        void MarkFrame(const char* name)
        {
            const uint32_t nextIndex = (m_currentFrameIndex + 1u) % kMaxFrames;
            m_currentFrameIndex = nextIndex;

            ProfilerFrame& frame = m_frams[nextIndex];
            std::lock_guard<std::mutex> lock(m_frameMutex);
            frame.Clear();
            frame.frameStartMs = GetTimeMs();
            frame.frameTag = name;    
        }

        void RecordEvent(const char* name, uint32_t threadId, float startMs, float endMs, uint8_t depth)
        {
            ProfilerFrame& frame = m_frams[m_currentFrameIndex];
            std::lock_guard<std::mutex> lock(m_frameMutex);
            frame.events.push_back(ProfilerEvent
                {
                    name, threadId, startMs, endMs, depth
                }

            );
        }

        void AddCounter(const char* name, double value)
        {
            ProfilerFrame& frame = m_frams[m_currentFrameIndex];
            std::lock_guard<std::mutex> lock(m_frameMutex);

            if(frame.counterIndex.count(name) > 0)
            {
                frame.counterIndex[name] = frame.counterIndex[name]++;
            }
            else
            {
                frame.counterIndex[name] = 1;
            }
        }

        const ProfilerFrame& GetFrame(uint32_t index) const
        {
            return m_frams[index % kMaxFrames];  // 使用传入的 index
        }

        uint32_t GetCurrentFrameIndex() const {return m_currentFrameIndex;}

        float GetTimeMs() const
        {
            using namespace std::chrono;
            auto now = std::chrono::high_resolution_clock::now();
            auto us  = duration_cast<microseconds>(now - m_startTime).count();
            return static_cast<float>(us / 1000.0);
        }

        void BeginRange(const char* name)
        {
            RangeData& data = GetThreadRangeData();
            data.name = name;
            data.startMs = GetTimeMs();
            data.threadID = GetThreadIdForProfiler();
        }
        
        void EndRange(const char* name)
        {
            RangeData& data = GetThreadRangeData();
            if ((data.name == nullptr || data.name != name)) 
            {
                __debugbreak();
            }
            
            float endMs = GetTimeMs();
            RecordEvent(data.name, data.threadID, data.startMs, endMs, /*depth=*/0);
            data.name = nullptr;
        }

       /* uint8_t PushDepth()
        {
            return m_threadDepth++;
        }

        void PopDepth()
        {
            if (m_threadDepth > 0) m_threadDepth--;
        }*/
       
        // 在 Profiler 类中添加
        float GetLastFrameTimeMs() const 
        {
            if (m_currentFrameIndex == 0) 
                return 0.0f;
            
            const uint32_t prevIndex = (m_currentFrameIndex > 0) 
                ? (m_currentFrameIndex - 1) 
                : (kMaxFrames - 1);
            
            const ProfilerFrame& currentFrame = m_frams[m_currentFrameIndex];
            const ProfilerFrame& prevFrame = m_frams[prevIndex];
            
            return currentFrame.frameStartMs - prevFrame.frameStartMs;
        }

        float GetFPS() const
        {
            float frameTimeMs = GetLastFrameTimeMs();
            if (frameTimeMs < 0.001f) return 0.0f;
            return 1000.0f / frameTimeMs;
        }
    private:

        struct RangeData
        {
            const char* name    = nullptr;
            uint32_t    threadID = 0;
            float       startMs  = 0.0f;
        };

        static RangeData& GetThreadRangeData()
        {
            static thread_local RangeData s_data;
            return s_data;
        }

        static uint32_t GetThreadIdForProfiler()
        {
            auto id = std::this_thread::get_id();
            std::hash<std::thread::id> hasher;
            return static_cast<uint32_t>(hasher(id));
        }

        Profiler()
        {
            m_startTime = std::chrono::high_resolution_clock::now();
        }

        static constexpr uint32_t kMaxFrames = 300;

        std::chrono::high_resolution_clock::time_point m_startTime;
        ProfilerFrame m_frams[kMaxFrames];

        std::atomic<uint32_t> m_currentFrameIndex{0};

        mutable std::mutex m_frameMutex;

        //static thread_local uint8_t m_threadDepth;
    };

    
    struct ProfilerZoneScope
    {
        explicit ProfilerZoneScope(const char* name)
            : m_name(name)
        {
            Profiler& p = Profiler::Get();
            m_startMs = p.GetTimeMs();
            m_threadId = GetThreadId();
            //m_depth = p.PushDepth();
        }

        ~ProfilerZoneScope()
        {
            Profiler& p = Profiler::Get();
            float end = p.GetTimeMs();
            p.RecordEvent(m_name, m_threadId, m_startMs, end, m_depth);
            //p.PopDepth();
        }
    private:
        static uint32_t GetThreadId()
        {
            auto id = std::this_thread::get_id();
            std::hash<std::thread::id> hasher;
            return static_cast<uint32_t>(hasher(id));
        }

        const char* m_name;
        float       m_startMs = 0.0f;
        uint32_t    m_threadId = 0;
        uint8_t     m_depth = 0;
    };
}

#define PROFILER_FRAME_MARK(name)   Profiler::Get().MarkFrame(name)
#define PROFILER_ZONE(name)         ProfilerZoneScope __scope##__LINE__{name}
#define PROFILER_COUNTER_ADD(name,v)  Profiler::Get().AddCounter(name, (v))
#define PROFILER_EVENT_BEGIN(name)  EngineCore::Profiler::Get().BeginRange(name)
#define PROFILER_EVENT_END(name)    EngineCore::Profiler::Get().EndRange(name)
// 关闭时全部变成空宏
#else
#define PROFILER_FRAME_MARK(name)     ((void)0)
#define PROFILER_ZONE(name)           ((void)0)
#define PROFILER_COUNTER_ADD(name,v)  ((void)0)
#define PROFILER_EVENT_BEGIN(name)  ((void)0)
#define PROFILER_EVENT_END(name)    ((void)0)

#endif // ENABLE_PROFILER