#pragma once
#include "GPUBuffers.h"
#include <d3d9.h>
#include <wrl/client.h>

class D3D9VertexBuffer final : public VertexBuffer
{
public:
	D3D9VertexBuffer(DWORD _fvf, VertexFormat format, uint32_t stride, uint32_t count) 
		: VertexBuffer(format, stride, count), fvf(_fvf) {}

	Microsoft::WRL::ComPtr<IDirect3DVertexBuffer9> vertexBuffer = nullptr;
	DWORD fvf = 0;
};

class D3D9IndexBuffer final : public IndexBuffer
{
public:
	D3D9IndexBuffer(D3DFORMAT _format, uint32_t count) 
		: IndexBuffer(count), format(_format) {}

	Microsoft::WRL::ComPtr<IDirect3DIndexBuffer9> indexBuffer = nullptr;
	D3DFORMAT format = D3DFMT_INDEX16;
};
