#pragma once

#include "Core.h"
#include "Event/Event.h"

namespace Immortal
{

class Widget;
class IMMORTAL_API Layer
{
public:
    Layer(const std::string &name = "Layer");

    virtual ~Layer();

    virtual void OnAttach() { }

    virtual void OnDetach() { }

    virtual void OnUpdate() { }

    virtual void OnEvent(Event &e) { }

    const std::string &GetName() const
    {
		return name;
    }

protected:
    std::string name;
};

}
