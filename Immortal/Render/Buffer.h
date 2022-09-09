#pragma once

#include "Core.h"
#include "Format.h"
#include "Interface/IObject.h"

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
        return format;
    }

    uint32_t ComponentCount() const
    {
        return format.ComponentCount();
    }

    uint32_t Offset() const
    {
        return offset;
    }

    const std::string &Name() const
    {
        return name;
    }

    std::string name;
    uint32_t    offset{ 0 };
    Format      format{ Format::None };
};

struct InputElementDescription
{
    InputElementDescription()
    {
    
    }

    InputElementDescription(const InputElementDescription &other) :
        Stride{ other.Stride },
        elements{ other.elements.begin(), other.elements.end() }
    {

    }

    InputElementDescription(InputElementDescription &&other) :
        Stride{ other.Stride },
        elements{ std::move(other.elements) }
    {

    }

    InputElementDescription &operator=(const InputElementDescription &other)
    {
        Stride = other.Stride;
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
        Setup();
    }

    InputElementDescription(std::vector<InputElement> &&elements) :
        elements{ std::move(elements) }
    {
        Setup();
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

    const InputElement &operator[](size_t index) const
    {
        return elements[index];
    }

    auto Size() const
    {
        return elements.size();
    }

    bool Empty() const
    {
        return elements.empty();
    }

private:
    void Setup()
    {
        uint32_t offset = 0;
        for (auto &e : elements)
        {
            e.offset =  offset;
            offset += e.format.Size();
        }
        Stride = offset;
    }

    std::vector<InputElement> elements;

public:
    uint32_t Stride{ 0 };
};

using VertexLayout = InputElementDescription;

class Buffer : public IObject
{
public:
    using IndexType = uint32_t;

    enum class Type
    {
        Layout,
        Vertex              = BIT(1),
        Index               = BIT(2),
        Uniform             = BIT(3),
        PushConstant        = BIT(4),
        TransferSource      = BIT(5),
        TransferDestination = BIT(6),
        Storage             = BIT(7),
        Unspecified         = BIT(31)
    };

    enum class Usage
    {
        Persistent
    };

    struct BindInfo
    {
        void Increase()
        {
            offset += size;
        }

        Buffer::Type type;
        uint32_t size;
        uint32_t offset;
    };

public:
    Buffer() :
        type{ Type::Unspecified }
    {

    }

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

    virtual void Update(uint32_t size, const void *data, uint32_t offset = 0)
    {
        
    }

    virtual Anonymous Descriptor() const
    {
        return nullptr;
    }

    virtual Buffer *Bind(const BindInfo &bindInfo) const
    {
        return nullptr;
    }

    template <class T>
    void Update(const std::vector<T> &data, uint32_t offset = 0)
    {
        Update(U32(data.size() * sizeof(T)), data.data(), offset);
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

namespace Interface
{
    using Buffer = SuperBuffer;
}

SL_DEFINE_BITWISE_OPERATION(Buffer::Type, uint32_t)

}
