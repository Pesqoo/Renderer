#include "MainWindow.h"
#include "Resource.h"
#include "windowsx.h"

#include "Engine/D3D9/D3D9RenderDevice.h"
#include "Engine/D3D11/D3D11RenderDevice.h"

bool MainWindow::Create(
    HINSTANCE hInstance,
    int width,
    int height)
{
    m_hInstance = hInstance;
    m_width = width;
    m_height = height;

    LoadStringW(hInstance, IDS_APP_TITLE, m_szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_RENDERER, m_szWindowClass, MAX_LOADSTRING);

    WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MainWindow::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RENDERER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = m_szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if (!RegisterClassExW(&wcex))
        return false;

    m_hWnd = CreateWindowExW(
        0,
        m_szWindowClass,
        m_szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 
        CW_USEDEFAULT,
        width, 
        height,
        nullptr,
        nullptr,
        hInstance,
        this);

    if (!m_hWnd)
        return false;

    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    m_renderDevice = std::make_unique<D3D11RenderDevice>();

    RenderInitParams params{};
    params.hWnd = m_hWnd;
    params.width = width;
    params.height = height;
    params.vsync = true;

    if (!m_renderDevice->Initialize(params))
        return false;

    if (!GenerateCube())
        return false;

    return true;
}

void MainWindow::Process(float dt)
{
    m_camera.Process(dt);
}

void MainWindow::Render()
{
    m_renderDevice->BeginFrame();
    m_renderDevice->SetCamera(&m_camera);

    Mat4 worldA = Mat4::Identity();
    m_renderDevice->SetWorld(worldA);
    m_renderDevice->DrawIndexed(m_cubeVB, m_cubeIB, m_cubeIndexCount);

    Mat4 worldB = Mat4::Translation(0.0f, 0.0f, 3.0f);
    m_renderDevice->SetWorld(worldB);
    m_renderDevice->DrawIndexed(m_cubeVB, m_cubeIB, m_cubeIndexCount);

    m_renderDevice->EndFrame();
}

LRESULT CALLBACK MainWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    MainWindow* pThis = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);

        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->m_hWnd = hWnd;
    }
    else
    {
        pThis = reinterpret_cast<MainWindow*>(
            GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }

    if (pThis)
        return pThis->HandleMessage(msg, wParam, lParam);

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

LRESULT MainWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        case WM_MOUSEWHEEL:
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            float steps = static_cast<float>(delta) / WHEEL_DELTA;
            m_camera.Zoom(steps);
            return 0;
        }
        case WM_RBUTTONDOWN:
        {
            m_isOrbiting = true;
            m_lastMouseX = GET_X_LPARAM(lParam);
            m_lastMouseY = GET_Y_LPARAM(lParam);
            SetCapture(m_hWnd);
            return 0;
        }
        case WM_RBUTTONUP:
        {
            m_isOrbiting = false;
            ReleaseCapture();
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            if (m_isOrbiting && (wParam & MK_RBUTTON))
            {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);

                float dx = static_cast<float>(x - m_lastMouseX);
                float dy = static_cast<float>(y - m_lastMouseY);

                m_camera.Orbit(dx, dy);

                m_lastMouseX = x;
                m_lastMouseY = y;
            }
            return 0;
        }
        default:
            return DefWindowProcW(m_hWnd, msg, wParam, lParam);
    }
}

bool MainWindow::GenerateCube()
{
    VertexPC vertices[] =
    {
        // Front (-Z)
        { -1.f, -1.f, -1.f, {   0,   0, 255, 255 } },
        { -1.f,  1.f, -1.f, {   0, 255,   0, 255 } },
        {  1.f,  1.f, -1.f, { 255,   0,   0, 255 } },
        {  1.f, -1.f, -1.f, { 255,   0,   0, 255 } },

        // Back (+Z)
        {  1.f,  1.f,  1.f, {   0,   0, 255, 255 } },
        {  1.f, -1.f,  1.f, {   0, 255,   0, 255 } },
        { -1.f, -1.f,  1.f, {   0,   0, 255, 255 } },
        { -1.f,  1.f,  1.f, {   0, 255,   0, 255 } },
    };

    uint16_t indices[] =
    {
        0, 1, 2, 0, 2, 3, // front
        0, 6, 7, 7, 1, 0, // left
        3, 2, 4, 4, 5, 3, // right
        0, 3, 5, 5, 6, 0, // bottom
        1, 7, 4, 4, 2, 1, // top
        5, 4, 7, 7, 6, 5, // back
    };

    m_cubeVB = m_renderDevice->CreateVertexBuffer(
        VertexFormat::PC,
        vertices,
        static_cast<std::uint32_t>(std::size(vertices)));

    m_cubeIndexCount = static_cast<std::uint32_t>(std::size(indices));
    m_cubeIB = m_renderDevice->CreateIndexBuffer(
        indices,
        m_cubeIndexCount,
        false);

    return m_cubeVB != 0 && m_cubeIB != 0;
}
