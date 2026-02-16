#pragma once
#include "VertexFormat.h"
#include <cstdint>
#include <memory>

class VertexBuffer
{
public:
    VertexBuffer(VertexFormat format, uint32_t stride, uint32_t count) 
        : m_format(format), m_stride(stride), m_count(count) {}
    virtual ~VertexBuffer() = default;

    VertexFormat GetFormat() const { return m_format; }
    uint32_t GetStride() const { return m_stride; }
    uint32_t GetCount()  const { return m_count; }

protected:
    VertexFormat m_format = VertexFormat::PC;
    uint32_t m_stride = 0;
    uint32_t m_count = 0;
};

class IndexBuffer
{
public:
    IndexBuffer(uint32_t count) : m_count(count) {}
    virtual ~IndexBuffer() = default;

    uint32_t GetCount() const { return m_count; }

protected:
    uint32_t m_count = 0;
};

using VertexBufferPtr = std::shared_ptr<VertexBuffer>;
using IndexBufferPtr = std::shared_ptr<IndexBuffer>;
