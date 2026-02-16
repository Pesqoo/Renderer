#pragma once
#include "GPUBuffers.h"
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

class IRenderDevice
{
public:
    virtual ~IRenderDevice() = default;

    virtual bool Initialize(const RenderInitParams& params) = 0;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;

    virtual void SetCamera(const Camera* camera) = 0;

    virtual void DrawIndexed(
        const VertexBufferPtr& vbBase,
        const IndexBufferPtr& ibBase,
        uint32_t indexCount) = 0;

    virtual VertexBufferPtr CreateVertexBuffer(
        VertexFormat format,
        const void* data,
        uint32_t vertexCount) = 0;

    virtual IndexBufferPtr CreateIndexBuffer(
        const void* data,
        uint32_t indexCount,
        bool use32Bit) = 0;
};
