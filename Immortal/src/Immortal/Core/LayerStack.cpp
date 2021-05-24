#include "impch.h"
#include "LayerStack.h"

namespace Immortal
{
	LayerStack::~LayerStack()
	{
		for (Layer *layer : m_layers)
		{
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer *layer)
	{
		m_layers.emplace(m_layers.begin() + m_layerInsertIndex, layer);
		m_layerInsertIndex++; 
	}

	void LayerStack::PushOverlay(Layer *overlay)
	{
		m_layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Layer *layer)
	{
		auto it = std::find(m_layers.begin(), m_layers.begin() + m_layerInsertIndex, layer);
		if (it != m_layers.begin() + m_layerInsertIndex)
		{
			layer->OnDetach();
			m_layers.erase(it);
			m_layerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer *overlay)
	{
		auto it = std::find(m_layers.begin() + m_layerInsertIndex, m_layers.end(), overlay);
		if (it != m_layers.end())
		{
			overlay->OnDetach();
			m_layers.erase(it);
		}
	}
}