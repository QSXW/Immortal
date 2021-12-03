#pragma once

#include "ImmortalCore.h"
#include "Format.h"

namespace Immortal
{

struct InputElement
{
    InputElement()
    {
    
    }

    InputElement(Format foramt, const std::string &name) :
        format{ foramt },
        name{ name }
    {

    }

    InputElement(const InputElement &other) :
        offset{ other.offset },
        format{ other.format }
    {
        name = other.name;
    }

    InputElement(InputElement &&other) :
        offset{ other.offset },
        format{ other.format },
        name{ std::move(other.name) }
    {

    }

    InputElement &operator=(const InputElement &other)
    {
        offset = other.offset;
        format = other.format;
        name   = other.name;

        return *this;
    }

    InputElement &operator=(InputElement &&other)
    {
        offset = other.offset;
        format = other.format;
        name   = std::move(other.name);

        return *this;
    }

    ~InputElement()
    {
        
    }

    template <class T>
    T BaseType() const
    {
        return Map::BaseFormat<T>(format);
    }

    uint32_t ComponentCount() const
    {
        return Map::FormatComponentCount(format);
    }

    uint32_t Offset() const
    {
        return offset;
    }

    std::string name;
    uint32_t    offset{ 0 };
    Format      format{ Format::UNDEFINED };
};

struct InputElementDescription
{
    InputElementDescription()
    {
    
    }

    InputElementDescription(const InputElementDescription &other) :
        stride{ other.stride },
        elements{ other.elements.begin(), other.elements.end() }
    {

    }

    InputElementDescription(InputElementDescription &&other) :
        stride{ other.stride },
        elements{ std::move(other.elements) }
    {

    }

    InputElementDescription &operator=(const InputElementDescription &other)
    {
        stride = other.Stride();
        elements.resize(other.elements.size());
        std::copy(other.elements.begin(), other.elements.end(), elements.begin());
        return *this;
    }

    InputElementDescription operator=(InputElementDescription &&other)
    {
        elements.swap(other.elements);
        return *this;
    }

    InputElementDescription(std::initializer_list<InputElement> &&list) :
        elements{ std::move(list) }
    {
        INIT();
    }

    InputElementDescription(std::vector<InputElement> &&elements) :
        elements{ std::move(elements) }
    {
        INIT();
    }

    auto begin()
    {
        return elements.begin();
    }

    auto end()
    {
        return elements.end();
    }

    auto begin() const
    {
        return elements.cbegin();
    }

    auto end() const
    {
        return elements.cend();
    }

    auto operator[](size_t index) const
    {
        return elements[index];
    }

    auto Size() const
    {
        return elements.size();
    }

    uint32_t Stride() const
    {
        return stride;
    }

    bool Empty() const
    {
        return elements.empty();
    }

private:
    void INIT()
    {
        uint32_t offset = 0;
        for (auto &e : elements)
        {
            e.offset =  offset;
            offset += Map::FormatSize(e.format);
        }
        stride = offset;
    }

    std::vector<InputElement> elements;

    uint32_t stride{ 0 };
};

using VertexLayout = InputElementDescription;

class Buffer
{
public:
    using IndexType = uint32_t;

    enum class Type
    {
        Layout,
        Vertex,
        Index,
        Uniform,
        PushConstant
    };

    enum class Usage
    {
        Persistent
    };

public:
    Buffer(Type type) :
        type{ type }
    {
    
    }

    Buffer(Type type, uint32_t size) :
        type{ type },
        size{ size },
        count{ size >> 2 }
    {
    
    }

    virtual ~Buffer() { }

    Type GetType() const
    {
        return type;
    }

    virtual void Update(uint32_t size, const void *data)
    {
        
    }

    uint32_t Size() const 
    {
        return size;
    }

    uint32_t Count() const
    {
        return count;
    }

protected:
    Type type;

    uint32_t size{ 0 };

    uint32_t count{ 0 };
};

using SuperBuffer = Buffer;

}
