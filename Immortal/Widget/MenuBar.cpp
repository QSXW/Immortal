#include "MenuBar.h"

#include "Framework/Application.h"

#include <algorithm>
#include <cmath>
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
constexpr float kMenuPopupRounding = 8.f;
constexpr float kMenuPopupBorderSize = 1.f;
constexpr float kMenuBarShadowSize = 0.f;
constexpr float kMenuPopupShadowSize = 8.f;
constexpr float kMenuItemFrameRounding = 5.f;
constexpr float kMenuFontSize = 16.f;
constexpr float kMenuFramePaddingX = 10.f;
constexpr float kMenuFramePaddingY = 10.f;
constexpr float kMenuPopupPadding = 8.f;
constexpr float kMenuItemHeight = 34.f;
constexpr float kMenuItemTextPadding = 12.f;
constexpr float kMenuShortcutGap = 36.f;
constexpr float kMenuMinItemWidth = 220.f;
constexpr float kMenuMaxItemWidth = 320.f;
constexpr ImU32 kMenuSepCol = IM_COL32(255, 255, 255, 18);
constexpr ImU32 kMenuBottomBorderCol = IM_COL32(255, 255, 255, 14);
constexpr ImU32 kMenuPopupShadowStartCol = IM_COL32(0, 0, 0, 58);
constexpr ImU32 kMenuPopupShadowEndCol = IM_COL32(0, 0, 0, 0);
constexpr ImU32 kMenuShortcutTextCol = IM_COL32(174, 178, 186, 255);
constexpr ImU32 kPerformanceChipBgCol = IM_COL32(255, 255, 255, 9);
constexpr ImU32 kPerformanceChipBorderCol = IM_COL32(255, 255, 255, 15);
constexpr ImU32 kPerformanceTextCol = IM_COL32(166, 171, 181, 235);

static ImU32 MultiplyAlpha(ImU32 color, float alpha)
{
	ImVec4 value = ImGui::ColorConvertU32ToFloat4(color);
	value.w *= std::clamp(alpha, 0.0f, 1.0f);
	return ImGui::ColorConvertFloat4ToU32(value);
}

static const WMenuBar *ResolveParentMenuBar(const WMenu *menu)
{
	return menu && menu->parent ? dynamic_cast<const WMenuBar *>(menu->parent) : nullptr;
}

static uint32_t ResolveMenuTextColor(const WMenu *menu)
{
	uint32_t color = menu->Color();
	if (color == 0xff000000 && menu->parent)
	{
		if (auto *menuBar = ResolveParentMenuBar(menu))
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

static float EaseOutCubic(float value)
{
	const float inverse = 1.0f - std::clamp(value, 0.0f, 1.0f);
	return 1.0f - inverse * inverse * inverse;
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

static void DrawMenuBarPerformanceStatus(
	ImDrawList *drawList,
	const ImRect &reservedRegion,
	float leftLimit,
	float &rightEdge,
	ImU32 accentColor)
{
	char statusText[96] = {};
	if (!drawList || !BuildMenuBarPerformanceStatus(statusText, sizeof(statusText)))
	{
		return;
	}

	const ImVec2 textSize = ImGui::CalcTextSize(statusText);
	const float chipHeight = std::min(24.0f, reservedRegion.GetHeight() - 6.0f);
	const float chipWidth = textSize.x + 34.0f;
	const float chipLeft = rightEdge - chipWidth;
	if (chipHeight <= 0.0f || chipLeft < leftLimit)
	{
		return;
	}

	const float chipTop = reservedRegion.Min.y + (reservedRegion.GetHeight() - chipHeight) * 0.5f;
	const ImRect chipRect{ ImVec2{ chipLeft, chipTop }, ImVec2{ rightEdge, chipTop + chipHeight } };
	drawList->AddRectFilled(chipRect.Min, chipRect.Max, kPerformanceChipBgCol, 6.0f);
	drawList->AddRect(chipRect.Min, chipRect.Max, kPerformanceChipBorderCol, 6.0f, 0, 1.0f);
	const ImVec2 dotCenter{ chipRect.Min.x + 12.0f, chipRect.GetCenter().y };
	drawList->AddCircleFilled(dotCenter, 2.5f, accentColor, 12);
	drawList->AddText(
		ImVec2{ chipRect.Min.x + 20.0f, chipRect.Min.y + (chipHeight - textSize.y) * 0.5f },
		kPerformanceTextCol,
		statusText);
	rightEdge = chipRect.Min.x - 8.0f;
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
	const WMenuBar *menuBar = ResolveParentMenuBar(this);
	const ImU32 hoverColor = HoveredColor() != 0
		? HoveredColor()
		: (menuBar ? menuBar->HoveredColor() : IM_COL32(255, 255, 255, 16));
	const ImU32 pressedColor = menuBar ? menuBar->PressedColor() : IM_COL32(74, 115, 168, 150);
	const ImU32 accentColor = menuBar ? menuBar->AccentColor() : IM_COL32(96, 165, 250, 255);
	const ImU32 borderColor = menuBar ? menuBar->PopupBorderColor() : IM_COL32(255, 255, 255, 24);

	float widestLabel = 0.0f;
	float widestShortcut = 0.0f;
	bool hasShortcut = false;
	for (const auto &item : items)
	{
		widestLabel = std::max(widestLabel, ImGui::CalcTextSize(item.name.c_str()).x);
		if (!item.tips.empty())
		{
			hasShortcut = true;
			widestShortcut = std::max(widestShortcut, ImGui::CalcTextSize(item.tips.c_str()).x);
		}
	}
	const float shortcutWidth = hasShortcut ? kMenuShortcutGap + widestShortcut : 0.0f;
	const float menuItemWidth = std::clamp(
		kMenuItemTextPadding * 2.0f + widestLabel + shortcutWidth,
		kMenuMinItemWidth,
		kMenuMaxItemWidth);

	const float popupOpacity = 0.22f + 0.78f * factor;
	ImVec4 popbgColor = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
	popbgColor.w *= popupOpacity;
	StyleColorStack<uint32_t> styleColor{
	    { ImGuiCol_PopupBg, ImGui::ColorConvertFloat4ToU32(popbgColor) },
	    { ImGuiCol_Text, textColor },
	    { ImGuiCol_Border, MultiplyAlpha(borderColor, popupOpacity) },
	    { ImGuiCol_Separator, MultiplyAlpha(kMenuSepCol, popupOpacity) },
	    { ImGuiCol_WindowShadowStart, MultiplyAlpha(kMenuPopupShadowStartCol, popupOpacity) },
	    { ImGuiCol_WindowShadowEnd, kMenuPopupShadowEndCol },
	};

	StyleVarStack<float> popupMetrics{
	    { ImGuiStyleVar_PopupRounding, kMenuPopupRounding },
	    { ImGuiStyleVar_PopupBorderSize, kMenuPopupBorderSize },
	    { ImGuiStyleVar_WindowShadowSize, kMenuPopupShadowSize },
	    { ImGuiStyleVar_FrameRounding, kMenuItemFrameRounding },
	};
	StyleVarStack<ImVec2> popupPadding{
	    { ImGuiStyleVar_WindowPadding, ImVec2{ kMenuPopupPadding, kMenuPopupPadding } },
	};
	FontSizeStack fontSize(kMenuFontSize);

	ImDrawList *menuBarDrawList = ImGui::GetWindowDrawList();
	const bool menuOpen = ImGui::BeginMenu(text.c_str());
	const ImRect triggerRect{ ImGui::GetItemRectMin(), ImGui::GetItemRectMax() };
	const bool triggerHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
	const bool triggerPressed = triggerHovered && ImGui::IsMouseDown(ImGuiMouseButton_Left);

	if (menuOpen)
	{
		t = std::min(1.0f, t + Time::DeltaTime * 9.0f);
		factor = EaseOutCubic(t);
		const float contentOpacity = 0.30f + 0.70f * factor;
		// ImGui requires styles pushed after BeginMenu() to be popped before EndMenu().
		{
			StyleVarStack<float> contentMetrics{
			    { ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * contentOpacity },
			};
			StyleVarStack<ImVec2> contentSpacing{
			    { ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 2.0f } },
			};
			StyleColorStack<uint32_t> itemColors{
			    { ImGuiCol_Header, IM_COL32(0, 0, 0, 0) },
			    { ImGuiCol_HeaderHovered, hoverColor },
			    { ImGuiCol_HeaderActive, pressedColor },
			    { ImGuiCol_Text, textColor },
			    { ImGuiCol_TextDisabled, kMenuShortcutTextCol },
			};

			for (auto &item : items)
			{
				ImGui::PushID(&item);
				if (item.name.empty())
				{
					ImGui::Dummy(ImVec2{ menuItemWidth, 3.0f });
					ImGui::Separator();
					ImGui::Dummy(ImVec2{ menuItemWidth, 3.0f });
					ImGui::PopID();
					continue;
				}

				if (item.type == MenuItemType::Item)
				{
					const bool selected = ImGui::Selectable(
						"##menu_item",
						false,
						ImGuiSelectableFlags_None,
						ImVec2{ menuItemWidth, kMenuItemHeight });
					const ImRect rowRect{ ImGui::GetItemRectMin(), ImGui::GetItemRectMax() };
					const bool held = ImGui::IsItemActive() ||
						(ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left));
					ImDrawList *drawList = ImGui::GetWindowDrawList();
					if (held)
					{
						drawList->AddRectFilled(
							ImVec2{ rowRect.Min.x, rowRect.Min.y + 7.0f },
							ImVec2{ rowRect.Min.x + 2.0f, rowRect.Max.y - 7.0f },
							accentColor,
							1.0f);
					}

					const ImVec2 labelSize = ImGui::CalcTextSize(item.name.c_str());
					const float textY = rowRect.Min.y + (rowRect.GetHeight() - labelSize.y) * 0.5f;
					const float labelRight = item.tips.empty()
						? rowRect.Max.x - kMenuItemTextPadding
						: rowRect.Max.x - kMenuItemTextPadding - widestShortcut - kMenuShortcutGap;
					drawList->PushClipRect(
						ImVec2{ rowRect.Min.x + kMenuItemTextPadding, rowRect.Min.y },
						ImVec2{ labelRight, rowRect.Max.y },
						true);
					drawList->AddText(
						ImVec2{ rowRect.Min.x + kMenuItemTextPadding, textY },
						ImGui::GetColorU32(ImGuiCol_Text),
						item.name.c_str());
					drawList->PopClipRect();

					if (!item.tips.empty())
					{
						const ImVec2 shortcutSize = ImGui::CalcTextSize(item.tips.c_str());
						drawList->AddText(
							ImVec2{
								rowRect.Max.x - kMenuItemTextPadding - shortcutSize.x,
								rowRect.Min.y + (rowRect.GetHeight() - shortcutSize.y) * 0.5f },
							ImGui::GetColorU32(ImGuiCol_TextDisabled),
							item.tips.c_str());
					}

					if (selected && item.callback)
					{
						item.callback();
					}
				}
				else
				{
					if (ImGui::BeginMenu(item.name.c_str()))
					{
						if (item.callback)
						{
							item.callback();
						}
						ImGui::EndMenu();
					}
				}
				ImGui::PopID();
			}
		}
		ImGui::EndMenu();
	}
	else
	{
		t = 0.0f;
		factor = 0.0f;
	}

	if (menuBarDrawList && (menuOpen || triggerPressed))
	{
		const float targetWidth = std::max(12.0f, triggerRect.GetWidth() - 16.0f);
		const float progress = menuOpen ? std::max(0.35f, factor) : 0.35f;
		const float indicatorWidth = targetWidth * progress;
		const float centerX = triggerRect.GetCenter().x;
		menuBarDrawList->AddRectFilled(
			ImVec2{ centerX - indicatorWidth * 0.5f, triggerRect.Max.y - 2.0f },
			ImVec2{ centerX + indicatorWidth * 0.5f, triggerRect.Max.y },
			accentColor,
			1.0f);
		if (triggerPressed)
		{
			menuBarDrawList->AddRect(
				triggerRect.Min + ImVec2{ 1.0f, 1.0f },
				triggerRect.Max - ImVec2{ 1.0f, 1.0f },
				MultiplyAlpha(accentColor, 0.46f),
				kMenuItemFrameRounding,
				0,
				1.0f);
		}
	}

	return false;
}

WMenu *WMenu::Item(MenuItem &&item)
{
	item.name = Translator::Translate(String{ item.name, StringEncoding::UTF8 });
	item.type = MenuItemType::Item;
	items.emplace_back(std::move(item));
	return this;
}

WMenu *WMenu::Sub(MenuItem &&item)
{
	item.name = Translator::Translate(String{ item.name, StringEncoding::UTF8 });
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
	    { ImGuiCol_Header, OpenColor() },
	    { ImGuiCol_HeaderHovered, HoveredColor() },
	    { ImGuiCol_HeaderActive, PressedColor() },
	    { ImGuiCol_Border, PopupBorderColor() },
	    { ImGuiCol_Separator, kMenuSepCol },
	    { ImGuiCol_WindowShadowStart, kMenuPopupShadowStartCol },
	    { ImGuiCol_WindowShadowEnd, kMenuPopupShadowEndCol },
	};

	StyleVarStack<float> styleVar{
	    { ImGuiStyleVar_PopupRounding, kMenuPopupRounding },
	    { ImGuiStyleVar_PopupBorderSize, kMenuPopupBorderSize },
	    { ImGuiStyleVar_WindowShadowSize, kMenuBarShadowSize },
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

		ImGuiWindow *menuWindow = ImGui::GetCurrentWindow();
		const ImVec2 menuWindowPos = ImGui::GetWindowPos();
		const float frameHeight = ImGui::GetFrameHeight();
		const float contentRight = menuWindowPos.x + ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x;
		const ImVec2 rightRegionStart = ImGui::GetCursorScreenPos();
		const float rightRegionWidth = std::max(0.0f, contentRight - rightRegionStart.x);

#ifdef _WIN32
		Window *captionWindow = nullptr;
		HWND captionHandle = nullptr;
		if (showCaptionButtons)
		{
			captionWindow = Application::GetMainWindow();
			if (captionWindow && captionWindow->GetType() == Window::Type::Win32)
			{
				captionHandle = (HWND)captionWindow->GetPlatformSpecificHandle();
			}
		}
#endif

		if (rightRegionWidth > 1.0f)
		{
			// Reserve the whole right-side rectangle before placing overlay controls inside it.
			ImGui::Dummy(ImVec2{ rightRegionWidth, frameHeight });
			const ImRect reservedRegion{ ImGui::GetItemRectMin(), ImGui::GetItemRectMax() };
			CursorStack reservedCursor{ reservedRegion.Min };
			float rightContentEdge = reservedRegion.Max.x;

#ifdef _WIN32
			constexpr float captionButtonWidth = 46.0f;
			constexpr float captionButtonTotalWidth = captionButtonWidth * 3.0f;
			if (captionHandle)
			{
				rightContentEdge -= captionButtonTotalWidth;
			}
#endif

			if (auto drawRightSide = OnRightSideDraw())
			{
				constexpr float minimumRightSideWidth = 156.0f;
				if (rightContentEdge - reservedRegion.Min.x >= minimumRightSideWidth)
				{
					ImGui::SetCursorScreenPos(reservedRegion.Min);
					float localRightEdge = rightContentEdge - menuWindowPos.x;
					drawRightSide(localRightEdge, frameHeight);
					rightContentEdge = menuWindowPos.x + localRightEdge;
				}
			}

			DrawMenuBarPerformanceStatus(
				menuWindow->DrawList,
				reservedRegion,
				reservedRegion.Min.x + 8.0f,
				rightContentEdge,
				AccentColor());

#ifdef _WIN32
			if (captionHandle)
			{
				const float captionButtonStart = reservedRegion.Max.x - captionButtonTotalWidth;
				if (captionButtonStart >= reservedRegion.Min.x)
				{
					ImGui::SetCursorScreenPos(ImVec2{ captionButtonStart, reservedRegion.Min.y });
					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, (frameHeight - ImGui::GetTextLineHeight()) * 0.5f));
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

					const ImU32 bgNone = IM_COL32(0, 0, 0, 0);
					const ImU32 bgHoverChrome = IM_COL32(255, 255, 255, 15);
					const ImU32 bgPressChrome = IM_COL32(255, 255, 255, 27);
					const ImU32 bgHoverClose = IM_COL32(196, 43, 28, 255);
					const ImU32 bgPressClose = IM_COL32(165, 34, 21, 255);
					const ImU32 chromeGlyph = IM_COL32(204, 207, 214, 255);
					const ImU32 chromeGlyphHi = IM_COL32(255, 255, 255, 255);

					WINDOWPLACEMENT placement = { sizeof(placement) };
					::GetWindowPlacement(captionHandle, &placement);
					const bool zoomed = placement.showCmd == SW_SHOWMAXIMIZED;

					ImGui::PushStyleColor(ImGuiCol_Button, bgNone);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgHoverChrome);
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgPressChrome);
					if (ImGui::InvisibleButton("##immortal_cap_min", ImVec2(captionButtonWidth, frameHeight)))
					{
						captionWindow->CaptionButtonMinimize();
					}
					{
						const ImVec2 rectMin = ImGui::GetItemRectMin();
						const ImVec2 rectMax = ImGui::GetItemRectMax();
						const bool hovered = ImGui::IsItemHovered();
						const bool active = ImGui::IsItemActive();
						if (active)
						{
							menuWindow->DrawList->AddRectFilled(rectMin, rectMax, bgPressChrome);
						}
						else if (hovered)
						{
							menuWindow->DrawList->AddRectFilled(rectMin, rectMax, bgHoverChrome);
						}
						DrawCaptionChromeMin(rectMin, rectMax, hovered ? chromeGlyphHi : chromeGlyph);
					}
					ImGui::PopStyleColor(3);
					ImGui::SameLine(0.0f, 0.0f);

					ImGui::PushStyleColor(ImGuiCol_Button, bgNone);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgHoverChrome);
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgPressChrome);
					if (ImGui::InvisibleButton("##immortal_cap_max", ImVec2(captionButtonWidth, frameHeight)))
					{
						captionWindow->CaptionButtonMaximizeOrRestore();
					}
					{
						const ImVec2 rectMin = ImGui::GetItemRectMin();
						const ImVec2 rectMax = ImGui::GetItemRectMax();
						const bool hovered = ImGui::IsItemHovered();
						const bool active = ImGui::IsItemActive();
						if (active)
						{
							menuWindow->DrawList->AddRectFilled(rectMin, rectMax, bgPressChrome);
						}
						else if (hovered)
						{
							menuWindow->DrawList->AddRectFilled(rectMin, rectMax, bgHoverChrome);
						}
						const ImU32 glyphColor = hovered ? chromeGlyphHi : chromeGlyph;
						if (zoomed)
						{
							DrawCaptionChromeRestore(rectMin, rectMax, glyphColor);
						}
						else
						{
							DrawCaptionChromeMax(rectMin, rectMax, glyphColor);
						}
					}
					ImGui::PopStyleColor(3);
					ImGui::SameLine(0.0f, 0.0f);

					ImGui::PushStyleColor(ImGuiCol_Button, bgNone);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgHoverClose);
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgPressClose);
					if (ImGui::InvisibleButton("##immortal_cap_close", ImVec2(captionButtonWidth, frameHeight)))
					{
						::PostMessageW(captionHandle, WM_CLOSE, 0, 0);
					}
					{
						const ImVec2 rectMin = ImGui::GetItemRectMin();
						const ImVec2 rectMax = ImGui::GetItemRectMax();
						const bool hovered = ImGui::IsItemHovered();
						const bool active = ImGui::IsItemActive();
						if (active)
						{
							menuWindow->DrawList->AddRectFilled(rectMin, rectMax, bgPressClose);
						}
						else if (hovered)
						{
							menuWindow->DrawList->AddRectFilled(rectMin, rectMax, bgHoverClose);
						}
						DrawCaptionChromeClose(rectMin, rectMax, hovered ? chromeGlyphHi : chromeGlyph);
					}
					ImGui::PopStyleColor(3);
					ImGui::PopStyleVar(4);
				}
			}
#endif
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
	item.name = Translator::Translate(String{ item.name, StringEncoding::UTF8 });
	items.emplace_back(std::move(item));
	return this;
}

}
