#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <cstring>

struct alignas(16) VSConstants
{
    float worldViewProj[16];
};

inline static Microsoft::WRL::ComPtr<ID3DBlob> CompileHLSL(
    const char* source,
    const char* entry,
    const char* target)
{
    Microsoft::WRL::ComPtr<ID3DBlob> bytecode;
    Microsoft::WRL::ComPtr<ID3DBlob> errors;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = D3DCompile(
        source, 
        std::strlen(source),
        nullptr, 
        nullptr, 
        nullptr,
        entry, 
        target, 
        flags, 
        0,
        bytecode.GetAddressOf(),
        errors.GetAddressOf());

    if (FAILED(hr))
        return {};

    return bytecode;
}
