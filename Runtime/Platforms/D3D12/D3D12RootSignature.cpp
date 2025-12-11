#include "PreCompiledHeader.h"
#include "D3D12RootSignature.h"

namespace EngineCore
{
    unordered_map<RootSignatureKey, ComPtr<ID3D12RootSignature>, RootSignatureKey> D3D12RootSignature::mRootSigMap;
}