#pragma once
#include "PreCompiledHeader.h"
#include <unordered_map>
#include <string>
#include "Math/Math.h"
#include "Core/PublicStruct.h"

namespace EngineCore
{
    class MaterialPropertyBlock
    {
    public:
        MaterialPropertyBlock() = default;

        void Initialize(unordered_map<string, ShaderConstantInfo>& reflectionInfo,
            uint32_t bufferSize)
            {
                mReflectionInfo = &reflectionInfo;
                mBuffer.resize(bufferSize, 0);
            }
        
        template<typename T>
        inline void SetValue(const string& name, const T& value)
        {
            ASSERT(mReflectionInfo != nullptr && mReflectionInfo->count(name) > 0);

            // 方案1：使用 at()
            //const ShaderConstantInfo& info = mReflectionInfo->at(name);

            // 方案2：使用 find()（更推荐，因为已经 count 过了）
            auto it = mReflectionInfo->find(name);
            const ShaderConstantInfo& info = it->second;

            ASSERT(info.offset + sizeof(T) <= mBuffer.size());
            memcpy(mBuffer.data() + info.offset, &value, sizeof(T));
        }
        
        // 获取数据（用于拷贝到 GPU）
        const uint8_t* GetData() const { return mBuffer.data(); }
        uint32_t GetSize() const { return mBuffer.size(); }
        
        // 清空数据
        void Clear() { std::fill(mBuffer.begin(), mBuffer.end(), 0); }

    private:
        std::vector<uint8_t> mBuffer;
        unordered_map<string, ShaderConstantInfo>* mReflectionInfo = nullptr;
    };
}