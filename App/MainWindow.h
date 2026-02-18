#pragma once
#include "Windows.h"
#include "IRenderDevice.h"
#include "Camera.h"
#include <memory>

constexpr int MAX_LOADSTRING = 100;

class MainWindow
{
public:
    MainWindow() = default;
    ~MainWindow() = default;

    bool Create(
        HINSTANCE hInstance,
        int width,
        int height);

    void Process(float dt);
    void Render();

    HWND GetHwnd() const { return m_hWnd; }

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    // TODO: move to own mesh
    bool GenerateCube();
    VertexBufferPtr m_cubeVB = 0;
    IndexBufferPtr m_cubeIB = 0;
    std::uint32_t m_cubeIndexCount = 0;

    HWND m_hWnd = nullptr;
    HINSTANCE m_hInstance = nullptr;

    WCHAR m_szTitle[MAX_LOADSTRING] = {};
    WCHAR m_szWindowClass[MAX_LOADSTRING] = {};

    int m_width = 0;
    int m_height = 0;

    Camera m_camera;
    std::unique_ptr<IRenderDevice> m_renderDevice;

    bool m_isOrbiting = false;
    bool m_isPanning = false;
    int m_lastMouseX = 0;
    int m_lastMouseY = 0;
};
