#include "Application.h"
#include <chrono>

constexpr int SCREEN_WIDTH = 1024;
constexpr int SCREEN_HEIGHT = 768;

int Application::Run()
{
    if (!m_mainWindow.Create(m_hInstance, SCREEN_WIDTH, SCREEN_HEIGHT))
        return -1;

    using clock = std::chrono::high_resolution_clock;
    auto lastFrameTime = clock::now();

    int frames = 0;
    float accumSec = 0.0f;

    MSG msg{};
    while (true)
    {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                return static_cast<int>(msg.wParam);

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        const auto now = clock::now();
        const auto frameDelta = now - lastFrameTime;
        const float dt = std::chrono::duration<float>(frameDelta).count();
        lastFrameTime = now;

        frames++;
        accumSec += dt;

        if (accumSec >= 1.0f)
        {
            const float fps = frames / accumSec;
            frames = 0;
            accumSec = 0.0f;

            wchar_t buf[128];
            swprintf_s(buf, L"Renderer - FPS: %.0f", fps);
            SetWindowTextW(m_mainWindow.GetHwnd(), buf);
        }
        
        m_mainWindow.Process(dt);
        m_mainWindow.Render();
    }
}
