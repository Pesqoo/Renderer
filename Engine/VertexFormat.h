#pragma once
#include <cstdint>

enum class VertexFormat : uint32_t
{
    PC, // float3 position + rgba8 color
};

inline uint32_t VertexStride(VertexFormat fmt)
{
    switch (fmt)
    {
        case VertexFormat::PC: return 16;
        default: return 0;
    }
}

struct ColorRGBA8
{
    uint8_t r, g, b, a;
};

struct VertexPC
{
    float x, y, z;
    ColorRGBA8 color;
};

static_assert(sizeof(VertexPC) == 16);
