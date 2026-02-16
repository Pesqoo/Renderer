#include <algorithm>

#include "D3D9RenderDevice.h"
#include "Camera.h"
#include "D3D9Buffers.h"
#include "D3D9Util.h"


bool D3D9RenderDevice::Initialize(const RenderInitParams& params)
{
    m_width = params.width;
    m_height = params.height;

    m_D3D9.Attach(Direct3DCreate9(D3D_SDK_VERSION));
    if (!m_D3D9)
        return false;

    D3DPRESENT_PARAMETERS d3dpp{};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = params.hWnd;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.BackBufferWidth = params.width;
    d3dpp.BackBufferHeight = params.height;
    d3dpp.EnableAutoDepthStencil = FALSE;
    d3dpp.PresentationInterval =
        params.vsync ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE;

    HRESULT hr = m_D3D9->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        params.hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &d3dpp,
        m_D3D9Device.GetAddressOf());

    if (FAILED(hr) || !m_D3D9Device)
        return false;

    m_D3D9Device->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_D3D9Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

    return true;
}

void D3D9RenderDevice::BeginFrame()
{
    if (!m_D3D9Device)
        return;

    m_D3D9Device->Clear(
        0,
        nullptr,
        D3DCLEAR_TARGET,
        D3DCOLOR_XRGB(0, 40, 100),
        1.0f,
        0);

    m_D3D9Device->BeginScene();
}

void D3D9RenderDevice::EndFrame()
{
    if (!m_D3D9Device)
        return;

    m_D3D9Device->EndScene();
    m_D3D9Device->Present(nullptr, nullptr, nullptr, nullptr);
}

void D3D9RenderDevice::SetCamera(const Camera* camera)
{
    m_camera = camera;
    if (!m_D3D9Device || !m_camera)
        return;

    const float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);

    Mat4 world = Mat4::Identity();
    Mat4 view = m_camera->GetViewLH();
    Mat4 proj = m_camera->GetProjLH(aspect);

    D3DMATRIX w = ToD3DMatrix(world);
    D3DMATRIX v = ToD3DMatrix(view);
    D3DMATRIX p = ToD3DMatrix(proj);

    m_D3D9Device->SetTransform(D3DTS_WORLD, &w);
    m_D3D9Device->SetTransform(D3DTS_VIEW, &v);
    m_D3D9Device->SetTransform(D3DTS_PROJECTION, &p);
}

void D3D9RenderDevice::DrawIndexed(
    const VertexBufferPtr& vbBase,
    const IndexBufferPtr& ibBase,
    uint32_t indexCount)
{
    if (!m_D3D9Device || !vbBase || !ibBase || indexCount == 0)
        return;

    auto* vb = static_cast<D3D9VertexBuffer*>(vbBase.get());
    auto* ib = static_cast<D3D9IndexBuffer*>(ibBase.get());

    if (!vb->vertexBuffer || !ib->indexBuffer)
        return;

    m_D3D9Device->SetFVF(vb->fvf);
    m_D3D9Device->SetStreamSource(0, vb->vertexBuffer.Get(), 0, (UINT)vb->GetStride());
    m_D3D9Device->SetIndices(ib->indexBuffer.Get());

    const UINT primitiveCount = indexCount / 3;

    m_D3D9Device->DrawIndexedPrimitive(
        D3DPT_TRIANGLELIST,
        0,
        0,
        vbBase->GetCount(),
        0,
        primitiveCount);
}

VertexBufferPtr D3D9RenderDevice::CreateVertexBuffer(
    VertexFormat format,
    const void* data,
    uint32_t vertexCount)
{
    if (!m_D3D9Device || !data || vertexCount == 0)
        return 0;

    const uint32_t stride = VertexStride(format);
    const DWORD fvf = ToD3D9FVF(format);
    if (stride == 0 || fvf == 0)
        return nullptr;

    auto out = std::make_shared<D3D9VertexBuffer>(fvf, format, stride, vertexCount);

    const UINT sizeInBytes = stride * vertexCount;
    HRESULT hr = m_D3D9Device->CreateVertexBuffer(
        sizeInBytes,
        0,
        fvf,
        D3DPOOL_MANAGED,
        out->vertexBuffer.GetAddressOf(),
        nullptr);

    if (FAILED(hr) || !out->vertexBuffer)
        return nullptr;

    void* dst = nullptr;
    hr = out->vertexBuffer->Lock(0, 0, &dst, 0);
    if (FAILED(hr) || !dst)
        return nullptr;

    std::memcpy(dst, data, sizeInBytes);
    out->vertexBuffer->Unlock();

    return out;
}

IndexBufferPtr D3D9RenderDevice::CreateIndexBuffer(
    const void* data,
    std::uint32_t indexCount,
    bool use32Bit)
{
    if (!m_D3D9Device || !data || indexCount == 0)
        return nullptr;

    D3DFORMAT format = use32Bit ? D3DFMT_INDEX32 : D3DFMT_INDEX16;
    auto out = std::make_shared<D3D9IndexBuffer>(format, indexCount);

    const UINT indexSize = use32Bit ? 4u : 2u;
    const UINT sizeInBytes = indexSize * indexCount;

    HRESULT hr = m_D3D9Device->CreateIndexBuffer(
        sizeInBytes,
        0,
        out->format,
        D3DPOOL_MANAGED,
        out->indexBuffer.GetAddressOf(),
        nullptr);

    if (FAILED(hr) || !out->indexBuffer)
        return nullptr;


    void* dst = nullptr;
    hr = out->indexBuffer->Lock(0, 0, &dst, 0);
    if (FAILED(hr) || !dst)
        return nullptr;

    std::memcpy(dst, data, sizeInBytes);
    out->indexBuffer->Unlock();

    return out;
}
