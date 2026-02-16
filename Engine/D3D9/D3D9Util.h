#pragma once
#include "VertexFormat.h"
#include <d3d9.h>

static DWORD ToD3D9FVF(VertexFormat fmt)
{
    switch (fmt)
    {
        case VertexFormat::PC:  return D3DFVF_XYZ | D3DFVF_DIFFUSE;
        default:                return 0;
    }
}

static D3DMATRIX ToD3DMatrix(const Mat4& m)
{
    D3DMATRIX r{};
    r._11 = m.m[0];  r._12 = m.m[1];  r._13 = m.m[2];  r._14 = m.m[3];
    r._21 = m.m[4];  r._22 = m.m[5];  r._23 = m.m[6];  r._24 = m.m[7];
    r._31 = m.m[8];  r._32 = m.m[9];  r._33 = m.m[10]; r._34 = m.m[11];
    r._41 = m.m[12]; r._42 = m.m[13]; r._43 = m.m[14]; r._44 = m.m[15];
    return r;
}
