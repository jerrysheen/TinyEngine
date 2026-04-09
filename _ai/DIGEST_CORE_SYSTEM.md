# Architecture Digest: CORE_SYSTEM
> Auto-generated. Focus: Runtime/Core, Runtime/Core/Allocator, Runtime/Core/Concurrency, Runtime/Math, Runtime/Serialization, Runtime/Utils, PublicStruct, PublicEnum, Profiler, Job, JobSystem, InstanceID, Allocator, Math, Vector3, Matrix4x4, Plane, Cross, Dot, Perspective

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作，并建立解耦的帧更新流（GameObject/Component、Scene、CPUScene/GPUScene、FrameContext多帧同步）。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。
- 针对更新链路重点追踪：Game::Update/Render/EndFrame -> SceneManager/Scene -> CPUScene -> GPUScene -> FrameContext。
- 重点识别NodeDirtyFlags、NodeDirtyPayload、PerFrameDirtyList、CopyOp等脏数据传播与跨帧同步结构。

## Understanding Notes
- 核心数据结构、内存分配、标识系统与序列化元数据是其他模块的基础。
- 优先记录公开结构体/枚举/Allocator/Profiler接口，包含Math数学库、JobSystem多线程系统。

## Key Files Index
- `[67]` **Runtime/Core/Concurrency/JobSystem.h** *(Content Included)*
- `[65]` **Runtime/Core/Concurrency/JobSystem.cpp** *(Content Included)*
- `[53]` **Runtime/Math/Matrix4x4.cpp** *(Content Included)*
- `[53]` **Runtime/Math/Matrix4x4.h** *(Content Included)*
- `[50]` **Runtime/Math/Plane.h** *(Content Included)*
- `[48]` **Runtime/Math/Vector3.h** *(Content Included)*
- `[46]` **Runtime/Math/Vector3.cpp** *(Content Included)*
- `[44]` **Runtime/Math/Plane.cpp** *(Content Included)*
- `[44]` **Runtime/Core/Allocator/LinearAllocator.h** *(Content Included)*
- `[39]` **Runtime/Core/PublicStruct.h** *(Content Included)*
- `[37]` **Runtime/Core/Profiler.h** *(Content Included)*
- `[36]` **Runtime/Core/InstanceID.h** *(Content Included)*
- `[36]` **Runtime/Math/Math.h** *(Content Included)*
- `[34]` **Runtime/Core/PublicEnum.h** *(Content Included)*
- `[32]` **Runtime/Math/Frustum.cpp** *(Content Included)*
- `[31]` **Runtime/Core/PublicEnum.cpp** *(Content Included)*
- `[31]` **Runtime/Core/PublicStruct.cpp** *(Content Included)*
- `[29]` **Runtime/Core/Game.cpp** *(Content Included)*
- `[29]` **Runtime/Math/AABB.h** *(Content Included)*
- `[28]` **Runtime/Math/Frustum.h** *(Content Included)*
- `[27]` **Runtime/Graphics/GPUBufferAllocator.h**
- `[27]` **Runtime/Math/AABB.cpp**
- `[27]` **Runtime/Math/Quaternion.h**
- `[26]` **Runtime/Math/Quaternion.cpp**
- `[26]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h**
- `[25]` **Runtime/Graphics/GPUBufferAllocator.cpp**
- `[25]` **Runtime/Renderer/PerDrawAllocator.h**
- `[25]` **Runtime/Platforms/D3D12/D3D12DescAllocator.cpp**
- `[23]` **Runtime/Graphics/IGPUBufferAllocator.h**
- `[23]` **Runtime/Math/Vector4.h**
- `[22]` **Runtime/Math/Vector2.h**
- `[22]` **Runtime/Core/Concurrency/CpuEvent.h**
- `[21]` **Runtime/Math/Vector4.cpp**
- `[20]` **Runtime/Entry.cpp**
- `[20]` **Runtime/Math/Vector2.cpp**
- `[20]` **Runtime/Core/Concurrency/CpuEvent.cpp**
- `[17]` **Runtime/Renderer/RenderBackend.h**
- `[16]` **Runtime/Core/Object.h**
- `[16]` **Runtime/Serialization/SceneLoader.h**
- `[15]` **Runtime/GameObject/Camera.cpp**
- `[13]` **Runtime/GameObject/Transform.h**
- `[12]` **Runtime/CoreAssert.h**
- `[12]` **Runtime/Core/Game.h**
- `[12]` **Runtime/Core/ThreadSafeQueue.h**
- `[12]` **Runtime/Renderer/RenderSorter.h**
- `[12]` **Runtime/Renderer/RenderUniforms.h**
- `[12]` **Runtime/Serialization/AssetHeader.h**
- `[12]` **Runtime/Serialization/DDSTextureLoader.h**
- `[12]` **Runtime/Serialization/MaterialLoader.h**
- `[12]` **Runtime/Serialization/MeshLoader.h**
- `[12]` **Runtime/Serialization/ShaderLoader.h**
- `[12]` **Runtime/Serialization/StreamHelper.h**
- `[12]` **Runtime/Serialization/TextureLoader.h**
- `[12]` **Runtime/Utils/HashCombine.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp**
- `[11]` **Runtime/Renderer/RenderCommand.h**
- `[10]` **Runtime/GameObject/Transform.cpp**
- `[10]` **Runtime/Renderer/RenderBackend.cpp**
- `[10]` **Runtime/Scene/BistroSceneLoader.cpp**
- `[10]` **Runtime/Scene/GPUScene.h**

## Evidence & Implementation Details

### File: `Runtime/Core/Concurrency/JobSystem.h`
```cpp
namespace EngineCore
{
    struct JobCounter
    {
        std::atomic<int> value;
    };
```
...
```cpp
    };

    struct InternalJob
    {
        void (*function)(void*, void*);
        void* JobData;
        void* rawCounter;
    };
```
...
```cpp
    {
    public:
        JobSystem();
        ~JobSystem();
        static void Create();
        static void Shutdown();
        static JobSystem* GetInstance();

        template<typename CallableJob>
        void KickJob(CallableJob job, JobHandle& handler, JobCounter* counter)
        {
            if(counter == nullptr)
            {
                counter = GetAvaliableCounter();
            }

            void* jobData = new CallableJob(job);

            counter->value.fetch_add(1);
            auto lambda = [](void* jobData, void* rawCounter)
            {
                CallableJob* job = (CallableJob*)jobData;
                job();
                JobCounter* counter = (JobCounter*)rawCounter;
                counter->value.fetch_sub(1);
                delete jobData;
            }
            handler.counter = counter;
            InternalKickJob(lambda, jobData, counter);
        }
```
...
```cpp
        std::vector<std::thread> m_Workers;

        void WaitForJob(JobHandle handle);

    private:
        void InternalKickJob(void (*function)(void*, void*), void* JobData, void* rawCounter);
        void WorkerThreadLoop(); 
        bool TryExecuteOneJob();
        static JobSystem* s_Instance;
        JobCounter* GetAvaliableCounter();
        std::deque<JobCounter*> counterQueue;
    };

};
```

### File: `Runtime/Core/Concurrency/JobSystem.cpp`
```cpp
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
```
...
```cpp
            InternalJob job;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                // wait 为true，就是不sleep，继续走的条件是，
                // isRunning为false了，需要往下走逻辑， 或者是jobQueue有东西了
                // 需要往下pop()； 不然这个线程关不掉？
                wakeWorker.wait(lock, [&]{ return !isRunning || !jobQueue.empty();});
```
...
```cpp
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
```

### File: `Runtime/Math/Matrix4x4.cpp`
```cpp
#include "Vector4.h"

namespace EngineCore
{
    //-------------------------------------------
    // RowMajor的形式
    // D3D12驱动读取的时候会当成ColMajor，相当于做了一次隐式Transpose()
    // 用mul(pos, Matrix)刚好又能和RowMajor对上
    //-------------------------------------------
    Matrix4x4 Matrix4x4::Identity = Matrix4x4();
    Matrix4x4::Matrix4x4()
    {
        m00 = 1.0f; m01 = 0.0f; m02 = 0.0f; m03 = 0.0f;
		m10 = 0.0f; m11 = 1.0f; m12 = 0.0f; m13 = 0.0f;
		m20 = 0.0f; m21 = 0.0f; m22 = 1.0f; m23 = 0.0f;
		m30 = 0.0f; m31 = 0.0f; m32 = 0.0f; m33 = 1.0f;
    }

    Matrix4x4::Matrix4x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33):
        m00(m00), m01(m01), m02(m02), m03(m03),
        m10(m10), m11(m11), m12(m12), m13(m13),
        m20(m20), m21(m21), m22(m22), m23(m23),
        m30(m30), m31(m31), m32(m32), m33(m33)
    {
    }


    // 统一处理成， 左手坐标系，z轴正方向指向target
    // 后续变化在ViewSpace中完成
    Matrix4x4 Matrix4x4::LookAt(const Vector3 &position, const Vector3 &target, const Vector3 &up)
    {
        Vector3 zAxis = Vector3::Normalized((target - position));
        // 左手坐标系：up × forward = right
        Vector3 xAxis = Vector3::Normalized((Vector3::Cross(up, zAxis)));
        // 左手坐标系：forward × right = up
        Vector3 yAxis = Vector3::Normalized((Vector3::Cross(zAxis, xAxis)));
    
        // 这个问题这么理解，首先思考从View -> World，很快能的出来R也就是旋转部分
        // 为什么，因为col方向是原先的基向量（view）在新的空间下的表示（world的xyz）
        //  xAxis.x, yAxis.y, zAxis.z;
        //  xAxis.x, yAxis.y, zAxis.z;
        //  xAxis.x, yAxis.y, zAxis.z;
        // 又因为R的逆矩阵， 是正交矩阵，所以就是 R^T
        // 剩下来的p怎么思考， 就思考p代表世界空间， 需要先根据相机矩阵做一个旋转，得到正确的值
        // 也就是 R^T T， 这是行列式表示，变成公式就是点积
        return Matrix4x4(
            xAxis.x, xAxis.y, xAxis.z, -Vector3::Dot(position, xAxis),
            yAxis.x, yAxis.y, yAxis.z, -Vector3::Dot(position, yAxis),
            zAxis.x, zAxis.y, zAxis.z, -Vector3::Dot(position, zAxis),
                0,      0,      0,          1
        );
    }   

    Matrix4x4 Matrix4x4::Perspective(float mFov, float mAspect, float mNear, float mFar)
    {
        float fovRadians = mFov * 3.14159265359f / 180.0f; 
        //[NDC 0, 1] z轴已经对齐，不用转化
        #ifdef D3D12_API
            return Matrix4x4(
                1.0f/(mAspect * std::tan(fovRadians/2)), 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f / (std::tan(fovRadians/2)), 0.0f, 0.0f,
                0.0f, 0.0f, mFar / (mFar - mNear), -(mFar * mNear) / (mFar - mNear),  // DirectX风格
                0.0f, 0.0f, 1.0f, 0.0f
            );
        #elif
        //[NDC -1, 1] z轴需要反向，不用转化
        #endif
    }

    Matrix4x4 Matrix4x4::TRS(const Vector3 &position, const Quaternion &rotation, const Vector3 &scale)
    {
        // 从四元数计算旋转矩阵的元素
        float xx = rotation.x * rotation.x;
        float yy = rotation.y * rotation.y;
        float zz = rotation.z * rotation.z;
        float xy = rotation.x * rotation.y;
        float xz = rotation.x * rotation.z;
        float yz = rotation.y * rotation.z;
        float wx = rotation.w * rotation.x;
        float wy = rotation.w * rotation.y;
```
...
```cpp
        scale.z = Vector3::Length(axisZ);

        ASSERT(scale.x != 0);
        axisX /= scale.x;
        axisY /= scale.y;
        axisZ /= scale.z;

        quaternion = Quaternion::FromRotationMatrix(axisX, axisY, axisZ);
    }

    // 矩阵乘法 RawMajor
    // 结果矩阵 C = A * B
    // C[i][j] = sum(A[i][k] * B[k][j])
    Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const
    {
```

### File: `Runtime/Math/Matrix4x4.h`
```cpp
namespace EngineCore
{
    class Matrix4x4
    {
    public:
      static Matrix4x4 Identity;
      Matrix4x4();
      Matrix4x4(float m00, float m01, float m02, float m03,
                  float m10, float m11, float m12, float m13,
                  float m20, float m21, float m22, float m23,
                  float m30, float m31, float m32, float m33);
      static Matrix4x4 LookAt(const Vector3& position, const Vector3& target, const Vector3& up);
      static Matrix4x4 Perspective(float mFov, float mAspect, float mNear, float mFar);
      static Matrix4x4 TRS(const Vector3& position, const Quaternion& rotation, const Vector3& scale);
      // 旋转矩阵生成 (参数为角度)
      static Matrix4x4 RotateX(float degrees);
      static Matrix4x4 RotateY(float degrees);
      static Matrix4x4 RotateZ(float degrees);
      
      static Vector4 Matrix4x4::Multiply(const Matrix4x4& matrixA, const Vector4& vector);
      static Matrix4x4 Matrix4x4::Multiply(const Matrix4x4& matrixA, const Matrix4x4& matrixB);
      static void WorldMatrixDecompose(const Matrix4x4& matrix, Vector3& position, Quaternion& quaternion, Vector3& scale); 
      // 矩阵乘法运算符
      Matrix4x4 operator*(const Matrix4x4& other) const;
      inline Vector3 ExtractWorldPosition() const
      {
          return Vector3{m03, m13, m23};
      }
    public:
		  float m00; float m01; float m02; float m03;
		  float m10; float m11; float m12; float m13;
		  float m20; float m21; float m22; float m23;
		  float m30; float m31; float m32; float m33;
    };
```

### File: `Runtime/Math/Plane.h`
```cpp
namespace EngineCore
{
    struct alignas(16) Plane
    {
        // Ax + By + Cz + D = 0 ;
        // normal = (A, B, C) / sqrt(a*a + b*b + c*c);
        // distance = normal * p
        // (A, B, C) / sqrt(a*a + b*b + c*c) * p = distance
        // 又  (A, B, C) · p = -D
        // 同除 sqrt(a*a + b*b + c*c)，左边为distance
        Vector3 normal;
        float distance;
        Plane() = default;
        Plane(const Vector3& normal, float distance);

        // 规定大于0为正向，
        // 假设 0，0，-1， 200， 可以得到平面为正方向为 -z，在z轴200处。
        // p(0, 0, 190)，在平面内， 所以是相加大于0在平面内
        float GetDistanceToPoint(const Vector3& point)
        {
            return (Vector3::Dot(normal, point) + distance);
        }
    };
```

### File: `Runtime/Math/Vector3.h`
```cpp
namespace EngineCore
{
    class Vector3
    {
    public:
        float x;
        float y;
        float z;
    public:
        static Vector3 One;
        static Vector3 Zero;
        static Vector3 Normalized(const Vector3& value);
        static Vector3 Cross(const Vector3& a, const Vector3& b);
        static float Distance(const Vector3& a, const Vector3& b);
        static float Length(const Vector3& a);
        static float Dot(const Vector3& a, const Vector3& b);
        Vector3(){x = 0; y = 0; z = 0;};
        Vector3(float x);
        Vector3(float x, float y, float z);
    
        Vector3 operator-(const Vector3& value) const;
        Vector3 operator+(const Vector3& value) const;
        Vector3 operator*(float value) const;
        Vector3 operator/(float value) const;
        Vector3& operator/=(float value);
    };
```

### File: `Runtime/Core/Allocator/LinearAllocator.h`
```cpp
namespace EngineCore
{
    class LinearAllocator
    {
    public:
        LinearAllocator(uint32_t size)
        {
            buffer.resize(size);
            currentOffset = 0;
        }

        void Reset()
        {
            currentOffset = 0;
        }

        // align of 得到当前class/struct的对齐方式
        void* allocate(uint32_t size, size_t align = alignof(std::max_align_t))
        {
            // 补齐空位， 下一个位置从当前struct align倍数开始，避免出错
            size_t alignedOffset = (currentOffset + align - 1) & ~(align - 1);
            ASSERT(alignedOffset + size <= buffer.size());
            // 这个地方错的， buffer.resize 会导致前面分配的失效。
            // if(alignedOffset + size > buffer.size())
            // {
            //     size_t newSize = buffer.size() * 2;
            //     while (alignedOffset + size > newSize) newSize *= 2;
            //     buffer.resize(newSize);
            // }

            void* ptr = buffer.data() + alignedOffset;
            currentOffset = alignedOffset + size;
            return ptr;
        }

        template<typename T>
        T* allocArray(int size)
        {
            return (T*)allocate(size * sizeof(T), alignof(T));
        }
    private:
        std::vector<uint8_t> buffer;
        size_t currentOffset = 0;
    };
```

### File: `Runtime/Core/PublicStruct.h`
```cpp
#include "Graphics/RenderTexture.h"

namespace EngineCore
{



    // constantbuffer中的变量记录
    struct ShaderConstantInfo
    {
        string variableName;
        ShaderVariableType type;
        int bufferIndex;
        int size;
        int offset;
    };

    // shader中通过反射得到的资源名称、类型
    struct ShaderBindingInfo 
    {
        string resourceName;
        ShaderResourceType type;
        int registerSlot;              
        int size = 0;                  // 对CB有意义，其他资源可为0
        int space = 0;
        int bindCount = 1;             // 绑定数量，数组时 > 1
        ShaderBindingInfo (const string& resourceName, ShaderResourceType type, int registerSlot, int size, int space, int bindCount = 1)
            : resourceName(resourceName), type(type), registerSlot(registerSlot), size(size),
            space(space), bindCount(bindCount)
        {};
    };


    struct ShaderReflectionInfo
    {
        RootSignatureKey mRootSigKey;

        // todo: 确定这个地方是用vector还是直接单个对象
        ShaderStageType type;
        vector<ShaderBindingInfo > mConstantBufferInfo;
        vector<ShaderBindingInfo > mTextureInfo;
        vector<ShaderBindingInfo > mSamplerInfo;
        vector<ShaderBindingInfo > mUavInfo;

        ShaderReflectionInfo(){};


        // 定义偏移量常量 (方便修改)
        static const int BIT_OFFSET_CBV = 0;
        static const int BIT_OFFSET_SRV = 16;
        static const int BIT_OFFSET_UAV = 48;
        static const int BIT_OFFSET_SAMPLER = 56;
    };

    struct LightData
    {

    };


    // 前向声明，防止循环引用。
    class Transform;
    class MeshRenderer;
    class MeshFilter;

    struct RenderPacket
    {
        // 为了测试，先用直接塞数据的方式。
        uint64_t sortingKey = 0;
        AssetID meshID;
        AssetID materialID;
        uint32_t objectIndex;
        float distanToCamera = 0;
    };

    struct PerDrawHandle
    {
        uint8_t* destPtr;
        uint32_t offset;
        uint32_t size;
```
...
```cpp


    struct DrawRecord
    {
        Material* mat;
        Mesh* mesh;

        PerDrawHandle perDrawHandle;
        uint32_t instanceCount = 1;

        DrawRecord(Material* mat, Mesh* mesh)
            :mat(mat), mesh(mesh), perDrawHandle{0,0}, instanceCount(1) {}
        DrawRecord(Material* mat, Mesh* mesh, const PerDrawHandle& handle, uint32_t instCount = 1)
            :mat(mat), mesh(mesh), perDrawHandle(handle), instanceCount(instCount){}
    };
```
...
```cpp
    };

    class RenderPass;
    struct RenderPassAsset
    {
        vector<RenderPass*> renderPasses;
        inline void Clear()
        {
             for (RenderPass* pass : renderPasses) 
                delete pass;
        };
    };
```
...
```cpp
    };

    struct ContextFilterSettings
    {

    };
```
...
```cpp
    {
        uint32_t perObejectIndex = UINT32_MAX;
        inline bool isValid() const {return perObejectIndex != UINT32_MAX;}
```

### File: `Runtime/Core/Profiler.h`
```cpp
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
```
...
```cpp
    };
    
    struct ProfilerFrame
    {
        float frameStartMs = 0;
        const char* frameTag = nullptr;

        std::vector<ProfilerEvent> events;

        std::unordered_map<std::string, uint32_t> counterIndex;

        void Clear()
        {
            events.clear();
            counterIndex.clear();
            frameTag = nullptr;
            frameStartMs = 0.0f;
        }
    };
```
...
```cpp

    
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
```

### File: `Runtime/Core/InstanceID.h`
```cpp
namespace EngineCore
{
    struct InstanceID
    {
        uint64_t v{0};
        explicit operator bool() const { return v != 0; };
    public:
    };
```

### File: `Runtime/Core/PublicEnum.h`
```cpp
#pragma once

namespace EngineCore
{

    enum class ShaderVariableType
    {
        FLOAT, VECTOR2, VECTOR3, VECTOR4, MATRIX4X4, MATRIX3X4, MATRIX3X3, UNKNOWN
    };

    enum class ShaderStageType
    {
        VERTEX_STAGE, FRAGMENT_STAGE
    };

    enum class ShaderResourceType
    {
        CONSTANT_BUFFER,
        TEXTURE,
        SAMPLER,
        UAV
    };

    enum class VertexAttribute
    {
        POSITION,
        NORMAL,
        TANGENT,
        UV0
    };

    enum class Primitive
    {
        Quad,
        Cube,
        Sphere
    };
   
    inline ShaderVariableType InferShaderVaribleTypeBySize(uint32_t size)
    {
        // 以byte为准 比如float为4byte
        switch (size)
        {
        case 4:
            return ShaderVariableType::FLOAT; 
            break;
        case 8:
            return ShaderVariableType::VECTOR2;
            break;
        case 12:
            return ShaderVariableType::VECTOR2;
            break;
        case 16:
            return ShaderVariableType::VECTOR2;
            break;
        case 64:
            return ShaderVariableType::VECTOR2;
            break;
        case 48:
            return ShaderVariableType::VECTOR2;
            break;
        case 36:
            return ShaderVariableType::VECTOR2;
            break;
        default:
            break;
        }
    }
    
    enum class SortingCriteria
    {
        ComonOpaque,
        ComonTransparent
    };
}
```

### File: `Runtime/Math/AABB.h`
```cpp
        AABB() = default;
        AABB(const Vector3& minPoint, const Vector3& maxPoint)
            : minValue(minPoint), maxValue(maxPoint) {}
```
...
```cpp

        /// Transform变换后的AABB（考虑旋转和缩放）
        void Transform(const Matrix4x4& matrix);
    };
}
```

### File: `Runtime/Math/Frustum.h`
```cpp
namespace EngineCore
{
    enum class IntersectResult
    {
        Inside = 0,
        Outside = 1,
        Intersect = 2
    };
```
...
```cpp
        Plane frustumPlane[6];
        Frustum() = default;
        void UpdateFrustumPlane(const Matrix4x4& matrix);
        IntersectResult TestAABB(const AABB& bound);
    };
}
```