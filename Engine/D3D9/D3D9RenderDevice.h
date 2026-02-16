#pragma once
#include "IRenderDevice.h"
#include "Maths.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <wrl/client.h>
#include <vector>

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

class Camera;

class D3D9RenderDevice final : public IRenderDevice
{
public:
    bool Initialize(const RenderInitParams& params) override;

    void BeginFrame() override;
    void EndFrame() override;

    void SetCamera(const Camera* camera) override;

    void DrawIndexed(
        const VertexBufferPtr& vbBase,
        const IndexBufferPtr& ibBase,
        uint32_t indexCount) override;

    VertexBufferPtr CreateVertexBuffer(
        VertexFormat format,
        const void* data,
        uint32_t vertexCount) override;

    IndexBufferPtr CreateIndexBuffer(
        const void* data,
        uint32_t indexCount,
        bool use32Bit) override;

private:
    Microsoft::WRL::ComPtr<IDirect3D9> m_D3D9;
    Microsoft::WRL::ComPtr<IDirect3DDevice9> m_D3D9Device;

    int m_width = 0;
    int m_height = 0;

    const Camera* m_camera = nullptr;
};
