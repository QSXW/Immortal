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
            if (wParam == VK_CONTROL)
            {
				DirectWindow::Input->KeysDown[uint32_t(KeyCode::Control)] = down;
            }
            else if (wParam == VK_SHIFT)
            {
				DirectWindow::Input->KeysDown[uint32_t(KeyCode::Shift)] = down;
            }
            else if (wParam == VK_MENU)
            {
				DirectWindow::Input->KeysDown[uint32_t(KeyCode::Alt)] = down;
            }
            DirectWindow::Input->KeysDown[wParam] = down;

            if (down)
            {
                KeyPressedEvent e{
                    (int)wParam,
                    msg == WM_KEYDOWN ? (uint16_t)0 : LOWORD(lParam)
                };
                DirectWindow::EventDispatcher(e);
            }
            else
            {
                KeyReleasedEvent e{
                    (int)wParam
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

    handle = ::CreateWindowExW(
        0,
        wc.lpszClassName,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        (x - width) / 2,
        (y - height) / 2,
	    width,
	    height,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr);

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
