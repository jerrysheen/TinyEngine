#include "PreCompiledHeader.h"
#include "JobSystem.h"

namespace EngineCore
{
    JobSystem* JobSystem::s_Instance = nullptr;

    JobSystem::JobSystem()
    {
        isRunning = true;

        unsigned int numThreads = std::thread::hardware_concurrency();
        ASSERT(numThreads >= 2);
        numThreads = numThreads - 2;

        // job会优先起在空闲核内， 如果要指定核，要用别的接口
        for(unsigned int i = 0; i < numThreads; i++)
        {
            m_Workers.emplace_back(&JobSystem::WorkerThreadLoop, this);
        }
    }

    JobSystem::~JobSystem()
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            isRunning = false;
        }

        wakeWorker.notify_all();

        for(std::thread& worker : m_Workers)
        {
            if(worker.joinable())
            {
                worker.join();
            }
        }

        while(!counterQueue.empty())
        {
            delete counterQueue.front();
            counterQueue.pop_front();
        }
    }

    void JobSystem::Create()
    {
        if(s_Instance != nullptr)
        {
            return;
        }
        s_Instance = new JobSystem();
    }

    void JobSystem::Shutdown()
    {
    }

    JobSystem *JobSystem::GetInstance()
    {
        if(s_Instance == nullptr)
        {
            Create();
        }
        return s_Instance;
    }

    void JobSystem::WaitForJob(JobHandle handle)
    {
        while(handle.counter->value > 0)
        {
            bool executed = TryExecuteOneJob();
        }
    }

    void JobSystem::InternalKickJob(void (*function)(void *, void *), void *JobData, void *rawCounter)
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            jobQueue.push_front({function, JobData, rawCounter});
        }
        wakeWorker.notify_one();
    }

    void JobSystem::WorkerThreadLoop()
    {
        while(true)
        {
            InternalJob job;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                // wait 为true，就是不sleep，继续走的条件是，
                // isRunning为false了，需要往下走逻辑， 或者是jobQueue有东西了
                // 需要往下pop()； 不然这个线程关不掉？
                wakeWorker.wait(lock, [&]{ return !isRunning || !jobQueue.empty();});
                // 如果关闭了并且jobqueue已经为0了，就直接跳出这个线程，和 {} 无关。
                if(!isRunning && jobQueue.empty()) 
                {
                    return;
                }
                if(jobQueue.empty()) continue;
                
                job = jobQueue.front();
                jobQueue.pop_front();
            }
            job.function(job.JobData, job.rawCounter);
        }
    }

    bool JobSystem::TryExecuteOneJob()
    {
        InternalJob job;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if(jobQueue.empty()) return false;
            job = jobQueue.front();
            jobQueue.pop_front();
        }
        job.function(job.JobData, job.rawCounter);
    }

    JobCounter *JobSystem::GetAvaliableCounter()
    {
        if(counterQueue.empty())
        {
            return new JobCounter();
        }

        JobCounter* counter = counterQueue.front();
        counterQueue.pop_front();
        return counter;
    }

};