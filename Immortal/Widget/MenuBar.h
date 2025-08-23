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

	std::vector<MenuItem> nextItems;
};

class IMMORTAL_API WMenu : public Widget
{
public:
	WIDGET_SET_PROPERTIES(WMenu)
	WIDGET_PROPERTY_TEXT
	WIDGET_PROPERTY_COLOR
	WIDGET_SET_PROPERTY(HoveredColor, hoveredColor, uint32_t)

public:
	WMenu(Widget *parent = nullptr);

    WMenu *Item(MenuItem &&item);

	WMenu *Sub(MenuItem &&item);

protected:
	std::list<MenuItem> items;
	
	float t = 0.0f;

	float factor = 0.0f;
};

class IMMORTAL_API WMenuBar : public Widget
{
public:
	WIDGET_SET_PROPERTIES(WMenuBar)
	WIDGET_PROPERTY_COLOR
	WIDGET_PROPERTY_BACKGROUND_COLOR
	WIDGET_SET_PROPERTY(PopupBackgroundColor, popupBackgroundColor,   uint32_t, 0xffffffff)
	WIDGET_SET_PROPERTY(HoveredColor, hoveredColor, uint32_t, 0xfff0f0f0)
	WIDGET_SET_PROPERTY_FUNC(Spacing, spacing, ImVec2)
	WIDGET_SET_PROPERTY(OnEvent, onEvent, std::function<void(Event &)>)

public:
	WMenuBar(Widget *parent = nullptr);

	virtual bool Draw() override;

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
	WIDGET_SET_PROPERTY(HoveredColor, hoveredColor, uint32_t)

public:
	WItemList(Widget *parent = nullptr);

	virtual bool Draw() override;

	WItemList *Item(WItem &&item);

private:
	std::list<WItem> items;
};

}
