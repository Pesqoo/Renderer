#include "MainWindow.h"
#include "Resource.h"
#include "windowsx.h"

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

    if (!m_renderDevice.Initialize(m_hWnd, width, height))
        return false;

    return true;
}

void MainWindow::RenderFrame()
{
    m_renderDevice.RenderFrame();
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
        // Retrieve pointer for other messages
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
            m_renderDevice.Zoom(steps);
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

                m_renderDevice.AddOrbitDelta(dx, dy);

                m_lastMouseX = x;
                m_lastMouseY = y;
            }
            return 0;
        }
        default:
            return DefWindowProcW(m_hWnd, msg, wParam, lParam);
    }
}
