
#include "PreCompiledHeader.h"
#include "d3dUtil.h"
#include <comdef.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
using Microsoft::WRL::ComPtr;

namespace
{
    ID3D12Device* gDebugDevice = nullptr;

    std::string WStringToUtf8(const std::wstring& value)
    {
        if (value.empty())
        {
            return {};
        }

        int required = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (required <= 1)
        {
            return {};
        }

        std::string utf8(static_cast<size_t>(required - 1), '\0');
        WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, utf8.data(), required, nullptr, nullptr);
        return utf8;
    }

    const wchar_t* GetAllocationTypeName(D3D12_DRED_ALLOCATION_TYPE allocationType)
    {
        switch (allocationType)
        {
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_QUEUE: return L"CommandQueue";
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_ALLOCATOR: return L"CommandAllocator";
        case D3D12_DRED_ALLOCATION_TYPE_PIPELINE_STATE: return L"PipelineState";
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_LIST: return L"CommandList";
        case D3D12_DRED_ALLOCATION_TYPE_FENCE: return L"Fence";
        case D3D12_DRED_ALLOCATION_TYPE_DESCRIPTOR_HEAP: return L"DescriptorHeap";
        case D3D12_DRED_ALLOCATION_TYPE_HEAP: return L"Heap";
        case D3D12_DRED_ALLOCATION_TYPE_QUERY_HEAP: return L"QueryHeap";
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_SIGNATURE: return L"CommandSignature";
        case D3D12_DRED_ALLOCATION_TYPE_PIPELINE_LIBRARY: return L"PipelineLibrary";
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER: return L"VideoDecoder";
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_PROCESSOR: return L"VideoProcessor";
        case D3D12_DRED_ALLOCATION_TYPE_RESOURCE: return L"Resource";
        case D3D12_DRED_ALLOCATION_TYPE_PASS: return L"Pass";
        case D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSION: return L"CryptoSession";
        case D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSIONPOLICY: return L"CryptoSessionPolicy";
        case D3D12_DRED_ALLOCATION_TYPE_PROTECTEDRESOURCESESSION: return L"ProtectedResourceSession";
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER_HEAP: return L"VideoDecoderHeap";
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_POOL: return L"CommandPool";
        case D3D12_DRED_ALLOCATION_TYPE_COMMAND_RECORDER: return L"CommandRecorder";
        case D3D12_DRED_ALLOCATION_TYPE_STATE_OBJECT: return L"StateObject";
        case D3D12_DRED_ALLOCATION_TYPE_METACOMMAND: return L"MetaCommand";
        case D3D12_DRED_ALLOCATION_TYPE_SCHEDULINGGROUP: return L"SchedulingGroup";
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_ESTIMATOR: return L"VideoMotionEstimator";
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_VECTOR_HEAP: return L"VideoMotionVectorHeap";
        case D3D12_DRED_ALLOCATION_TYPE_VIDEO_EXTENSION_COMMAND: return L"VideoExtensionCommand";
        default: return L"Unknown";
        }
    }

    void AppendInfoQueueMessages(std::wstring& msg)
    {
        ComPtr<ID3D12InfoQueue> infoQueue;
        if (!gDebugDevice || FAILED(gDebugDevice->QueryInterface(IID_PPV_ARGS(&infoQueue))))
        {
            return;
        }

        const UINT64 messageCount = infoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();
        const UINT64 firstMessage = (messageCount > 8) ? (messageCount - 8) : 0;
        for (UINT64 i = firstMessage; i < messageCount; ++i)
        {
            SIZE_T messageLength = 0;
            if (FAILED(infoQueue->GetMessage(i, nullptr, &messageLength)) || messageLength == 0)
            {
                continue;
            }

            std::vector<char> bytes(messageLength);
            auto* message = reinterpret_cast<D3D12_MESSAGE*>(bytes.data());
            if (FAILED(infoQueue->GetMessage(i, message, &messageLength)))
            {
                continue;
            }

            msg += L"\n[D3D12] ";
            msg += AnsiToWString(message->pDescription ? message->pDescription : "");
        }
    }

    void AppendDredData(std::wstring& msg)
    {
        if (!gDebugDevice)
        {
            return;
        }

        ComPtr<ID3D12DeviceRemovedExtendedData1> dred;
        if (FAILED(gDebugDevice->QueryInterface(IID_PPV_ARGS(&dred))))
        {
            return;
        }

        D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1 breadcrumbs = {};
        if (SUCCEEDED(dred->GetAutoBreadcrumbsOutput1(&breadcrumbs)))
        {
            msg += L"\n[DRED] AutoBreadcrumbs:";
            const D3D12_AUTO_BREADCRUMB_NODE1* node = breadcrumbs.pHeadAutoBreadcrumbNode;
            int nodeCount = 0;
            while (node && nodeCount < 16)
            {
                const wchar_t* cmdListName = node->pCommandListDebugNameW ? node->pCommandListDebugNameW : L"<unnamed>";
                const wchar_t* queueName = node->pCommandQueueDebugNameW ? node->pCommandQueueDebugNameW : L"<unnamed>";
                UINT32 lastBreadcrumb = node->pLastBreadcrumbValue ? *node->pLastBreadcrumbValue : 0;
                msg += L"\n  CommandList=";
                msg += cmdListName;
                msg += L", Queue=";
                msg += queueName;
                msg += L", LastOp=";
                msg += std::to_wstring(lastBreadcrumb);
                msg += L"/";
                msg += std::to_wstring(node->BreadcrumbCount);
                node = node->pNext;
                ++nodeCount;
            }
        }

        D3D12_DRED_PAGE_FAULT_OUTPUT1 pageFault = {};
        if (SUCCEEDED(dred->GetPageFaultAllocationOutput1(&pageFault)))
        {
            msg += L"\n[DRED] PageFaultVA=0x";
            std::wstringstream stream;
            stream << std::hex << pageFault.PageFaultVA;
            msg += stream.str();

            const D3D12_DRED_ALLOCATION_NODE1* allocationNode = pageFault.pHeadExistingAllocationNode;
            int allocationCount = 0;
            while (allocationNode && allocationCount < 16)
            {
                msg += L"\n  ExistingAllocation=";
                msg += allocationNode->ObjectNameW ? allocationNode->ObjectNameW : L"<unnamed>";
                msg += L", Type=";
                msg += GetAllocationTypeName(allocationNode->AllocationType);
                allocationNode = allocationNode->pNext;
                ++allocationCount;
            }

            allocationNode = pageFault.pHeadRecentFreedAllocationNode;
            allocationCount = 0;
            while (allocationNode && allocationCount < 16)
            {
                msg += L"\n  RecentFreedAllocation=";
                msg += allocationNode->ObjectNameW ? allocationNode->ObjectNameW : L"<unnamed>";
                msg += L", Type=";
                msg += GetAllocationTypeName(allocationNode->AllocationType);
                allocationNode = allocationNode->pNext;
                ++allocationCount;
            }
        }
    }
}

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber) :
    ErrorCode(hr),
    FunctionName(functionName),
    Filename(filename),
    LineNumber(lineNumber)
{
}

bool d3dUtil::IsKeyDown(int vkeyCode)
{
    return (GetAsyncKeyState(vkeyCode) & 0x8000) != 0;
}

std::string d3dUtil::ToString(HRESULT hr)
{
    std::stringstream ss;
    ss << std::hex << std::uppercase << static_cast<unsigned long>(hr);
    return ss.str();
}

void d3dUtil::SetDebugDevice(ID3D12Device* device)
{
    gDebugDevice = device;
}

ID3D12Device* d3dUtil::GetDebugDevice()
{
    return gDebugDevice;
}

std::wstring d3dUtil::GetDetailedHRESULTMessage(HRESULT hr)
{
    _com_error err(hr);
    std::wstring msg = err.ErrorMessage();

    if (gDebugDevice)
    {
        HRESULT removedReason = gDebugDevice->GetDeviceRemovedReason();
        if (removedReason != S_OK)
        {
            _com_error removedErr(removedReason);
            msg += L" | DeviceRemovedReason: ";
            msg += removedErr.ErrorMessage();
            msg += L" (0x" + AnsiToWString(ToString(removedReason)) + L")";
            AppendDredData(msg);
        }

#if defined(_DEBUG)
        AppendInfoQueueMessages(msg);
#endif
    }

    return msg;
}

void d3dUtil::WriteDiagnosticLog(const std::wstring& text)
{
    std::string logDirectory = EngineCore::PathSettings::GetExecutablePath();
    if (logDirectory.empty())
    {
        logDirectory = ".";
    }

    std::replace(logDirectory.begin(), logDirectory.end(), '\\', '/');
    if (!logDirectory.empty() && logDirectory.back() != '/')
    {
        logDirectory += '/';
    }
    logDirectory += "Logs";

    std::error_code ec;
    std::filesystem::create_directories(logDirectory, ec);
    std::string logPath = logDirectory + "/TinyEngine_D3D12.log";

    std::ofstream out(logPath, std::ios::app | std::ios::binary);
    if (!out.is_open())
    {
        return;
    }

    SYSTEMTIME now = {};
    GetLocalTime(&now);
    std::ostringstream header;
    header << "[" << std::setfill('0')
        << std::setw(4) << now.wYear << "-"
        << std::setw(2) << now.wMonth << "-"
        << std::setw(2) << now.wDay << " "
        << std::setw(2) << now.wHour << ":"
        << std::setw(2) << now.wMinute << ":"
        << std::setw(2) << now.wSecond << "."
        << std::setw(3) << now.wMilliseconds << "] ";
    out << header.str() << WStringToUtf8(text) << "\r\n";
}

void d3dUtil::ReportFailure(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber)
{
    std::wstring text = functionName + L" failed in " + filename + L"; line " +
        std::to_wstring(lineNumber) + L"; error: " + GetDetailedHRESULTMessage(hr) + L"\n";

    OutputDebugStringW(text.c_str());
    std::wcerr << text;
    WriteDiagnosticLog(text);
}

ComPtr<ID3DBlob> d3dUtil::LoadBinary(const std::wstring& filename)
{
    std::ifstream fin(filename, std::ios::binary);

    fin.seekg(0, std::ios_base::end);
    std::ifstream::pos_type size = (int)fin.tellg();
    fin.seekg(0, std::ios_base::beg);

    ComPtr<ID3DBlob> blob;
    ThrowIfFailed(D3DCreateBlob(size, blob.GetAddressOf()));

    fin.read((char*)blob->GetBufferPointer(), size);
    fin.close();

    return blob;
}

Microsoft::WRL::ComPtr<ID3D12Resource> d3dUtil::CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    const void* initData,
    UINT64 byteSize,
    Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
{
    ComPtr<ID3D12Resource> defaultBuffer;

    // Create the actual default buffer resource.
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

    // In order to copy CPU memory data into our default buffer, we need to create
    // an intermediate upload heap. 
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.GetAddressOf())));


    // Describe the data we want to copy into the default buffer.
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    subResourceData.RowPitch = byteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    // Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
    // will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
    // the intermediate upload heap data will be copied to mBuffer.
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), 
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
    UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

    // Note: uploadBuffer has to be kept alive after the above function calls because
    // the command list has not been executed yet that performs the actual copy.
    // The caller can Release the uploadBuffer after it knows the copy has been executed.


    return defaultBuffer;
}

// uploadbuffer不需要
Microsoft::WRL::ComPtr<ID3D12Resource> d3dUtil::CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    const void* initData,
    UINT64 byteSize)
{
    ComPtr<ID3D12Resource> defaultBuffer;
    ComPtr<ID3D12Resource> uploadBuffer;

    // Create the actual default buffer resource.
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

    // In order to copy CPU memory data into our default buffer, we need to create
    // an intermediate upload heap. 
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.GetAddressOf())));


    // Describe the data we want to copy into the default buffer.
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    subResourceData.RowPitch = byteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    // Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
    // will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
    // the intermediate upload heap data will be copied to mBuffer.
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), 
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
    UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

    // Note: uploadBuffer has to be kept alive after the above function calls because
    // the command list has not been executed yet that performs the actual copy.
    // The caller can Release the uploadBuffer after it knows the copy has been executed.


    return defaultBuffer;
}

ComPtr<ID3DBlob> d3dUtil::CompileShader(
	const std::wstring& filename,
	const D3D_SHADER_MACRO* defines,
	const std::string& entrypoint,
	const std::string& target)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ComPtr<ID3DBlob> byteCode = nullptr;
	ComPtr<ID3DBlob> errors;
	hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

	if(errors != nullptr)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	ThrowIfFailed(hr);

	return byteCode;
}

DXGI_FORMAT d3dUtil::GetResourceFormat(const EngineCore::TextureFormat& format)
{
    switch (format)
    {
        case EngineCore::TextureFormat::R8G8B8A8: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case EngineCore::TextureFormat::R16Float: return DXGI_FORMAT_R16_FLOAT;
        case EngineCore::TextureFormat::D24S8: return DXGI_FORMAT_R24G8_TYPELESS; // 深度设置为typeless，方便DSV/SRV用不同的格式。
        case EngineCore::TextureFormat::D32S8: return DXGI_FORMAT_R32G8X24_TYPELESS; // 深度设置为typeless，方便DSV/SRV用不同的格式。
        case EngineCore::TextureFormat::DXT1: return DXGI_FORMAT_BC1_UNORM;
        case EngineCore::TextureFormat::DXT5: return DXGI_FORMAT_BC3_UNORM;
        case EngineCore::TextureFormat::BC7: return DXGI_FORMAT_BC7_UNORM;
        case EngineCore::TextureFormat::BC7_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;
        default: ASSERT(false); return DXGI_FORMAT_UNKNOWN;
    }
}

std::wstring DxException::ToString()const
{
    return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) +
        L"; error: " + d3dUtil::GetDetailedHRESULTMessage(ErrorCode);
}

D3D12_RESOURCE_DIMENSION d3dUtil::GetD3DDimension(const EngineCore::TextureDimension& dimension)
{
    switch (dimension)
    {
    case EngineCore::TextureDimension::TEXTURE2D : return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    
    default:
        break;
    }
}


