
```cpp
#include <cstdint>
#include <vector>
#include <deque>
#include <mutex>
#include <cassert>

// 模拟 GPU 的 Timeline Fence（D3D12 ID3D12Fence / Vulkan VkTimelineSemaphore）
using FenceValue = uint64_t;

// 代表一整块显存/内存区域（比如 2MB）
struct UploadPage {
    void*  cpuMappedAddress = nullptr;
    size_t gpuVirtualAddress = 0;
    size_t pageSize = 0;
    size_t currentOffset = 0;
    
    // 核心：这个 Page 需要 GPU 执行到哪个 Fence 值才能被安全复用
    FenceValue retireFenceValue = 0; 

    // 线性分配（极快，无锁，因为每个线程独占自己的 Page）
    void* Suballocate(size_t size, size_t alignment, size_t& outGpuAddress) {
        size_t alignedOffset = (currentOffset + alignment - 1) & ~(alignment - 1);
        if (alignedOffset + size > pageSize) {
            return nullptr; // Page 满了
        }
        
        currentOffset = alignedOffset + size;
        outGpuAddress = gpuVirtualAddress + alignedOffset;
        return static_cast<uint8_t*>(cpuMappedAddress) + alignedOffset;
    }

    void Reset() {
        currentOffset = 0;
        retireFenceValue = 0;
    }
};

// 现代化引擎的全局 Upload 池
class GlobalUploadPool {
private:
    std::mutex m_mutex;
    size_t m_pageSize;
    
    std::vector<UploadPage*> m_freePages;
    std::deque<UploadPage*> m_inFlightPages; // 按 FenceValue 排序的已提交 Page

public:
    GlobalUploadPool(size_t pageSize = 2 * 1024 * 1024) : m_pageSize(pageSize) {}

    // 请求一个新的 Page
    UploadPage* RequestPage(FenceValue gpuCompletedFence) {
        std::lock_guard<std::mutex> lock(m_mutex);

        // 核心逻辑：尝试回收（抢时间！）
        // 只要 GPU 已经跑过了待回收队列中最老 Page 的 retireFence，立刻复用，不用等整帧！
        while (!m_inFlightPages.empty()) {
            if (m_inFlightPages.front()->retireFenceValue <= gpuCompletedFence) {
                UploadPage* recoveredPage = m_inFlightPages.front();
                m_inFlightPages.pop_front();
                recoveredPage->Reset();
                m_freePages.push_back(recoveredPage);
            } else {
                break; // 队列是有序的，第一个没完成，后面的肯定也没完成
            }
        }

        // 如果有空闲的，直接给
        if (!m_freePages.empty()) {
            UploadPage* page = m_freePages.back();
            m_freePages.pop_back();
            return page;
        }

        // 如果没有，且真的全在被 GPU 占用，才向底层 RHI 申请新的（扩容）
        return AllocateNewPageFromRHI();
    }

    // 当 CommandChunk 提交给 GPU 后，把用完的 Page 归还并打上 Fence 印记
    void RetirePage(UploadPage* page, FenceValue submitFenceValue) {
        std::lock_guard<std::mutex> lock(m_mutex);
        page->retireFenceValue = submitFenceValue;
        m_inFlightPages.push_back(page);
    }

private:
    UploadPage* AllocateNewPageFromRHI() {
        // 调用底层 API (D3D12/Vulkan) 分配真实的 Buffer...
        UploadPage* newPage = new UploadPage();
        newPage->pageSize = m_pageSize;
        // ... mapped address & gpu address setup ...
        return newPage;
    }
};

// ==============================================================================
// 在你们的流程中如何使用：

class FrameContext {
    GlobalUploadPool* m_pool;
    UploadPage* m_currentPage = nullptr;
    std::vector<UploadPage*> m_usedPagesThisFrame;

public:
    // 分配空间用于拷贝 SceneDelta 数据
    void* AllocateUploadSpace(size_t size, size_t alignment, size_t& outGpuAddr) {
        if (!m_currentPage) {
            // 假设我们能查询到 GPU 当前执行到的 Fence
            m_currentPage = m_pool->RequestPage(GetGPUCompletedFence());
            m_usedPagesThisFrame.push_back(m_currentPage);
        }

        void* ptr = m_currentPage->Suballocate(size, alignment, outGpuAddr);
        if (!ptr) { // 当前 Page 不够了，换一页
            m_currentPage = m_pool->RequestPage(GetGPUCompletedFence());
            m_usedPagesThisFrame.push_back(m_currentPage);
            ptr = m_currentPage->Suballocate(size, alignment, outGpuAddr);
        }
        return ptr;
    }

    // 封口：当指令打包提交后
    void OnCommandChunkSubmitted(FenceValue signalFence) {
        // 不需要等整帧！某个上传任务的 Command 刚丢进 Queue，立刻把它的 Page 标记为 Retire。
        // 等 GPU 的 Copy/Graphics 引擎跑过 signalFence，GlobalUploadPool 就能自动回收它。
        for (auto page : m_usedPagesThisFrame) {
            m_pool->RetirePage(page, signalFence);
        }
        m_usedPagesThisFrame.clear();
        m_currentPage = nullptr;
    }
};
```
