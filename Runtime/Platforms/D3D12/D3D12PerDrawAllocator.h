#include "Renderer/PerDrawAllocator.h"
#include "d3dUtil.h"

namespace EngineCore
{
    class D3D12PerDrawAllocator : public PerDrawAllocator
    {
    public:
        D3D12PerDrawAllocator(ID3D12Device* device, uint32_t capacityBytes);
   
        virtual ~D3D12PerDrawAllocator();

        virtual PerDrawHandle Allocate(uint32_t size) override;
        virtual uint8_t*    GetCPUAddress(PerDrawHandle h) override; // 用来 memcpy
        virtual uint64_t    GetGPUBaseAddress() const override ;  // 大 buffer 的 base GPU VA
        virtual void        ResetFrame() override;
    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> mBuffer;
        uint8_t*  mMappedCPU = nullptr;
        uint64_t  mGPUBaseAddress = 0;
        uint32_t  mCapacity = 0;
        uint32_t  mCurrentOffset = 0;
        
        uint32_t mCapacityBytes;
        static constexpr uint32_t ALIGNMENT = 256; 
    };
}