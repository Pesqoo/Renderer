#include "D3D9RenderDevice.h"
#include <algorithm>

D3D9RenderDevice::D3D9RenderDevice()
{
    m_startTime = clock::now();
    m_lastFrameTime = m_startTime;
}

D3D9RenderDevice::~D3D9RenderDevice()
{
    if (m_vertexBuffer)
    { 
        m_vertexBuffer->Release(); 
        m_vertexBuffer = nullptr;
    }

    if (m_D3D9Device)
    { 
        m_D3D9Device->Release();       
        m_D3D9Device = nullptr; 
    }

    if (m_D3D9) { 
        m_D3D9->Release();          
        m_D3D9 = nullptr; 
    }
}

bool D3D9RenderDevice::Initialize(HWND hWnd, int width, int height)
{
    m_width = width;
    m_height = height;

    InitDevice(hWnd, width, height);
    if (!m_D3D9Device)
        return false;

    InitGraphics();

    m_D3D9Device->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_D3D9Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

    return true;
}

void D3D9RenderDevice::InitDevice(HWND hWnd, int width, int height)
{
    m_D3D9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (!m_D3D9)
        return;

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));

    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.BackBufferWidth = width;
    d3dpp.BackBufferHeight = height;
    d3dpp.EnableAutoDepthStencil = FALSE;

    if (FAILED(m_D3D9->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp,
        &m_D3D9Device)))
    {
        m_D3D9->Release();
        m_D3D9 = nullptr;
        return;
    }
}

void D3D9RenderDevice::InitGraphics()
{
    CUSTOMVERTEX vertices[] =
    {
        { -1.0f, -1.0f, -1.0f, D3DCOLOR_XRGB(0,   0,   255) },
        { -1.0f,  1.0f, -1.0f, D3DCOLOR_XRGB(0,   255, 0) },
        {  1.0f,  1.0f, -1.0f, D3DCOLOR_XRGB(255, 0,   0) },
        {  1.0f, -1.0f, -1.0f, D3DCOLOR_XRGB(255, 0,   0) },

   
        {  1.0f,  1.0f, 1.0f, D3DCOLOR_XRGB(0,   0,   255) },
        {  1.0f, -1.0f, 1.0f, D3DCOLOR_XRGB(0,   255, 0) },
        { -1.0f, -1.0f, 1.0f, D3DCOLOR_XRGB(0,   0,   255) },
        { -1.0f,  1.0f, 1.0f, D3DCOLOR_XRGB(0,   255, 0) },
    };

    WORD indices[] =
    { 
        0, 1, 2, 0, 2, 3, // front
        0, 6, 7, 7, 1, 0, // left
        3, 2, 4, 4, 5, 3, // right
        0, 3, 5, 5, 6, 0, // bottom
        1, 7, 4, 4, 2, 1, // top
        5, 4, 7, 7, 6, 5, // back
    };

    if (FAILED(m_D3D9Device->CreateVertexBuffer(
        8 * sizeof(CUSTOMVERTEX),
        0,
        CUSTOMFVF,
        D3DPOOL_MANAGED,
        &m_vertexBuffer,
        nullptr)))
    {
        return;
    }

    if (FAILED(m_D3D9Device->CreateIndexBuffer(
        36 * sizeof(WORD),
        0,
        D3DFMT_INDEX16,
        D3DPOOL_MANAGED,
        &m_indexBuffer,
        nullptr)))
    {
        return;
    }

    void* iData = nullptr;
    m_indexBuffer->Lock(0, 0, &iData, 0);
    memcpy(iData, indices, sizeof(indices));
    m_indexBuffer->Unlock();

    void* pVoid = nullptr;
    m_vertexBuffer->Lock(0, 0, &pVoid, 0);
    memcpy(pVoid, vertices, sizeof(vertices));
    m_vertexBuffer->Unlock();
}

void D3D9RenderDevice::SetupTransforms()
{
    using clock = std::chrono::high_resolution_clock;

    auto now = clock::now();
    std::chrono::duration<float> elapsed = now - m_startTime;
    std::chrono::duration<float> frameDelta = now - m_lastFrameTime;
    m_lastFrameTime = now;

    float t = elapsed.count();
    float dt = frameDelta.count();

    const float damping = 10.0f;
    float alpha = 1.0f - std::exp(-damping * dt);
    m_cameraDistance += (m_targetDistance - m_cameraDistance) * alpha;

    // world
    D3DXMATRIX rotX, rotY, world;
    D3DXMatrixRotationX(&rotX, t * 0.7f);
    D3DXMatrixRotationY(&rotY, 1.1f);
    world = rotX * rotY;
    m_D3D9Device->SetTransform(D3DTS_WORLD, &world);

    // view
    D3DXMATRIX view;
    D3DXVECTOR3 eye(0.0f, 0.0f, -m_cameraDistance);
    D3DXVECTOR3 at(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
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

void D3D9RenderDevice::RenderFrame()
{
    if (!m_D3D9Device)
        return;

    m_D3D9Device->Clear(0, nullptr, D3DCLEAR_TARGET,
        D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);

    if (SUCCEEDED(m_D3D9Device->BeginScene()))
    {
        m_D3D9Device->SetFVF(CUSTOMFVF);
        SetupTransforms();

        m_D3D9Device->SetStreamSource(0, m_vertexBuffer, 0, sizeof(CUSTOMVERTEX));
        m_D3D9Device->SetIndices(m_indexBuffer);
        m_D3D9Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);

        m_D3D9Device->EndScene();
    }

    m_D3D9Device->Present(nullptr, nullptr, nullptr, nullptr);
}

void D3D9RenderDevice::Zoom(float steps)
{
    const float zoomSpeed = 2.0f;

    m_targetDistance -= steps * zoomSpeed;
    m_targetDistance = std::clamp(m_targetDistance, 3.0f, 50.0f);

    //m_cameraDistance -= steps * zoomSpeed;
    //m_cameraDistance = std::clamp(m_cameraDistance, 3.0f, 50.0f);
}
