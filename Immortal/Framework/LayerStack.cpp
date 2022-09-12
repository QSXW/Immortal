#include "LayerStack.h"

namespace Immortal
{

LayerStack::~LayerStack()
{
	clear();
}

Layer *LayerStack::PushLayer(Layer *layer)
{
    layers.emplace(layers.begin() + layerInsertIndex, layer);
    layerInsertIndex++; 
    return layer;
}

Layer *LayerStack::PushOverlay(Layer *overlay)
{
    layers.emplace_back(overlay);
    return overlay;
}

void LayerStack::PopLayer(Layer *layer)
{
    auto it = std::find(layers.begin(), layers.begin() + layerInsertIndex, layer);
    if (it != layers.begin() + layerInsertIndex)
    {
        layer->OnDetach();
        layers.erase(it);
        layerInsertIndex--;
    }
}

void LayerStack::PopOverlay(Layer *overlay)
{
    auto it = std::find(layers.begin() + layerInsertIndex, layers.end(), overlay);
    if (it != layers.end())
    {
        overlay->OnDetach();
        layers.erase(it);
    }
}

void LayerStack::clear()
{
	for (Layer *layer : layers)
	{
		delete layer;
	}
	layers.clear();
}

}
