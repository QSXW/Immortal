#include "DirectWindow.h"

#include <string>
#include <locale>
#include <codecvt>
#include <shellapi.h>

#include "Event/ApplicationEvent.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"

namespace Immortal
{

std::unique_ptr<NativeInput> DirectWindow::Input = nullptr;

Window::EventCallbackFunc DirectWindow::EventDispatcher = nullptr;

static bool IsVirtualKeyDown(int virtualKey)
{
	return (::GetKeyState(virtualKey) & 0x8000) != 0;
}

static KeyCode VirtualKey2KeyCode(WPARAM wParam)
{
    switch (wParam)
    {
        case VK_TAB:
            return KeyCode::Tab;
		case VK_LEFT:
            return KeyCode::Left;
		case VK_RIGHT:
			return KeyCode::Right;
		case VK_UP:
			return KeyCode::Up;
		case VK_DOWN:
			return KeyCode::Down;
		case VK_PRIOR:
			return KeyCode::PageUp;
		case VK_NEXT:
			return KeyCode::PageDown;
		case VK_HOME:
			return KeyCode::Home;
		case VK_END:
			return KeyCode::End;
		case VK_INSERT:
			return KeyCode::Insert;
		case VK_DELETE:
			return KeyCode::Delete;
		case VK_BACK:
			return KeyCode::Backspace;
		case VK_SPACE:
			return KeyCode::Space;
		case VK_RETURN:
			return KeyCode::Enter;
        case VK_ESCAPE:
			return KeyCode::Escape;
		case VK_OEM_7:
			return KeyCode::SingleQuote;
		case VK_OEM_COMMA:
			return KeyCode ::Comma;
		case VK_OEM_MINUS:
			return KeyCode::Hyphen;
		case VK_OEM_PERIOD:
			return KeyCode::FullStop;
		case VK_OEM_2:
			return KeyCode ::Slash;
		case VK_OEM_1:
			return KeyCode ::Semicolon;
		case VK_OEM_PLUS:
			return KeyCode::Equals;
		case VK_OEM_4:
			return KeyCode ::OpeningBracket;
		case VK_OEM_5:
			return KeyCode::BackSlash;
		case VK_OEM_6:
			return KeyCode::ClosingBraket;
		case VK_OEM_3:
			return KeyCode::GraveAccent;
		case VK_CAPITAL:
			return KeyCode::CapsLock;
		case VK_SCROLL:
			return KeyCode::ScrollLock;
		case VK_NUMLOCK:
			return KeyCode::NumLock;
		case VK_SNAPSHOT:
			return KeyCode::PrintScreen;
		case VK_PAUSE:
			return KeyCode::Pause;
		case VK_NUMPAD0:
			return KeyCode::KP0;
		case VK_NUMPAD1:
			return KeyCode::KP1;
		case VK_NUMPAD2:
			return KeyCode::KP2;
		case VK_NUMPAD3:
			return KeyCode::KP3;
		case VK_NUMPAD4:
			return KeyCode::KP4;
		case VK_NUMPAD5:
			return KeyCode::KP5;
		case VK_NUMPAD6:
			return KeyCode::KP6;
		case VK_NUMPAD7:
			return KeyCode::KP7;
		case VK_NUMPAD8:
			return KeyCode::KP8;
		case VK_NUMPAD9:
			return KeyCode::KP9;
		case VK_DECIMAL:
			return KeyCode::KPDecimal;
		case VK_DIVIDE:
			return KeyCode::KPDivide;
		case VK_MULTIPLY:
			return KeyCode::KPMultiply;
		case VK_SUBTRACT:
			return KeyCode::KPSubtract;
		case VK_ADD:
			return KeyCode::KPAdd;
		case VK_RETURN + 256:
			return KeyCode::KPEnter;
		case VK_LSHIFT:
			return KeyCode::LeftShift;
		case VK_LCONTROL:
			return KeyCode::LeftControl;
		case VK_LMENU:
			return KeyCode::LeftAlt;
		case VK_LWIN:
			return KeyCode::LeftSuper;
		case VK_RSHIFT:
			return KeyCode::RightShift;
		case VK_RCONTROL:
			return KeyCode::RightControl;
		case VK_RMENU:
			return KeyCode::RightAlt;
		case VK_RWIN:
			return KeyCode::RightSuper;
		case VK_APPS:
			return KeyCode::Menu;
        case '0': return KeyCode::D0;
        case '1': return KeyCode::D1;
        case '2': return KeyCode::D2;
        case '3': return KeyCode::D3;
        case '4': return KeyCode::D4;
        case '5': return KeyCode::D5;
        case '6': return KeyCode::D6;
        case '7': return KeyCode::D7;
        case '8': return KeyCode::D8;
        case '9': return KeyCode::D9;
        case 'A': return KeyCode::A;
        case 'B': return KeyCode::B;
        case 'C': return KeyCode::C;
        case 'D': return KeyCode::D;
        case 'E': return KeyCode::E;
        case 'F': return KeyCode::F;
        case 'G': return KeyCode::G;
        case 'H': return KeyCode::H;
        case 'I': return KeyCode::I;
        case 'J': return KeyCode::J;
        case 'K': return KeyCode::K;
        case 'L': return KeyCode::L;
        case 'M': return KeyCode::M;
        case 'N': return KeyCode::N;
        case 'O': return KeyCode::O;
        case 'P': return KeyCode::P;
        case 'Q': return KeyCode::Q;
        case 'R': return KeyCode::R;
        case 'S': return KeyCode::S;
        case 'T': return KeyCode::T;
        case 'U': return KeyCode::U;
        case 'V': return KeyCode::V;
        case 'W': return KeyCode::W;
        case 'X': return KeyCode::X;
        case 'Y': return KeyCode::Y;
        case 'Z': return KeyCode::Z;
        case VK_F1: return KeyCode::F1;
        case VK_F2: return KeyCode::F2;
        case VK_F3: return KeyCode::F3;
        case VK_F4: return KeyCode::F4;
        case VK_F5: return KeyCode::F5;
        case VK_F6: return KeyCode::F6;
        case VK_F7: return KeyCode::F7;
        case VK_F8: return KeyCode::F8;
        case VK_F9: return KeyCode::F9;
        case VK_F10: return KeyCode::F10;
        case VK_F11: return KeyCode::F11;
        case VK_F12: return KeyCode::F12;
        case VK_F13: return KeyCode::F13;
        case VK_F14: return KeyCode::F14;
        case VK_F15: return KeyCode::F15;
        case VK_F16: return KeyCode::F16;
        case VK_F17: return KeyCode::F17;
        case VK_F18: return KeyCode::F18;
        case VK_F19: return KeyCode::F19;
        case VK_F20: return KeyCode::F20;
        case VK_F21: return KeyCode::F21;
        case VK_F22: return KeyCode::F22;
        case VK_F23: return KeyCode::F23;
        case VK_F24: return KeyCode::F24;
		case VK_BROWSER_BACK:
			return KeyCode::BrowserBack;
		case VK_BROWSER_FORWARD:
			return KeyCode::BrowserForward;
		case VK_VOLUME_UP:
			return KeyCode::VolumnUp;
		case VK_VOLUME_DOWN:
			return KeyCode::VolumnDown;
		case VK_VOLUME_MUTE:
			return KeyCode::VolumnMute;
		default:
			return KeyCode::Terminator;
    }
}

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_SIZE:
        {
            WindowResizeEvent resizeEvent{
                (UINT)LOWORD(lParam),
                (UINT)HIWORD(lParam)
            };
            DirectWindow::EventDispatcher(resizeEvent);
			break;
        }

        case WM_MOVE:
        {
            WindowMoveEvent moveEvent{
                (int)LOWORD(lParam),
                (int)HIWORD(lParam)
            };
            DirectWindow::EventDispatcher(moveEvent);
			break;
        }
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        {
            bool down = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
			KeyCode keyCode = KeyCode::Terminator;
            if (wParam == VK_CONTROL)
            {
                if (IsVirtualKeyDown(VK_LCONTROL))
                {
					keyCode = KeyCode::LeftControl;
                }
                if (IsVirtualKeyDown(VK_RCONTROL))
				{
					keyCode = KeyCode::RightControl;
                }
            }
            else if (wParam == VK_SHIFT)
            {
				if (IsVirtualKeyDown(VK_LSHIFT))
				{
					keyCode = KeyCode::LeftShift;
				}
				if (IsVirtualKeyDown(VK_RSHIFT))
				{
					keyCode = KeyCode::RightShift;
				}
            }
            else if (wParam == VK_MENU)
            {
				if (IsVirtualKeyDown(VK_LMENU))
				{
					keyCode = KeyCode::LeftAlt;
				}
				if (IsVirtualKeyDown(VK_RMENU))
				{
					keyCode = KeyCode::RightAlt;
				}
            }
            else
			{
				keyCode = VirtualKey2KeyCode(wParam);
            }

			DirectWindow::Input->KeysDown[(int)keyCode] = down;
            if (down && keyCode != KeyCode::Terminator)
            {
                KeyPressedEvent e{
				    (int)keyCode,
                    msg == WM_KEYDOWN ? (uint16_t)0 : LOWORD(lParam)
                };
                DirectWindow::EventDispatcher(e);
            }
			break;
        }

        case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
        {
            MouseCode button;
            if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK)
            {
                button = MouseCode::Left;
            }
            if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK)
            {
                button = MouseCode::Right;
            }
            if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK)
            {
                button = MouseCode::Middle;
            }
            if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK)
            {
                button = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? MouseCode::Button3 : MouseCode::Button4;;
            }
			DirectWindow::Input->MouseDown[uint32_t(button)] = true;

            MouseButtonPressedEvent e{
                button
            };
            DirectWindow::EventDispatcher(e);

            break;
        }

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        {
            MouseCode button;
            if (msg == WM_LBUTTONUP)
            {
                button = MouseCode::Left;
            }
            if (msg == WM_RBUTTONUP)
            {
                button = MouseCode::Right;
            }
            if (msg == WM_MBUTTONUP)
            {
                button = MouseCode::Middle;
            }
            if (msg == WM_XBUTTONUP)
            {
                button = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? MouseCode::Button3 : MouseCode::Button4;
            }
			DirectWindow::Input->MouseDown[uint32_t(button)] = false;
            MouseButtonReleasedEvent e{
                button
            };
            DirectWindow::EventDispatcher(e);

            break;
        }

        case WM_MOUSEWHEEL:
        {
            MouseScrolledEvent e{
                0.0f,
                (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA
            };
            DirectWindow::EventDispatcher(e);
			break;
        }

        case WM_MOUSEHWHEEL:
        {
            MouseScrolledEvent e{
                (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA,
                0.0f
            };
            DirectWindow::EventDispatcher(e);
			break;
        }

        case WM_SETFOCUS:
        case WM_KILLFOCUS:
        {
			DirectWindow::Input->Focus = (msg == WM_SETFOCUS);
			if (!DirectWindow::Input->Focus)
			{
				DirectWindow::Input->Clear();
			}
			break;
        }

        case WM_SYSCOMMAND:
        {
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			{
				return 0;
			}
			return ::DefWindowProc(hWnd, msg, wParam, lParam);
        }

        case WM_DESTROY:
        {
            ::PostQuitMessage(0);
            WindowCloseEvent closeEvent;
            DirectWindow::EventDispatcher(closeEvent);
			break;
        }

        case WM_DROPFILES:
        {
            char path[1024] = {};
            HDROP hDrop = (HDROP)wParam;

            WindowDragDropEvent dragDropEvent;
            for (uint32_t i = 0; ; i++)
            {
				uint32_t length = DragQueryFileA(hDrop, i, path, SL_ARRAY_LENGTH(path));
                if (!length)
                {
					break;
                }
				dragDropEvent.AddFilePath(path);
            }
            DragFinish(hDrop);
			DirectWindow::EventDispatcher(dragDropEvent);
			break;
        }

        default:
			return ::DefWindowProc(hWnd, msg, wParam, lParam); 
    }

    return 0;
}

DirectWindow::DirectWindow(Anonymous handle) :
    Window{},
    wc{},
    handle{ (HWND)handle },
    owned{}
{
	type = Type::Win32;
}

DirectWindow::DirectWindow(const std::string &title, uint32_t width, uint32_t height) :
    Window{},
    wc{},
    handle{},
    owned{ true }
{
	Construct(title, width, height);
}

DirectWindow::~DirectWindow()
{
    if (owned)
    {
		Shutdown();
    }
}

Anonymous DirectWindow::GetBackendHandle() const
{
    return (void *)handle;
}

Anonymous DirectWindow::GetPlatformSpecificHandle() const
{
    return (void *)handle;
}

void DirectWindow::Show()
{
    ::ShowWindow(handle, SW_SHOWDEFAULT);
    ::UpdateWindow(handle);
}

void DirectWindow::ProcessEvents()
{
    MSG msg;
    while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
}

void DirectWindow::SetTitle(const std::string &title)
{
    SetWindowTextA(handle, title.c_str());
}

void DirectWindow::SetIcon(const std::string &filepath)
{

}

void DirectWindow::Construct(const std::string &_title, uint32_t width, uint32_t height)
{
    type = Type::Win32;

    std::wstring title = ToWString(_title);

    wc = {
        sizeof(WNDCLASSEX),
        CS_CLASSDC,
        WndProc,
        0L,
        0L,
        GetModuleHandleW(nullptr),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        title.c_str(),
        nullptr
    };

    ::RegisterClassExW(&wc);

    auto x = GetSystemMetrics(SM_CXSCREEN);
    auto y = GetSystemMetrics(SM_CYSCREEN);
	
    RECT rect = {0, 0, x, y};
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);

    handle = ::CreateWindowExW(
        0,
        wc.lpszClassName,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
	    width  == 0 ? CW_USEDEFAULT : (x - width) / 2,
	    height == 0 ? CW_USEDEFAULT : (y - height) / 2,
	    width  == 0 ? CW_USEDEFAULT : width,
	    height == 0 ? CW_USEDEFAULT : height,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr);

    if (width == 0 || height == 0)
    {
		PostMessageW(handle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    }

    DragAcceptFiles(handle, TRUE);

    Input.reset(new NativeInput{ this });
}

uint32_t DirectWindow::GetWidth() const
{
    RECT rect{};
    GetClientRect(handle, &rect);
    return rect.right - rect.left;
}

uint32_t DirectWindow::GetHeight() const
{
    RECT rect{};
    GetClientRect(handle, &rect);
    return rect.bottom - rect.top;
}

void DirectWindow::SetEventCallback(const EventCallbackFunc &callback)
{
    EventDispatcher = callback;
}

void DirectWindow::Shutdown()
{
    ::DestroyWindow(handle);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

}
