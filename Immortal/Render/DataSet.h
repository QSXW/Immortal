#pragma once

#include "Buffer.h"

namespace Immortal
{

namespace DataSet
{

template <class T>
class Vertex
{
public:
    Vertex(const std::initializer_list<T> &&list, std::vector<uint32_t> &&indices, InputElementDescription &&description) :
        vertices{ list },
        indices{ std::move(indices) },
        inputElementDescription{ description }
    {

    }

    const std::vector<T> &Vertices() const
    {
        return vertices;
    }

    const std::vector<uint32_t> &Indices() const
    {
        return indices;
    }

    const InputElementDescription &Description() const
    {
        return inputElementDescription;
    }

    std::vector<T> vertices;

    std::vector<uint32_t> indices;

    InputElementDescription inputElementDescription;
};

namespace Classic
{

static Vertex<float> Triangle{
    {
         0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f,
         0.0f, -0.5f, 0.0f, 0.5f, 0.0f, 1.0f
    },
    {
        0, 1, 2
    },
    {
        { Format::VECTOR3, "POSITION" },
        { Format::VECTOR3, "COLOR"    }
    }
};

}

}

}
