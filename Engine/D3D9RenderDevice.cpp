#include "D3D9RenderDevice.h"
#include "Camera.h"
#include <algorithm>

D3D9RenderDevice::~D3D9RenderDevice()
{
    Shutdown();
}

bool D3D9RenderDevice::Initialize(const RenderInitParams& params)
{
    m_width = params.width;
    m_height = params.height;

    m_D3D9 = Direct3DCreate9(D3D_SDK_VERSION);
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

    if (FAILED(m_D3D9->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        params.hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &d3dpp,
        &m_D3D9Device)))
    {
        return false;
    }

    m_D3D9Device->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_D3D9Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

    return true;
}

void D3D9RenderDevice::Shutdown()
{
    ReleaseAllBuffers();

    if (m_D3D9Device)
    {
        m_D3D9Device->Release();
        m_D3D9Device = nullptr;
    }

    if (m_D3D9)
    {
        m_D3D9->Release();
        m_D3D9 = nullptr;
    }
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

    // world
    D3DXMATRIX world;
    D3DXMatrixIdentity(&world);
    m_D3D9Device->SetTransform(D3DTS_WORLD, &world);

    // view
    Vec3 _eye = m_camera->GetEye();
    Vec3 _at = m_camera->GetTarget();
    Vec3 _up = m_camera->GetUp();

    D3DXVECTOR3 eye(_eye.x, _eye.y, _eye.z);
    D3DXVECTOR3 at(_at.x, _at.y, _at.z);
    D3DXVECTOR3 up(_up.x, _up.y, _up.z);

    D3DXMATRIX view;
    D3DXMatrixLookAtLH(&view, &eye, &at, &up);
    m_D3D9Device->SetTransform(D3DTS_VIEW, &view);

    // projection
    float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
    D3DXMATRIX proj;
    D3DXMatrixPerspectiveFovLH(
        &proj,
        D3DXToRadian(60.0f),
        aspect,
        0.1f,
        100.0f);
    m_D3D9Device->SetTransform(D3DTS_PROJECTION, &proj);
}

void D3D9RenderDevice::DrawIndexed(
    VertexBufferHandle vbHandle,
    IndexBufferHandle ibHandle,
    uint32_t vertexStride,
    uint32_t indexCount)
{
    if (!m_D3D9Device)
        return;

    if (vbHandle == 0 || vbHandle > m_vertexBuffers.size())
        return;
    if (ibHandle == 0 || ibHandle > m_indexBuffers.size())
        return;

    const auto& vbEntry = m_vertexBuffers[vbHandle - 1];
    const auto& ibEntry = m_indexBuffers[ibHandle - 1];

    if (!vbEntry.vertexBuffer || !ibEntry.indexBuffer)
        return;

    constexpr DWORD CUSTOMFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
    m_D3D9Device->SetFVF(CUSTOMFVF);

    m_D3D9Device->SetStreamSource(0, vbEntry.vertexBuffer, 0, static_cast<INT>(vertexStride));
    m_D3D9Device->SetIndices(ibEntry.indexBuffer);

    UINT primitiveCount = indexCount / 3;

    m_D3D9Device->DrawIndexedPrimitive(
        D3DPT_TRIANGLELIST,
        0,
        0,
        vbEntry.count,
        0,
        primitiveCount);
}

VertexBufferHandle D3D9RenderDevice::CreateVertexBuffer(
    const void* data,
    uint32_t vertexStride,
    uint32_t vertexCount)
{
    if (!m_D3D9Device || !data || vertexStride == 0 || vertexCount == 0)
        return 0;

    IDirect3DVertexBuffer9* vertexBuffer = nullptr;
    const UINT sizeInBytes = vertexStride * vertexCount;

    if (FAILED(m_D3D9Device->CreateVertexBuffer(
        sizeInBytes,
        0,
        0,
        D3DPOOL_MANAGED,
        &vertexBuffer,
        nullptr)))
    {
        return 0;
    }

    void* dst = nullptr;
    if (SUCCEEDED(vertexBuffer->Lock(0, 0, &dst, 0)))
    {
        std::memcpy(dst, data, sizeInBytes);
        vertexBuffer->Unlock();
    }

    auto entry = VBEntry(vertexBuffer, sizeInBytes, vertexStride, vertexCount);
    m_vertexBuffers.push_back(entry);

    return static_cast<VertexBufferHandle>(m_vertexBuffers.size());
}

IndexBufferHandle D3D9RenderDevice::CreateIndexBuffer(
    const void* data,
    std::uint32_t indexCount,
    bool use32Bit)
{
    if (!m_D3D9Device || !data || indexCount == 0)
        return 0;

    IDirect3DIndexBuffer9* indexBuffer = nullptr;
    D3DFORMAT format = use32Bit ? D3DFMT_INDEX32 : D3DFMT_INDEX16;
    UINT indexSize = use32Bit ? 4u : 2u;
    UINT sizeInBytes = indexSize * indexCount;

    if (FAILED(m_D3D9Device->CreateIndexBuffer(
        sizeInBytes,
        0,
        format,
        D3DPOOL_MANAGED,
        &indexBuffer,
        nullptr)))
    {
        return 0;
    }

    void* dst = nullptr;
    if (SUCCEEDED(indexBuffer->Lock(0, 0, &dst, 0)))
    {
        std::memcpy(dst, data, sizeInBytes);
        indexBuffer->Unlock();
    }

    auto entry = IBEntry(indexBuffer, indexCount, use32Bit);
    m_indexBuffers.push_back(entry);

    return static_cast<IndexBufferHandle>(m_indexBuffers.size());
}

void D3D9RenderDevice::DestroyVertexBuffer(VertexBufferHandle handle)
{
    if (handle == 0 || handle > m_vertexBuffers.size())
        return;

    auto& entry = m_vertexBuffers[handle - 1];
    if (entry.vertexBuffer)
    {
        entry.vertexBuffer->Release();
        entry.vertexBuffer = nullptr;
    }
}

void D3D9RenderDevice::DestroyIndexBuffer(IndexBufferHandle handle)
{
    if (handle == 0 || handle > m_indexBuffers.size())
        return;

    auto& entry = m_indexBuffers[handle - 1];
    if (entry.indexBuffer)
    {
        entry.indexBuffer->Release();
        entry.indexBuffer = nullptr;
    }
}

void D3D9RenderDevice::ReleaseAllBuffers()
{
    for (auto& vbEntry : m_vertexBuffers)
    {
        if (vbEntry.vertexBuffer)
        {
            vbEntry.vertexBuffer->Release();
            vbEntry.vertexBuffer = nullptr;
        }
    }
    m_vertexBuffers.clear();

    for (auto& ibEntry : m_indexBuffers)
    {
        if (ibEntry.indexBuffer)
        {
            ibEntry.indexBuffer->Release();
            ibEntry.indexBuffer = nullptr;
        }
    }
    m_indexBuffers.clear();
}
