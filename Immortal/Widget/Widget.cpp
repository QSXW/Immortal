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

	const char *str = text.c_str();
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

	Color(0xff020202);
	BackgroundColor(0xffffffff);
	HoveredColor(0x55d89624);
}

bool WRightClickPopup::Draw()
{
	using namespace ImGui;
	constexpr float kItemWidth  = 240.0f;
	constexpr float kItemHeight = 28.0f;
	constexpr float kPadding    = 5.0f;

	if (IsMouseReleased(ImGuiMouseButton_Right) && IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
	{
		mousePos = ImGui::GetMousePos();
		Open();
	}

	auto itemId = ImGui::GetItemID();
	if (activeItemId != itemId)
	{
		return false;
	}

	StyleColorStack<uint32_t> styleColorStack{
		{ImGuiCol_PopupBg, backgroundColor},
		{ImGuiCol_Separator, color},
	};

	StyleVarStack<float> styleVarStack{
		{ImGuiStyleVar_PopupRounding, 4.0f},
		{ImGuiStyleVar_PopupBorderSize, 0.0f},
		{ImGuiStyleVar_WindowShadowSize, 4.0f}};

	ImGuiContext &g      = *GImGui;
	ImGuiWindow  *window = g.CurrentWindow;
	if (window->SkipItems)
	{
		return false;
	}

	WidgetLock lock{this};
	id = window->GetID(this);
	if (ManualOpen())
	{
		ImGui::OpenPopupEx(id, 1);
		ManualOpen(false);
	}

	bool opened = IsPopupOpen(id, 0);
	if (!opened)
	{
		tween.seek(0);
		ActiveItemId(0);
		return false;
	}

	ImVec2 windowSize = {kItemWidth + kPadding * 2, kItemHeight * std::max(items.size(), size_t(1)) + kPadding * 2};
	if (mousePos.x != 0 && mousePos.y != 0 && mousePos.y + windowSize.y > g.IO.DisplaySize.y)
	{
		ImGui::SetNextWindowPos({mousePos.x, mousePos.y - windowSize.y}, ImGuiCond_Always);
		mousePos = {};
	}

	float factor = tween.step(7 * Time::DeltaTime);
	windowSize.x *= factor;
	windowSize.y *= factor;
	ImGui::SetNextWindowSize(windowSize);

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar;
	if (BeginPopupEx(id, windowFlags))
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			IsMouseClicked(true);
		}
		if ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) /*|| IsMouseClicked(ImGuiMouseButton_Right)*/) && !IsWindowHovered())
		{
			CloseCurrentPopup();
		}

		if (callback)
		{
			callback();
		}

		StyleVarStack<ImVec2> styleVarStack{
			{ImGuiStyleVar_WindowPadding, ImVec2{padding.right, padding.bottom}}};

		EXPORT_WINDOW
		MOVEPOS(0, kPadding);

		ImVec2 itemSize = {kItemWidth, kItemHeight};

		float lineHeight = ImGui::GetTextLineHeight();
		float textAlignment = (kItemHeight - lineHeight) * 0.5;
		for (auto &[s, callback] : items)
		{
			if (s.empty())
			{
				ImGui::Separator();
				continue;
			}
			WidgetLock lock{&s};

			auto window = ImGui::GetCurrentWindow();
			if (window->SkipItems)
				break;

			MOVEPOS(kPadding, 0);
			ImGuiContext &g = *GImGui;
			const ImGuiStyle &style = g.Style;

			const char *label = "###";
			const ImGuiID id = window->GetID(&s);
			const ImVec2 labelSize = ImGui::CalcTextSize(label, NULL, true);

			ImVec2 pos = window->DC.CursorPos;
			ImVec2 size = ImGui::CalcItemSize(itemSize, labelSize.x + style.FramePadding.x * 2.0f, labelSize.y + style.FramePadding.y * 2.0f);

			const ImRect bb(pos, pos + size);
			ImGui::ItemSize(size, style.FramePadding.y);
			if (!ImGui::ItemAdd(bb, id))
				break;

			bool helded = 0;
			bool hovered = 0;
			bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &helded);
			if (pressed)
			{
				callback();
				ImGui::CloseCurrentPopup();
			}
			window->DrawList->AddRectFilled(bb.Min, bb.Max, hovered ? hoveredColor : ImGui::GetColorU32(backgroundColor), 0.0f);
			window->DrawList->AddText({bb.Min.x + 36, bb.Min.y + textAlignment}, ImGui::GetColorU32(color), s.c_str());
		}
	}
	EndPopup();

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

bool WRightClickPopup::IsOpened() const
{
	return ImGui::IsPopupOpen(id, 0);
}

void WRightClickPopup::Open()
{
	ActiveItemId(ImGui::GetItemID());
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
