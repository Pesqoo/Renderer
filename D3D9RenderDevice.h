#pragma once
#include "Camera.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <chrono>

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

constexpr int CUSTOMFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

struct CUSTOMVERTEX
{
    FLOAT x, y, z;
    DWORD color;
};

class D3D9RenderDevice
{
public:
    D3D9RenderDevice() = default;
    ~D3D9RenderDevice();

    bool Initialize(HWND hWnd, int width, int height);
    void SetCamera(const Camera* camera) { m_camera = camera; }

    void Render();

private:
    void InitDevice(HWND hWnd, int width, int height);
    void InitGraphics();
    void SetupTransforms();

private:
    LPDIRECT3D9              m_D3D9 = nullptr;
    LPDIRECT3DDEVICE9        m_D3D9Device = nullptr;

    LPDIRECT3DVERTEXBUFFER9  m_vertexBuffer = nullptr;
    LPDIRECT3DINDEXBUFFER9   m_indexBuffer = nullptr;

    int m_width = 0;
    int m_height = 0;

    const Camera* m_camera = nullptr;
};
