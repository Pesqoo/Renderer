#pragma once
#include "GPUBuffers.h"
#include <d3d11.h>
#include <wrl/client.h>

class D3D11VertexBuffer final : public VertexBuffer
{
public:
    D3D11VertexBuffer(VertexFormat format, uint32_t stride, uint32_t count) 
        : VertexBuffer(format, stride, count) {}

    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
};

class D3D11IndexBuffer final : public IndexBuffer
{
public:
    D3D11IndexBuffer(uint32_t count, bool is32) : IndexBuffer(count) {}

    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
};
