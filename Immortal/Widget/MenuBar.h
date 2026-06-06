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

	virtual bool Draw() override;

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
	WIDGET_SET_PROPERTY(PopupBackgroundColor, popupBackgroundColor, uint32_t, IM_COL32(252, 252, 254, 247))
	WIDGET_SET_PROPERTY(HoveredColor, hoveredColor, uint32_t, IM_COL32(218, 218, 221, 255))
	WIDGET_SET_PROPERTY_FUNC(Spacing, spacing, ImVec2)
	WIDGET_SET_PROPERTY(OnEvent, onEvent, std::function<void(Event &)>)
	WIDGET_SET_PROPERTY(OnRightSideDraw, onRightSideDraw, std::function<void(float &, float)>)

public:
	WMenuBar(Widget *parent = nullptr);

	virtual bool Draw() override;

	/** Win32 无边框窗口：在菜单栏右侧绘制系统风格的最小化 / 最大化 / 关闭（需自行开启）。 */
	WMenuBar *ShowCaptionButtons(bool value)
	{
		showCaptionButtons = value;
		return this;
	}

protected:
	ImVec2 spacing;

	bool showCaptionButtons = false;
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
