#pragma once
#include <windows.h>
#include "D3D9RenderDevice.h"

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

    void RenderFrame();

    HWND GetHwnd() const { return m_hWnd; }
private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hWnd = nullptr;
    HINSTANCE m_hInstance = nullptr;

    WCHAR m_szTitle[MAX_LOADSTRING];
    WCHAR m_szWindowClass[MAX_LOADSTRING];

    int m_width = 0;
    int m_height = 0;

    D3D9RenderDevice m_renderDevice;

    bool m_isOrbiting = false;
    int  m_lastMouseX = 0;
    int  m_lastMouseY = 0;
};
