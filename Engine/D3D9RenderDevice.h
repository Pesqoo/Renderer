#pragma once
#include "IRenderDevice.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <chrono>

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

constexpr int CUSTOMFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

class Camera;

struct CUSTOMVERTEX
{
    FLOAT x, y, z;
    DWORD color;
};

class D3D9RenderDevice : public IRenderDevice
{
public:
    ~D3D9RenderDevice() override;

    bool Initialize(const RenderInitParams& params) override;
    void Shutdown() override;

    void BeginFrame() override;
    void EndFrame() override;

    void SetCamera(const Camera* camera) override;

    void DrawIndexed(
        VertexBufferHandle vb,
        IndexBufferHandle ib,
        uint32_t vertexStride,
        uint32_t indexCount) override;

    VertexBufferHandle CreateVertexBuffer(
        const void* data,
        uint32_t vertexStride,
        uint32_t vertexCount) override;

    IndexBufferHandle CreateIndexBuffer(
        const void* data,
        uint32_t indexCount,
        bool use32Bit) override;

    void DestroyVertexBuffer(VertexBufferHandle handle) override;
    void DestroyIndexBuffer(IndexBufferHandle handle) override;

private:
    void ReleaseAllBuffers();

    struct VBEntry
    {
        VBEntry(IDirect3DVertexBuffer9* _vertexBuffer, uint32_t _size, uint32_t _stride, uint32_t _count)
            : vertexBuffer(_vertexBuffer), size(_size), stride(_stride), count(_count) {}

        IDirect3DVertexBuffer9* vertexBuffer = nullptr;
        uint32_t size = 0;
        uint32_t stride = 0;
        uint32_t count = 0;
    };

    struct IBEntry
    {
        IBEntry(IDirect3DIndexBuffer9* _indexBuffer, uint32_t _count, bool _use32)
            : indexBuffer(_indexBuffer), count(_count), use32(_use32) {}

        IDirect3DIndexBuffer9* indexBuffer = nullptr;
        uint32_t count = 0;
        bool use32 = false;
    };

    IDirect3D9* m_D3D9 = nullptr;
    IDirect3DDevice9* m_D3D9Device = nullptr;

    std::vector<VBEntry> m_vertexBuffers;
    std::vector<IBEntry> m_indexBuffers;

    int m_width = 0;
    int m_height = 0;

    const Camera* m_camera = nullptr;
};
