#pragma once

#include "Widget.h"
#include "ImGui/GuiLayer.h"
#include <list>

namespace Immortal
{

struct MenuItem
{
	std::string name;
	std::string tips;
	std::function<void()> callback;
};

class IMMORTAL_API WMenu : public Widget
{
public:
	WIDGET_SET_PROPERTIES(WMenu)
	WIDGET_PROPERTY_TEXT
	WIDGET_PROPERTY_COLOR

public:
    WMenu(Widget *parent = nullptr) :
        Widget{ parent }
    {
		Connect([this] {
			WidgetLock lock{this};
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, colors.hovered);
			PUSH_PADDING
            if (ImGui::BeginMenu(text.c_str()))
            {
			    for (auto &item : items)
				{
                    if (ImGui::MenuItem(item.name.c_str(), item.tips.c_str()))
                    {
						item.callback();
                    }
				}
				ImGui::EndMenu();
            }
			POP_PADDING
			ImGui::PopStyleColor(1);
		});
    }

    WMenu *Item(MenuItem &&item)
    {
		if (!GuiLayer::IsLanguage(Language::English))
		{
			item.name = WordsMap::Get(item.name);
		}
		items.emplace_back(std::move(item));
		return this;
    }

	WMenu *HoveredColor(const ImVec4 &color)
	{
		colors.hovered = color;

		return this;
	}

protected:
	std::list<MenuItem> items;

	struct
	{
		ImVec4 hovered;
	} colors;
};

class IMMORTAL_API WMenuBar : public Widget
{
public:
	WIDGET_SET_PROPERTIES(WMenuBar)
	WIDGET_PROPERTY_COLOR

public:
	WMenuBar(Widget *parent = nullptr) :
	    Widget{parent}
	{
		Connect([this] {
			WidgetLock lock{this};
			__PreCalculateSize();
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::PushStyleColor(ImGuiCol_PopupBg, backgroundColor);
            if (ImGui::BeginMenuBar())
            {
				position = ImGui::GetItemRectMin();
				auto [x, y] = ImGui::GetWindowSize();
				renderWidth = x;
				renderHeight = y;

				__RelativeTrampoline();
				ImGui::EndMenuBar();
            }
			ImGui::PopStyleColor(2);
		});
	}

	WIDGET_SET_PROPERTY(BackgroundColor, backgroundColor, const ImVec4 &)

protected:
	ImVec4 backgroundColor;
};

struct WItem
{
	std::string name;
	std::function<void()> callback;
};

class WItemList : public Widget
{
public:
	WIDGET_SET_PROPERTIES(WItemList)
	WIDGET_PROPERTY_COLOR

public:
	WItemList(Widget *parent = nullptr) :
	    Widget{parent}
	{
		Connect([&] {
			__PreCalculateSize();
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, colors.hovered);
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			for (auto &item : items)
			{
				if (ImGui::MenuItem(item.name.c_str()))
				{
					item.callback();
				}
			}
			ImGui::PopStyleColor(2);
		});
	}

	WItemList *Item(WItem &&item)
	{
		if (!GuiLayer::IsLanguage(Language::English))
		{
			item.name = WordsMap::Get(item.name);
		}
		items.emplace_back(std::move(item));
		return this;
	}

	WItemList *HoveredColor(const ImVec4 &color)
	{
		colors.hovered = color;
		return this;
	}

private:
	std::list<WItem> items;

	struct
	{
		ImVec4 hovered;
	} colors;
};

}
