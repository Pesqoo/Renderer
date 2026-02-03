#pragma once
#include "Windows.h"
#include <cstdint>

class Camera;

struct RenderInitParams
{
    HWND  hWnd;
    int   width;
    int   height;
    bool  vsync = true;
};

using VertexBufferHandle = uint32_t;
using IndexBufferHandle = uint32_t;

class IRenderDevice
{
public:
    virtual ~IRenderDevice() = default;

    virtual bool Initialize(const RenderInitParams& params) = 0;
    virtual void Shutdown() = 0;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;

    virtual void SetCamera(const Camera* camera) = 0;

    virtual void DrawIndexed(
        VertexBufferHandle vb,
        IndexBufferHandle  ib,
        uint32_t vertexStride,
        uint32_t indexCount) = 0;

    virtual VertexBufferHandle CreateVertexBuffer(
        const void* data,
        uint32_t vertexStride,
        uint32_t vertexCount) = 0;

    virtual IndexBufferHandle CreateIndexBuffer(
        const void* data,
        uint32_t indexCount,
        bool use32Bit) = 0;

    virtual void DestroyVertexBuffer(VertexBufferHandle handle) = 0;
    virtual void DestroyIndexBuffer(IndexBufferHandle handle) = 0;
};
