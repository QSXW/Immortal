#pragma once

#include "ImmortalCore.h"
#include "Layer.h"

#include <vector>

namespace Immortal
{
class IMMORTAL_API LayerStack
{
public:
    ~LayerStack();

    void PushLayer(Layer *layer);
    void PushOverlay(Layer *overlay);
    void PopLayer(Layer *layer);
    void PopOverlay(Layer* overlay);

    std::vector<Layer*>::iterator begin()
    { 
        return layers.begin();
    }
    
    std::vector<Layer*>::iterator end()
    {
        return layers.end();
    }

private:
    std::vector<Layer*> layers;
    uint32_t layerInsertIndex = 0;
};
}
