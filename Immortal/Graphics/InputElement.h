#pragma once

#include "Core.h"
#include "Format.h"
#include "Types.h"

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

    uint32_t GetOffset() const
    {
		return offset;
    }

    const Format &GetFormat() const
    {
		return format;
    }

    const std::string &GetSemanticsName() const
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
        stride = other.stride;
        elements.resize(other.elements.size());
        std::copy(other.elements.begin(), other.elements.end(), elements.begin());
        return *this;
    }

    InputElementDescription operator=(InputElementDescription &&other)
    {
		stride = other.stride;
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

    uint32_t GetStride() const
    {
		return stride;
    }

    void SetStride(uint32_t value)
    {
		stride = value;
    }

protected:
    void Setup()
    {
        uint32_t offset = 0;
        for (auto &e : elements)
        {
            e.offset =  offset;
            offset += e.format.Size();
        }
		stride = offset;
    }

    std::vector<InputElement> elements;

protected:
    uint32_t stride{ 0 };
};

}
