#include "MenuBar.h"

#include "Framework/Application.h"

#include <cstdio>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Graphics/Window.h"
#endif

namespace Immortal
{

namespace
{
/** 与 Widget.cpp WRightClickPopup 一致 */
constexpr float kMenuPopupRounding = 4.f;
constexpr float kMenuPopupBorderSize = 1.f;
constexpr float kMenuPopupShadowSize = 0.f;
constexpr float kMenuItemFrameRounding = 4.f;
constexpr float kMenuFontSize = 16.f;
constexpr float kMenuFramePaddingX = 9.f;
constexpr float kMenuFramePaddingY = 8.f;
constexpr ImU32 kMenuBorderCol = IM_COL32(69, 69, 69, 255);
constexpr ImU32 kMenuSepCol = IM_COL32(56, 56, 56, 255);
constexpr ImU32 kMenuBottomBorderCol = IM_COL32(0, 0, 0, 90);

static uint32_t ResolveMenuTextColor(const WMenu *menu)
{
	uint32_t color = menu->Color();
	if (color == 0xff000000 && menu->parent)
	{
		if (auto *menuBar = dynamic_cast<WMenuBar *>(menu->parent))
		{
			color = menuBar->Color();
		}
	}

	return color;
}

static ImVec2 ResolveMenuBarSpacing(const ImVec2 &spacing)
{
	if (spacing.x == 0.0f && spacing.y == 0.0f)
	{
		return ImVec2{ 6.0f, 0.0f };
	}

	return spacing;
}

static const char *ResolveBackendApiName(BackendAPI api)
{
	switch (api)
	{
	case BackendAPI::D3D11:
		return "D3D11";
	case BackendAPI::D3D12:
		return "D3D12";
	case BackendAPI::Vulkan:
		return "Vulkan";
	case BackendAPI::Metal:
		return "Metal";
	case BackendAPI::OpenGL:
		return "OpenGL";
	default:
		return "Unknown";
	}
}

static bool BuildMenuBarPerformanceStatus(char *buffer, size_t bufferSize)
{
	if (!buffer || bufferSize == 0)
	{
		return false;
	}

	buffer[0] = '\0';
	auto *guiLayer = Application::This ? Application::This->GetGuiLayer() : nullptr;
	if (!guiLayer)
	{
		return false;
	}

	std::snprintf(
	    buffer,
	    bufferSize,
	    "%.1f FPS  %s",
	    ImGui::GetIO().Framerate,
	    ResolveBackendApiName(guiLayer->GetBackendAPI()));
	return true;
}

static void DrawMenuBarPerformanceStatusInline(float rightLimit)
{
	char statusText[96] = {};
	if (!BuildMenuBarPerformanceStatus(statusText, sizeof(statusText)))
	{
		return;
	}

	const float gap = 12.0f;
	const float statusWidth = ImGui::CalcTextSize(statusText).x;
	if (ImGui::GetCursorPosX() + gap + statusWidth > rightLimit)
	{
		return;
	}

	ImGui::SameLine(0.0f, gap);
	ImGui::AlignTextToFramePadding();
	ImGui::TextDisabled("%s", statusText);
}

#ifdef _WIN32
static void DrawCaptionChromeMin(const ImVec2 &mn, const ImVec2 &mx, ImU32 col, float glyphScale = 1.f)
{
	ImDrawList *dl = ImGui::GetWindowDrawList();
	const float cx = (mn.x + mx.x) * 0.5f;
	const float cy = (mn.y + mx.y) * 0.5f;
	const float half = 6.f * glyphScale;
	dl->AddLine(ImVec2(cx - half, cy), ImVec2(cx + half, cy), col, 1.35f);
}

static void DrawCaptionChromeMax(const ImVec2 &mn, const ImVec2 &mx, ImU32 col, float glyphScale = 1.f)
{
	ImDrawList *dl = ImGui::GetWindowDrawList();
	const float cx = (mn.x + mx.x) * 0.5f;
	const float cy = (mn.y + mx.y) * 0.5f;
	const float bw = mx.x - mn.x;
	const float bh = mx.y - mn.y;
	const float unit = bw < bh ? bw : bh;
	const float s = (unit - 14.f) * glyphScale;
	const float half = s * 0.5f;
	if (half <= 0.f)
		return;
	dl->AddRect(ImVec2(cx - half, cy - half), ImVec2(cx + half, cy + half), col, 0.f, 0, 1.35f);
}

static void DrawCaptionChromeRestore(const ImVec2 &mn, const ImVec2 &mx, ImU32 col, float glyphScale = 1.f)
{
	ImDrawList *dl = ImGui::GetWindowDrawList();
	const float cx = (mn.x + mx.x) * 0.5f;
	const float cy = (mn.y + mx.y) * 0.5f;
	const float bw = mx.x - mn.x;
	const float bh = mx.y - mn.y;
	const float unit = bw < bh ? bw : bh;
	const float s = unit * 0.34f * glyphScale;
	const float half = s * 0.5f;
	/** 对角偏移较小，两正方形在图标中心区域交叉重叠（类 Win 还原样式） */
	const float d = s * 0.20f;
	dl->AddRect(ImVec2(cx - d - half, cy - d - half), ImVec2(cx - d + half, cy - d + half), col, 0.f, 0, 1.35f);
	dl->AddRect(ImVec2(cx + d - half, cy + d - half), ImVec2(cx + d + half, cy + d + half), col, 0.f, 0, 1.35f);
}

static void DrawCaptionChromeClose(const ImVec2 &mn, const ImVec2 &mx, ImU32 col, float glyphScale = 1.f)
{
	ImDrawList *dl = ImGui::GetWindowDrawList();
	const float cx = (mn.x + mx.x) * 0.5f;
	const float cy = (mn.y + mx.y) * 0.5f;
	const float half = 5.f * glyphScale;
	dl->AddLine(ImVec2(cx - half, cy - half), ImVec2(cx + half, cy + half), col, 1.35f);
	dl->AddLine(ImVec2(cx + half, cy - half), ImVec2(cx - half, cy + half), col, 1.35f);
}
#endif
}

WMenu::WMenu(Widget *parent) :
    Widget{parent}
{

}

bool WMenu::Draw()
{
	const uint32_t textColor = ResolveMenuTextColor(this);
	ImVec4 popbgColor = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
	popbgColor.w *= factor;
	ImGui::PushStyleColor(ImGuiCol_PopupBg, popbgColor);
	StyleColorStack<uint32_t> styleColor{
	    { ImGuiCol_PopupBg, ImGui::ColorConvertFloat4ToU32(popbgColor) },
	    { ImGuiCol_Text, textColor },
	    { ImGuiCol_Border, kMenuBorderCol },
	    { ImGuiCol_Separator, kMenuSepCol },
	};

	StyleVarStack<float> popupMetrics{
	    { ImGuiStyleVar_PopupRounding, kMenuPopupRounding },
	    { ImGuiStyleVar_PopupBorderSize, kMenuPopupBorderSize },
	    { ImGuiStyleVar_WindowShadowSize, kMenuPopupShadowSize },
	    { ImGuiStyleVar_FrameRounding, kMenuItemFrameRounding },
	};
	StyleVarStack<ImVec2> styleVar{
	    { ImGuiStyleVar_WindowPadding, ImVec2{ 8.0f, 8.0f } },
	    { ImGuiStyleVar_ItemSpacing, ImVec2{ 8.0f, 8.0f } },
	};
	FontSizeStack fontSize(kMenuFontSize);
	/** 勿 SetNextWindowSize 固定高度：小于实际内容会出现纵向滚动条；由 ImGui 按内容增高。 */
	if (ImGui::BeginMenu(text.c_str()))
	{
		ImGui::Dummy({0, 0});
		EXPORT_WINDOW
		window->DC.MenuColumns.OffsetLabel = 20.0f;
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
				float padding = 8.0f;
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

	return false;
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
	FontSizeStack fontSize(kMenuFontSize);

	StyleColorStack<uint32_t> styleColor{
	    { ImGuiCol_MenuBarBg, BackgroundColor() },
	    { ImGuiCol_PopupBg, PopupBackgroundColor() },
	    { ImGuiCol_Text, Color() },
	    { ImGuiCol_Header, HoveredColor() },
	    { ImGuiCol_HeaderHovered, HoveredColor() },
	    { ImGuiCol_HeaderActive, HoveredColor() },
	    { ImGuiCol_Border, kMenuBorderCol },
	    { ImGuiCol_Separator, kMenuSepCol },
	};

	StyleVarStack<float> styleVar{
	    { ImGuiStyleVar_PopupRounding, kMenuPopupRounding },
	    { ImGuiStyleVar_PopupBorderSize, kMenuPopupBorderSize },
	    { ImGuiStyleVar_WindowShadowSize, kMenuPopupShadowSize },
	    { ImGuiStyleVar_FrameRounding, kMenuItemFrameRounding },
	};
	StyleVarStack<ImVec2> barStyleVar{
	    { ImGuiStyleVar_FramePadding, ImVec2{ kMenuFramePaddingX, kMenuFramePaddingY } },
	    { ImGuiStyleVar_ItemSpacing, ResolveMenuBarSpacing(spacing) },
	};

	if (ImGui::BeginMainMenuBar())
	{
		position = ImGui::GetItemRectMin();
		auto [x, y] = ImGui::GetWindowSize();
		renderWidth = x;
		renderHeight = y;
		{
			ImDrawList *drawList = ImGui::GetWindowDrawList();
			const ImVec2 barMin = ImGui::GetWindowPos();
			const ImVec2 barMax = barMin + ImGui::GetWindowSize();
			drawList->AddLine(
			    ImVec2{ barMin.x, barMax.y - 1.0f },
			    ImVec2{ barMax.x, barMax.y - 1.0f },
			    kMenuBottomBorderCol,
			    1.0f);
		}

		for (auto &child : children)
		{
			child->Draw();
		}

		float performanceStatusRightLimit = ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x;
		if (OnRightSideDraw())
		{
			performanceStatusRightLimit -= 168.0f;
		}
#ifdef _WIN32
		if (showCaptionButtons)
		{
			Window *win = Application::GetMainWindow();
			if (win && win->GetType() == Window::Type::Win32)
			{
				performanceStatusRightLimit -= 46.0f * 3.0f;
			}
		}
#endif
		DrawMenuBarPerformanceStatusInline(performanceStatusRightLimit);

#ifdef _WIN32
		if (showCaptionButtons)
		{
			Window *win = Application::GetMainWindow();
			if (win && win->GetType() == Window::Type::Win32)
			{
				HWND hwnd = (HWND)win->GetPlatformSpecificHandle();
				if (hwnd)
				{
					ImGuiStyle &st = ImGui::GetStyle();
					const float btnW = 46.0f;
					const float h = ImGui::GetFrameHeight();
					const float total = btnW * 3.0f;
					const float padR = st.WindowPadding.x;
					const float captionButtonStartX = ImGui::GetWindowWidth() - total - padR;
					float rightContentEdge = captionButtonStartX;
					if (auto drawRightSide = OnRightSideDraw())
					{
						drawRightSide(rightContentEdge, h);
					}
					ImGui::SetCursorPosX(captionButtonStartX);

					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, (h - ImGui::GetTextLineHeight()) * 0.5f));
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

					const ImU32 bgNone = IM_COL32(0, 0, 0, 0);
					const ImU32 bgHoverChrome = IM_COL32(53, 53, 58, 255);
					const ImU32 bgPressChrome = IM_COL32(68, 68, 74, 255);
					const ImU32 bgHoverClose = IM_COL32(196, 43, 28, 255);
					const ImU32 bgPressClose = IM_COL32(165, 34, 21, 255);

					const ImU32 chromeGlyph = IM_COL32(204, 204, 204, 255);
					const ImU32 chromeGlyphHi = IM_COL32(255, 255, 255, 255);

					WINDOWPLACEMENT wp = { sizeof(wp) };
					::GetWindowPlacement(hwnd, &wp);
					const bool zoomed = (wp.showCmd == SW_SHOWMAXIMIZED);

					ImGui::PushStyleColor(ImGuiCol_Button, bgNone);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgHoverChrome);
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgPressChrome);
					if (ImGui::InvisibleButton("##immortal_cap_min", ImVec2(btnW, h)))
					{
						win->CaptionButtonMinimize();
					}
					{
						const ImVec2 rmn = ImGui::GetItemRectMin();
						const ImVec2 rmx = ImGui::GetItemRectMax();
						const bool hi = ImGui::IsItemHovered();
						const bool act = ImGui::IsItemActive();
						ImDrawList *dlCap = ImGui::GetWindowDrawList();
						if (act)
						{
							dlCap->AddRectFilled(rmn, rmx, bgPressChrome);
						}
						else if (hi)
						{
							dlCap->AddRectFilled(rmn, rmx, bgHoverChrome);
						}
						DrawCaptionChromeMin(rmn, rmx, hi ? chromeGlyphHi : chromeGlyph);
					}
					ImGui::PopStyleColor(3);
					ImGui::SameLine(0.0f, 0.0f);

					ImGui::PushStyleColor(ImGuiCol_Button, bgNone);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgHoverChrome);
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgPressChrome);
					if (ImGui::InvisibleButton("##immortal_cap_max", ImVec2(btnW, h)))
					{
						win->CaptionButtonMaximizeOrRestore();
					}
					{
						const ImVec2 rmn = ImGui::GetItemRectMin();
						const ImVec2 rmx = ImGui::GetItemRectMax();
						const bool hi = ImGui::IsItemHovered();
						const bool act = ImGui::IsItemActive();
						ImDrawList *dlCap = ImGui::GetWindowDrawList();
						if (act)
						{
							dlCap->AddRectFilled(rmn, rmx, bgPressChrome);
						}
						else if (hi)
						{
							dlCap->AddRectFilled(rmn, rmx, bgHoverChrome);
						}
						const ImU32 g = hi ? chromeGlyphHi : chromeGlyph;
						if (zoomed)
						{
							DrawCaptionChromeRestore(rmn, rmx, g);
						}
						else
						{
							DrawCaptionChromeMax(rmn, rmx, g);
						}
					}
					ImGui::PopStyleColor(3);
					ImGui::SameLine(0.0f, 0.0f);

					ImGui::PushStyleColor(ImGuiCol_Button, bgNone);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgHoverClose);
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgPressClose);
					if (ImGui::InvisibleButton("##immortal_cap_close", ImVec2(btnW, h)))
					{
						::PostMessageW(hwnd, WM_CLOSE, 0, 0);
					}
					{
						const ImVec2 rmn = ImGui::GetItemRectMin();
						const ImVec2 rmx = ImGui::GetItemRectMax();
						const bool hi = ImGui::IsItemHovered();
						const bool act = ImGui::IsItemActive();
						ImDrawList *dlCap = ImGui::GetWindowDrawList();
						if (act)
						{
							dlCap->AddRectFilled(rmn, rmx, bgPressClose);
						}
						else if (hi)
						{
							dlCap->AddRectFilled(rmn, rmx, bgHoverClose);
						}
						DrawCaptionChromeClose(rmn, rmx, hi ? chromeGlyphHi : chromeGlyph);
					}
					ImGui::PopStyleColor(3);

					ImGui::PopStyleVar(4);
				}
			}
		}
#endif

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
