#pragma once
#include "IRenderDevice.h"
#include "Maths.h"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")


class D3D11RenderDevice final : public IRenderDevice
{
public:
    bool Initialize(const RenderInitParams& params) override;

    void BeginFrame() override;
    void EndFrame() override;

    void SetCamera(const Camera* camera) override;
    void SetWorld(const Mat4& world) override;

    void DrawIndexed(
        const VertexBufferPtr& vb,
        const IndexBufferPtr& ib,
        uint32_t indexCount) override;

    VertexBufferPtr CreateVertexBuffer(
        VertexFormat fmt,
        const void* data,
        uint32_t vertexCount) override;

    IndexBufferPtr CreateIndexBuffer(
        const void* data,
        uint32_t indexCount,
        bool use32Bit) override;

private:
    bool createBackbufferRTV();
    bool createPipeline_PC();

    void createDepthBuffer(UINT width, UINT height);
    void createDepthStencilState();

    void setViewport();

private:
    Microsoft::WRL::ComPtr<ID3D11Device>        m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain>      m_swapChain;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsPC;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_psPC;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_ilPC;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_vsConstants;

    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_dsv;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthState;

    Mat4 m_view = {};
    Mat4 m_proj = {};

    int m_width = 0;
    int m_height = 0;
    bool m_vsync = true;

    const Camera* m_camera = nullptr;
};
