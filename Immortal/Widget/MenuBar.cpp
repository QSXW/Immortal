#include "MenuBar.h"

namespace Immortal
{

WMenu::WMenu(Widget *parent) :
	Widget{parent}
{
	Connect([this] {
		ImVec4 popbgColor = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
		popbgColor.w *= factor;
		ImGui::PushStyleColor(ImGuiCol_PopupBg, popbgColor);
		StyleColorStack<uint32_t> styleColor{
			{ImGuiCol_PopupBg, ImGui::ColorConvertFloat4ToU32(popbgColor)},
		    {ImGuiCol_Text, Color()}};

		StyleVarStack<ImVec2> styleVar{
		    {ImGuiStyleVar_ItemSpacing, {10.0f, 10.0f}},
		};
		FontSizeStack fontSize(17.0f);
		ImGui::SetNextWindowSize({240.0f, ImGui::GetTextLineHeightWithSpacing() * items.size() + 10.0f /*+ (10.0f * (items.size()))*/}, ImGuiCond_Always);
		if (ImGui::BeginMenu(text.c_str()))
		{
			ImGui::Dummy({0, 0});
			EXPORT_WINDOW
			window->DC.MenuColumns.OffsetLabel = 24.0f;
			if (t < 1.0f)
			{
				t += Time::DeltaTime * 8.0f;

				auto easeInOut = [](float t, float b, float c, float d) {
					return c * Math::Sin(t / d * (Math::PI / 2)) + b;
				};

				factor = easeInOut(t, 0.0, 1.0f, 1.0f);
			}
			// factor = std::min(factor + Time::DeltaTime * 4.f, (float) (0.5f * Math::PI));
			for (auto &item : items)
			{
				if (item.type == MenuItemType::Item)
				{
					//ImGui::Dummy(ImVec2(.0f, 0.0f));
					//ImGui::SameLine();
					//if (ImGui::MenuItem(item.name.c_str(), item.tips.c_str()))
					//{
					//	item.callback();
					//}
					ImGui::Dummy(ImVec2(.0f, 0.0f));
					ImGui::SameLine();
					float window_width = 240.0f;
					float padding = 10.0f;
					float menu_item_width = window_width - (2 * padding); 
					//ImGui::SetCursorPosX(padding);
					if (ImGui::Selectable(item.name.c_str(), false, 0, ImVec2(menu_item_width, 0)))
					{
						item.callback();
					}
					ImGui::SameLine();
					ImGui::Dummy(ImVec2(.0f, 0.0f));
					//ImGui::SetCursorPosX(window_width);
				}
				else
				{
					if (ImGui::BeginMenu(item.name.c_str()))
					{
						item.callback();
					}
				}
			}
			ImGui::Dummy({0, 0});
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

WMenu *WMenu::Item(MenuItem &&item)
{
	if (!GuiLayer::IsLanguage(Language::English))
	{
		item.name = Translator::Translate(item.name);
	}
	item.type = MenuItemType::Item;
	items.emplace_back(std::move(item));
	return this;
}

WMenu *WMenu::Sub(MenuItem &&item)
{
	if (!GuiLayer::IsLanguage(Language::English))
	{
		item.name = Translator::Translate(item.name);
	}
	item.type = MenuItemType::Menu;
	items.emplace_back(std::move(item));
	return this;
}

WMenuBar::WMenuBar(Widget *parent) :
	Widget{parent},
	spacing{}
{

}

bool WMenuBar::Draw()
{
	using namespace ImGui;
	__PreCalculateSize();

	StyleColorStack<uint32_t> styleColor{
		{ImGuiCol_MenuBarBg,     BackgroundColor()     },
		{ImGuiCol_PopupBg,       PopupBackgroundColor()},
		{ImGuiCol_Text,          Color()               },
		{ImGuiCol_Header,        HoveredColor()        },
		{ImGuiCol_HeaderHovered, HoveredColor()        },
		{ImGuiCol_HeaderActive,  HoveredColor()        },
	};

	StyleVarStack<float> styleVar{
		{ImGuiStyleVar_PopupRounding, 2.0f},
		{ImGuiStyleVar_WindowShadowSize, 4.0f },
		{ImGuiStyleVar_FrameRounding, 4.0f}
	};

	if (ImGui::BeginMainMenuBar())
	{
		position = ImGui::GetItemRectMin();
		auto [x, y] = ImGui::GetWindowSize();
		renderWidth = x;
		renderHeight = y;

		for (auto &child : children)
		{
			child->render();
		}

		ImGui::EndMainMenuBar();
	}
	return false;
}

WItemList::WItemList(Widget *parent) :
	Widget{parent}
{

}

bool WItemList::Draw()
{
	__PreCalculateSize();

	StyleColorStack<uint32_t> styleColor{
		{ImGuiCol_HeaderHovered, HoveredColor()},
		{ImGuiCol_ButtonHovered, HoveredColor()},
		{ImGuiCol_Text, Color()}
	};

	for (auto &item : items)
	{
		if (ImGui::MenuItem(item.name.c_str()))
		{
			item.callback();
		}
	}

	return true;
}

WItemList *WItemList::Item(WItem &&item)
{
	if (!GuiLayer::IsLanguage(Language::English))
	{
		item.name = Translator::Translate(item.name);
	}
	items.emplace_back(std::move(item));
	return this;
}

}
