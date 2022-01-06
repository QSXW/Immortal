#pragma once

#include "Core.h"
#include "Layer.h"

#include <vector>

namespace Immortal
{

class IMMORTAL_API LayerStack
{
public:
    ~LayerStack();

    Layer *PushLayer(Layer *layer);

    Layer *PushOverlay(Layer *overlay);

    void PopLayer(Layer *layer);

    void PopOverlay(Layer* overlay);

    auto begin()
    { 
        return layers.begin();
    }
    
    auto end()
    {
        return layers.end();
    }

private:
    std::vector<Layer*> layers;

    uint32_t layerInsertIndex = 0;
};

}
