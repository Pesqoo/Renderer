#include <cstring>

#include "D3D11RenderDevice.h"
#include "D3D11Buffers.h"
#include "D3D11Util.h"
#include "Camera.h"


using Microsoft::WRL::ComPtr;

bool D3D11RenderDevice::Initialize(const RenderInitParams& params)
{
    m_width = params.width;
    m_height = params.height;
    m_vsync = params.vsync;

    DXGI_SWAP_CHAIN_DESC scd{};
    scd.BufferCount = 2;
    scd.BufferDesc.Width = params.width;
    scd.BufferDesc.Height = params.height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = params.hWnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.Flags = 0;

    UINT deviceFlags = 0;
#if defined(_DEBUG)
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL flOut{};
    const D3D_FEATURE_LEVEL fls[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        deviceFlags,
        fls, 
        (UINT)std::size(fls),
        D3D11_SDK_VERSION,
        &scd,
        m_swapChain.GetAddressOf(),
        m_device.GetAddressOf(),
        &flOut,
        m_context.GetAddressOf());

    if (FAILED(hr) || !m_device || !m_context || !m_swapChain)
        return false;

    if (!createBackbufferRTV())
        return false;

    if (!createPipeline_PC())
        return false;

    createDepthBuffer((UINT)params.width, (UINT)params.height);
    createDepthStencilState();
    setViewport();

    return true;
}

bool D3D11RenderDevice::createBackbufferRTV()
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = m_swapChain->GetBuffer(
        0, 
        __uuidof(ID3D11Texture2D),
        (void**)backBuffer.GetAddressOf());

    if (FAILED(hr) || !backBuffer)
        return false;

    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_rtv.GetAddressOf());
    return SUCCEEDED(hr) && m_rtv;
}

bool D3D11RenderDevice::createPipeline_PC()
{
    auto vsBlob = LoadCSO(L"SimpleVS.cso");
    auto psBlob = LoadCSO(L"SimplePS.cso");

    if (!vsBlob || !psBlob)
        return false;

    HRESULT hr = m_device->CreateVertexShader(
        vsBlob->GetBufferPointer(), 
        vsBlob->GetBufferSize(),
        nullptr, 
        m_vsPC.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = m_device->CreatePixelShader(
        psBlob->GetBufferPointer(), 
        psBlob->GetBufferSize(),
        nullptr, 
        m_psPC.GetAddressOf());
    if (FAILED(hr))
        return false;

    D3D11_INPUT_ELEMENT_DESC il[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = m_device->CreateInputLayout(
        il, 
        (UINT)std::size(il),
        vsBlob->GetBufferPointer(), 
        vsBlob->GetBufferSize(),
        m_ilPC.GetAddressOf());
    if (FAILED(hr)) 
        return false;

    D3D11_BUFFER_DESC cbd{};
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.ByteWidth = sizeof(VSConstants);
    cbd.Usage = D3D11_USAGE_DEFAULT;

    hr = m_device->CreateBuffer(&cbd, nullptr, m_vsConstants.GetAddressOf());
    return SUCCEEDED(hr) && m_vsConstants;
}

void D3D11RenderDevice::createDepthBuffer(UINT width, UINT height)
{
    D3D11_TEXTURE2D_DESC depthDesc{};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ComPtr<ID3D11Texture2D> depthTexture;
    m_device->CreateTexture2D(&depthDesc, nullptr, &depthTexture);

    m_device->CreateDepthStencilView(
        depthTexture.Get(),
        nullptr,
        m_dsv.GetAddressOf());
}

void D3D11RenderDevice::createDepthStencilState()
{
    D3D11_DEPTH_STENCIL_DESC dsDesc{};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    dsDesc.StencilEnable = FALSE;

    m_device->CreateDepthStencilState(
        &dsDesc,
        m_depthState.GetAddressOf());
    m_context->OMSetDepthStencilState(m_depthState.Get(), 0);
}

void D3D11RenderDevice::setViewport()
{
    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = (float)m_width;
    vp.Height = (float)m_height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    m_context->RSSetViewports(1, &vp);
}

void D3D11RenderDevice::BeginFrame()
{
    if (!m_context || !m_rtv)
        return;

    const float clear[4] = { 0.0f, 40.0f / 255.0f, 100.0f / 255.0f, 1.0f };
    m_context->OMSetRenderTargets(1, m_rtv.GetAddressOf(), m_dsv.Get());
    m_context->ClearRenderTargetView(m_rtv.Get(), clear);
    m_context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    m_context->IASetInputLayout(m_ilPC.Get());
    m_context->VSSetShader(m_vsPC.Get(), nullptr, 0);
    m_context->PSSetShader(m_psPC.Get(), nullptr, 0);

    m_context->VSSetConstantBuffers(0, 1, m_vsConstants.GetAddressOf());
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void D3D11RenderDevice::EndFrame()
{
    if (!m_swapChain)
        return;

    m_swapChain->Present(m_vsync ? 1 : 0, 0);
}

void D3D11RenderDevice::SetCamera(const Camera* camera)
{
    m_camera = camera;
    if (!m_context || !m_camera || !m_vsConstants)
        return;

    const float aspect = (m_height != 0) ? (float)m_width / (float)m_height : 1.0f;

    Mat4 world = Mat4::Identity();
    m_view = m_camera->GetViewLH();
    m_proj = m_camera->GetProjLH(aspect);

    Mat4 wv = Mul(world, m_view);
    Mat4 wvp = Mul(wv, m_proj);

    VSConstants cb{};
    Mat4::Transpose(wvp, cb.worldViewProj); // HLSL default column-major
    m_context->UpdateSubresource(m_vsConstants.Get(), 0, nullptr, &cb, 0, 0);
}

void D3D11RenderDevice::SetWorld(const Mat4& world)
{
    Mat4 wv = Mul(world, m_view);
    Mat4 wvp = Mul(wv, m_proj);

    VSConstants cb{};
    Mat4::Transpose(wvp, cb.worldViewProj);
    m_context->UpdateSubresource(m_vsConstants.Get(), 0, nullptr, &cb, 0, 0);
}

void D3D11RenderDevice::DrawIndexed(
    const VertexBufferPtr& vbBase,
    const IndexBufferPtr& ibBase,
    uint32_t indexCount)
{
    if (!m_context || !vbBase || !ibBase || indexCount == 0)
        return;

    auto* vb = static_cast<D3D11VertexBuffer*>(vbBase.get());
    auto* ib = static_cast<D3D11IndexBuffer*>(ibBase.get());
    if (!vb->buffer || !ib->buffer)
        return;

    UINT stride = vbBase->GetStride();
    UINT offset = 0;
    ID3D11Buffer* vbs[] = { vb->buffer.Get() };
    m_context->IASetVertexBuffers(0, 1, vbs, &stride, &offset);

    DXGI_FORMAT idxFmt = DXGI_FORMAT_R16_UINT; // ibBase->Is32Bit() ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
    m_context->IASetIndexBuffer(ib->buffer.Get(), idxFmt, 0);

    m_context->DrawIndexed(indexCount, 0, 0);
}

VertexBufferPtr D3D11RenderDevice::CreateVertexBuffer(
    VertexFormat fmt,
    const void* data,
    uint32_t vertexCount)
{
    if (!m_device || !data || vertexCount == 0)
        return nullptr;

    const uint32_t stride = VertexStride(fmt);
    if (stride == 0)
        return nullptr;

    auto out = std::make_shared<D3D11VertexBuffer>(fmt, stride, vertexCount);

    D3D11_BUFFER_DESC bd{};
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = stride * vertexCount;
    bd.Usage = D3D11_USAGE_IMMUTABLE;

    D3D11_SUBRESOURCE_DATA init{};
    init.pSysMem = data;

    HRESULT hr = m_device->CreateBuffer(&bd, &init, out->buffer.GetAddressOf());
    if (FAILED(hr) || !out->buffer)
        return nullptr;

    return out;
}

IndexBufferPtr D3D11RenderDevice::CreateIndexBuffer(
    const void* data,
    uint32_t indexCount,
    bool use32Bit)
{
    if (!m_device || !data || indexCount == 0)
        return nullptr;

    auto out = std::make_shared<D3D11IndexBuffer>(indexCount, use32Bit);

    const uint32_t indexSize = use32Bit ? 4u : 2u;

    D3D11_BUFFER_DESC bd{};
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.ByteWidth = indexSize * indexCount;
    bd.Usage = D3D11_USAGE_IMMUTABLE;

    D3D11_SUBRESOURCE_DATA init{};
    init.pSysMem = data;

    HRESULT hr = m_device->CreateBuffer(&bd, &init, out->buffer.GetAddressOf());
    if (FAILED(hr) || !out->buffer)
        return nullptr;

    return out;
}
