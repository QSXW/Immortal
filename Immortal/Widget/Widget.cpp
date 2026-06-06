#include "Widget.h"
#include "ImGui/GuiLayer.h"

namespace Immortal
{

using namespace ImGui;

namespace Icon
{
WidgetIcon Icons;
const char *Arrows[2];
ImFont *Font;
}

void SetWidgetArrows(ImFont *font, const char *left, const char *right, const char *down, const char *up)
{
	Icon::Font = font;
	Icon::Icons = {
		down,
		left,
		right,
		up
	};
	Icon::Arrows[0] = right;
	Icon::Arrows[1] = down;
}

namespace
{

constexpr float kRightClickMenuRounding = 6.f;
constexpr float kRightClickItemRounding = 6.f;
constexpr float kRightClickPad = 8.f;
constexpr float kRightClickTextIndent = 36.f;
constexpr float kRightClickItemWidth = 304.f;
constexpr float kRightClickItemHeight = 34.f;
constexpr ImU32 kRightClickSepCol = IM_COL32(226, 226, 230, 255);
constexpr ImU32 kRightClickBorderCol = IM_COL32(198, 198, 204, 110);

float RightClickPopupContentHeight(const std::vector<WRightClickPopup::Item> &items)
{
	const float sepBlockH = kRightClickPad + ImGui::GetStyle().SeparatorSize + kRightClickPad;
	float contentH = 0.f;
	for (const auto &entry : items)
	{
		contentH += entry.text.empty() ? sepBlockH : kRightClickItemHeight;
	}
	return contentH;
}

ImVec2 RightClickPopupWindowSize(const std::vector<WRightClickPopup::Item> &items)
{
	return { kRightClickItemWidth + kRightClickPad * 2.f, RightClickPopupContentHeight(items) + kRightClickPad * 2.f };
}

ImVec2 ClampRightClickSubmenuPos(const ImRect &rowBb, const ImVec2 &size)
{
	ImVec2 pos{ rowBb.Max.x + 2.f, rowBb.Min.y - kRightClickPad };
	const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	if (pos.x + size.x > displaySize.x)
	{
		pos.x = ImMax(0.f, rowBb.Min.x - size.x - 2.f);
	}
	if (pos.y + size.y > displaySize.y)
	{
		pos.y = ImMax(0.f, displaySize.y - size.y);
	}
	return pos;
}

WRightClickPopup::Item TranslateRightClickPopupItem(const WRightClickPopup::Item &item)
{
	WRightClickPopup::Item translated;
	translated.text = item.text.empty() ? item.text : Translator::Translate(item.text);
	translated.callback = item.callback;
	translated.children.reserve(item.children.size());
	for (const auto &child : item.children)
	{
		translated.children.emplace_back(TranslateRightClickPopupItem(child));
	}
	return translated;
}

bool DrawRightClickPopupItems(const std::vector<WRightClickPopup::Item> &items, ImU32 textF, ImU32 hoverF, ImGuiWindowFlags windowFlags)
{
	bool activated = false;
	const float lineHeight = ImGui::GetTextLineHeight();
	for (const auto &item : items)
	{
		ImGuiWindow *window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
		{
			break;
		}

		if (item.text.empty())
		{
			ImGui::Dummy(ImVec2(kRightClickItemWidth, kRightClickPad));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(kRightClickItemWidth, kRightClickPad));
			continue;
		}

		ImGui::PushID(&item);
		const ImGuiID rowId = window->GetID("##row");
		const bool hasChildren = !item.children.empty();
		const ImGuiID submenuId = hasChildren ? window->GetID("##submenu") : 0;
		const bool submenuOpen = hasChildren && ImGui::IsPopupOpen(submenuId, 0);
		const ImVec2 pos = window->DC.CursorPos;
		const ImVec2 size = ImVec2(kRightClickItemWidth, kRightClickItemHeight);
		const ImRect bb(pos, pos + size);

		ImGui::ItemSize(size, 0.f);
		if (!ImGui::ItemAdd(bb, rowId))
		{
			ImGui::PopID();
			continue;
		}

		bool held = false;
		bool buttonHovered = false;
		const bool buttonPressed = ImGui::ButtonBehavior(bb, rowId, &buttonHovered, &held);
		const bool hovered = buttonHovered ||
			(ImGui::IsMouseHoveringRect(bb.Min, bb.Max, false) &&
			 ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup));
		const bool pressed = buttonPressed || (hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left));
		const bool rowActive = hovered || submenuOpen;
		window->DrawList->AddRectFilled(bb.Min, bb.Max, rowActive ? hoverF : IM_COL32(0, 0, 0, 0), kRightClickItemRounding);

		const float textY = bb.Min.y + ImMax(0.f, (kRightClickItemHeight - lineHeight) * 0.5f);
		window->DrawList->PushClipRect(bb.Min, bb.Max, true);
		window->DrawList->AddText(ImVec2(bb.Min.x + kRightClickTextIndent, textY), textF, item.text.c_str());
		if (hasChildren)
		{
			const char *arrow = ">";
			const ImVec2 arrowSize = ImGui::CalcTextSize(arrow);
			window->DrawList->AddText(ImVec2(bb.Max.x - kRightClickPad - arrowSize.x, textY), textF, arrow);
		}
		window->DrawList->PopClipRect();

		if (hasChildren)
		{
			if (hovered || pressed)
			{
				ImGui::OpenPopupEx(submenuId);
			}
			if (hovered || submenuOpen)
			{
				const ImVec2 submenuSize = RightClickPopupWindowSize(item.children);
				ImGui::SetNextWindowPos(ClampRightClickSubmenuPos(bb, submenuSize), ImGuiCond_Always);
				ImGui::SetNextWindowSize(submenuSize);
			}
			if (ImGui::BeginPopupEx(submenuId, windowFlags))
			{
				if (DrawRightClickPopupItems(item.children, textF, hoverF, windowFlags))
				{
					activated = true;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
		else
		{
			if (hovered && ImGui::GetCurrentContext()->OpenPopupStack.Size > ImGui::GetCurrentContext()->BeginPopupStack.Size)
			{
				ImGui::ClosePopupToLevel(ImGui::GetCurrentContext()->BeginPopupStack.Size, true);
			}
			if (pressed)
			{
				if (item.callback)
				{
					item.callback();
				}
				activated = true;
			}
		}

		ImGui::PopID();
	}
	return activated;
}

}

std::unordered_map<std::string, Widget *> Widget::Identify2WidgetTracker;
std::unordered_map<Widget *, std::string> Widget::Widget2IdentifyTracker;

WWindow::WWindow()
{
    GuiLayer::Inject2Dockspace(this);
}

WDockerSpace::WDockerSpace() :
    Widget{nullptr}
{

}

bool WDockerSpace::Draw()
{
	static bool isOpen = true;
	static bool optionalPadding = false;
	static bool optionalFullScreen = true;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_NoWindowMenuButton;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

	if (optionalFullScreen)
	{
		const ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		// ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,      { 0.0f, 0.0f });
		ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 4.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_TabBarBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}
	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
	{
		window_flags |= ImGuiWindowFlags_NoBackground;
	}
	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	if (!optionalPadding)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	}
	/* Dock place */
	FontSizeStack fontSize{ImGui::GetFont(), 18.f};
	if (ImGui::Begin("MyDockSpace", &isOpen, window_flags))
	{
		ImGuiIO &io = ImGui::GetIO();
		ImGuiStyle &style = ImGui::GetStyle();

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		__Trampoline();

		if (!optionalPadding)
		{
			ImGui::PopStyleVar();
		}
		if (optionalFullScreen)
		{
			ImGui::PopStyleVar(4);
		}
	}
	ImGui::End();

	return false;
}

WFrame::WFrame(Widget *parent) :
    Widget{parent},
    state{}
{

}

String WFrame::BeginTitle() const
{
	if (windowId.empty())
	{
		return text;
	}

	std::string title{ text.c_str(), text.size() };
	title += "###";
	title += windowId.c_str();
	return String{ title, StringEncoding::UTF8 };
}

bool WFrame::Draw()
{
	if (!visible)
	{
		return false;
	}

	//StyleColorStack<uint32_t> styleColor{
	//	{ImGuiCol_TabActive, color},
	//	{ImGuiCol_WindowBg, color}
	//};

	StyleVarStack<ImVec2> styleVar{
		{ImGuiStyleVar_WindowPadding, {padding.right, padding.bottom}},
		{ImGuiStyleVar_ItemSpacing, {padding.right, padding.bottom}}
	};

	const String beginTitle = BeginTitle();
	const char *str = beginTitle.c_str();
	ImVec2 windowPos;
	ImVec2 windowSize;
	float titleBarHeight = 0;
	bool isFocused = false;
	float borderOffset = 1.0f;

	using namespace ImGui;
	if (Begin(str, nullptr, Flags() | ImGuiWindowFlags_NoCollapse))
	{
		windowPos      = GetWindowPos();
		windowSize     = GetWindowSize();
		titleBarHeight = GetCurrentWindow()->TitleBarHeight;

		state.isFocused = IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
		state.isHovered = IsWindowHovered(ImGuiFocusedFlags_ChildWindows);
		auto [x, y] = ImGui::GetContentRegionAvail();
		RenderWidth(x - borderOffset);
		RenderHeight(y - borderOffset);

		bool opened = BeginChild("##ChildFrame");
		if (opened)
		{
			ImGuiWindow *window = ImGui::GetCurrentWindow();
			position = window->DC.CursorStartPos;
			scroll = window->Scroll;
			__RelativeTrampoline();
			//for (auto &c : children)
			//{
			//	c->Draw();
			//}
		}
		EndChild();
	}
	End();

	return false;
}

WCollapsingHeader::WCollapsingHeader(bool defaultOpen) :
    tween{tweeny::from(0.0f).to(1.0f).during(10000).via(tweeny::easing::bounceOut)}
{
	if (defaultOpen)
	{
		Expanded(true);
		flags |= ImGuiTreeNodeFlags_DefaultOpen;
		targetSizeY = BodyHeigth();
	}
}

WRightClickPopup::WRightClickPopup(Widget *parent) :
    Widget{parent},
    callback{},
    id{}
{
	using tweeny::easing;
	tween = tweeny::from(0.0f).to(1.0f).during(500).via(tweeny::easing::quadraticInOut);

	Color(IM_COL32(42, 42, 44, 255));
	BackgroundColor(IM_COL32(252, 252, 254, 247));
	HoveredColor(IM_COL32(218, 218, 221, 255));
}

bool WRightClickPopup::Draw()
{
	using namespace ImGui;
	if (IsMouseReleased(ImGuiMouseButton_Right) && IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
	{
		mousePos = ImGui::GetMousePos();
		Open();
	}
	return DrawForOwner(ImGui::GetItemID());
}

bool WRightClickPopup::DrawForOwner(ImGuiID ownerForGate)
{
	using namespace ImGui;
	const ImGuiID compareId = ownerForGate ? ownerForGate : ImGui::GetItemID();
	if (activeItemId != compareId && !ManualOpen())
	{
		return false;
	}

	ImGuiContext &g = *GImGui;
	ImGuiWindow *window = g.CurrentWindow;
	if (window->SkipItems)
	{
		return false;
	}

	WidgetLock lock{ this };
	id = window->GetID(this);
	if (ManualOpen())
	{
		ImGui::OpenPopupEx(id, ImGuiPopupFlags_MouseButtonRight);
		ManualOpen(false);
	}

	bool opened = IsPopupOpen(id, 0);
	if (!opened)
	{
		tween.seek(0);
		ActiveItemId(0);
		return false;
	}

	const float fade = ImClamp(tween.step(7.f * Time::DeltaTime), 0.f, 1.f);
	auto mulAlpha = [](ImU32 col, float f) -> ImU32 {
		ImVec4 v = ImGui::ColorConvertU32ToFloat4(col);
		v.w *= f;
		return ImGui::ColorConvertFloat4ToU32(v);
	};
	const ImU32 popupBgF = mulAlpha(backgroundColor, fade);
	const ImU32 borderF = mulAlpha(kRightClickBorderCol, fade);
	const ImU32 sepF = mulAlpha(kRightClickSepCol, fade);
	const ImU32 textF = mulAlpha(color, fade);
	const ImU32 hoverF = mulAlpha(hoveredColor, fade);

	StyleColorStack<uint32_t> styleColorStack{
	    { ImGuiCol_PopupBg, popupBgF },
	    { ImGuiCol_Border, borderF },
	    { ImGuiCol_Separator, sepF },
	};

	StyleVarStack<float> styleVarStack{
	    { ImGuiStyleVar_PopupRounding, kRightClickMenuRounding },
	    { ImGuiStyleVar_PopupBorderSize, 1.0f },
	    { ImGuiStyleVar_WindowShadowSize, 2.0f },
	};

	ImVec2 windowSize = RightClickPopupWindowSize(items);
	if (mousePos.x != 0 && mousePos.y != 0 && mousePos.y + windowSize.y > g.IO.DisplaySize.y)
	{
		ImGui::SetNextWindowPos({ mousePos.x, mousePos.y - windowSize.y }, ImGuiCond_Always);
		mousePos = {};
	}

	ImGui::SetNextWindowSize(windowSize);

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar;
	/* Push before BeginPopupEx: Begin() copies style.WindowPadding into window->WindowPadding. */
	{
		StyleVarStack<ImVec2> menuPaddingStack{
		    { ImGuiStyleVar_WindowPadding, ImVec2{ kRightClickPad, kRightClickPad } },
		    { ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f } },
		};
		if (BeginPopupEx(id, windowFlags))
		{
			if ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && !IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
			{
				CloseCurrentPopup();
			}

			if (callback)
			{
				callback();
			}

			if (DrawRightClickPopupItems(items, textF, hoverF, windowFlags))
			{
				CloseCurrentPopup();
			}
			EndPopup();
		}
	}

	return true;
}

WRightClickPopup *WRightClickPopup::Items(std::initializer_list<std::pair<const String &, std::function<void()>>> &&list)
{
	for (auto &[s, callbcak] : list)
	{
		items.emplace_back(Translator::Translate(s), callbcak);
	}

	return this;
}

WRightClickPopup *WRightClickPopup::MenuItems(std::initializer_list<Item> &&list)
{
	for (const auto &item : list)
	{
		items.emplace_back(TranslateRightClickPopupItem(item));
	}

	return this;
}

bool WRightClickPopup::IsOpened() const
{
	return ImGui::IsPopupOpen(id, 0);
}

void WRightClickPopup::Open()
{
	ActiveItemId(ImGui::GetItemID());
	ManualOpen(true);
}

void WRightClickPopup::OpenForItem(ImGuiID itemId)
{
	ActiveItemId(itemId);
	mousePos = ImGui::GetMousePos();
	ManualOpen(true);
}

bool InputText(int id, const char *hint, char *buf, size_t size, float width, float height, uint32_t borderColor, uint32_t activeBorderColor, float borderSize, float rounding, ImGuiInputTextCallback callback, void *userData)
{
	WidgetLock lock{id};
	//width  -= borderSize * 2;
    //height -= borderSize * 2;

	StyleVarStack<ImVec2> styleVar{
	    {ImGuiStyleVar_FramePadding, {8.0f, (height - ImGui::GetTextLineHeight()) * 0.618f}}
	};

	EXPORT_WINDOW
	bool ret = ImGui::InputTextEx("##", hint, buf, size, {width, height}, /*ImGuiInputTextFlags_EnterReturnsTrue |*/ ImGuiInputTextFlags_CallbackResize, callback, userData);

	ImRect bb{ImGui::GetItemRectMin(), ImGui::GetItemRectMax()};
	window->DrawList->AddRect(bb.Min, bb.Max, (ImGui::IsItemActive() || ImGui::IsItemHovered()) ? activeBorderColor : borderColor, rounding, ImDrawFlags_None, borderSize);

	return ret;
}

static int InputTextCallback(ImGuiInputTextCallbackData *data)
{
	String *text = (String *) data->UserData;
	if (data->BufTextLen >= data->BufSize - 1)
	{
		text->reserve(data->BufSize + 1);
		text->resize(data->BufTextLen);
		data->BufSize = text->capacity();
		data->Buf     = text->data();
	}
	else
	{
		text->resize(data->BufTextLen);
	}

	return 0;
}

WInputText::WInputText()
{
	text.reserve(128);
}

bool WInputText::Draw(const ImVec2 &size)
{
	bool ret = InputText(ImGui::GetID(this), Hint().c_str(), text.data(), text.size() + 1, size.x <= 0 ? ImGui::CalcItemWidth() : size.x, size.y <= 0 ? ImGui::GetFrameHeight() : size.y, OutlineColor(), ActiveOutlineColor(), OutlineBorderSize(), Rounding(), InputTextCallback, &text);
	if (text[0] == '\0')
	{
		text.resize(0);
	}
	return ret;
}

}
