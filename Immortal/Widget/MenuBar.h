#pragma once

#include "Widget.h"
#include "ImGui/GuiLayer.h"
#include <list>

namespace Immortal
{

enum class MenuItemType
{
	Item,
	Menu
};

struct MenuItem
{
	std::string name;
	std::string tips;
	std::function<void()> callback;
	MenuItemType type;
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
			ImVec4 popbgColor = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
			popbgColor.w *= factor;
			ImGui::PushStyleColor(ImGuiCol_PopupBg, popbgColor);

            if (ImGui::BeginMenu(text.c_str()))
            {
				if (t < 1.0f)
				{
					t += Time::DeltaTime * 8.0f;

					auto easeInOut = [](float t, float b, float c, float d)
					{
						return c * Math::Sin(t / d * (Math::PI / 2)) + b;
					};
					
					factor = easeInOut(t, 0.0, 1.0f, 1.0f);
				}
				// factor = std::min(factor + Time::DeltaTime * 4.f, (float) (0.5f * Math::PI));
			    for (auto &item : items)
				{
					if (item.type == MenuItemType::Item)
					{
						if (ImGui::MenuItem(item.name.c_str(), item.tips.c_str()))
						{
							item.callback();
						}
					}
					else
					{
						if (ImGui::BeginMenu(item.name.c_str()))
						{
							item.callback();
						}
					}
				}
				ImGui::EndMenu();
            }
			else
			{
				t = 0.0f;
				factor = 0.0f;
			}
			ImGui::PopStyleColor();
		});
    }

    WMenu *Item(MenuItem &&item)
    {
		if (!GuiLayer::IsLanguage(Language::English))
		{
			item.name = Translator::Translate(item.name);
		}
		item.type = MenuItemType::Item;
		items.emplace_back(std::move(item));
		return this;
    }

	WMenu *Sub(MenuItem &&item)
	{
		if (!GuiLayer::IsLanguage(Language::English))
		{
			item.name = Translator::Translate(item.name);
		}
		item.type = MenuItemType::Menu;
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
	
	float t = 0.0f;

	float factor = 0.0f;
};

class IMMORTAL_API WMenuBar : public Widget
{
public:
	WIDGET_SET_PROPERTIES(WMenuBar)
	WIDGET_PROPERTY_COLOR
	WIDGET_PROPERTY_BACKGROUND_COLOR

public:
	WMenuBar(Widget *parent = nullptr) :
	    Widget{parent},
	    spacing{}
	{
		Connect([this] {
			__PreCalculateSize();

			ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::PushStyleColor(ImGuiCol_MenuBarBg, backgroundColor);
			ImGui::PushStyleColor(ImGuiCol_PopupBg, backgroundColor);
            if (ImGui::BeginMainMenuBar())
            {
				position = ImGui::GetItemRectMin();
				auto [x, y] = ImGui::GetWindowSize();
				renderWidth = x;
				renderHeight = y;

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, spacing);
				for (auto &child : children)
				{
					child->render();
				}
				ImGui::PopStyleVar(1);
				ImGui::EndMainMenuBar();
            }
			ImGui::PopStyleColor(3);
		});
	}

	WIDGET_SET_PROPERTY_FUNC(Spacing, spacing, ImVec2)

protected:
	ImVec2 spacing;
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
			item.name = Translator::Translate(item.name);
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
