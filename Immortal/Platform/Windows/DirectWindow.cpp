#include "DirectWindow.h"

#include <string>
#include <locale>
#include <codecvt>

#include <backends/imgui_impl_win32.cpp>

#include "Framework/Utils.h"
#include "Render/Frame.h"
#include "Event/ApplicationEvent.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Immortal
{

std::unique_ptr<NativeInput> DirectWindow::Input = nullptr;

Window::EventCallbackFunc DirectWindow::EventDispatcher = nullptr;

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
    {
        WindowResizeEvent resizeEvent{
            (UINT)LOWORD(lParam),
            (UINT)HIWORD(lParam)
        };
        DirectWindow::EventDispatcher(resizeEvent);
        return 0;
    }

    case WM_MOVE:
    {
        WindowMoveEvent moveEvent{
            (int)LOWORD(lParam),
            (int)HIWORD(lParam)
        };
        DirectWindow::EventDispatcher(moveEvent);
        return 0;
    }
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    {
        bool down = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
        if (wParam == VK_CONTROL)
        {
            DirectWindow::Input->KeysDown[U32(KeyCode::Control)]  = down;
        }
        else if (wParam == VK_SHIFT)
        {
            DirectWindow::Input->KeysDown[U32(KeyCode::Shift)]  = down;
        }
        else if (wParam == VK_MENU)
        {
            DirectWindow::Input->KeysDown[U32(KeyCode::Alt)] = down;
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
        return 0;
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
        DirectWindow::Input->MouseDown[U32(button)] = true;

        MouseButtonPressedEvent e{
            button
        };
        DirectWindow::EventDispatcher(e);

        return 0;
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
        DirectWindow::Input->MouseDown[U32(button)] = false;
        MouseButtonReleasedEvent e{
            button
        };
        DirectWindow::EventDispatcher(e);

        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        MouseScrolledEvent e{
            0.0f,
            (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA
        };
        DirectWindow::EventDispatcher(e);
        return 0;
    }

    case WM_MOUSEHWHEEL:
    {
        MouseScrolledEvent e{
            (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA,
            0.0f
        };
        DirectWindow::EventDispatcher(e);
        return 0;
    }

    case WM_SETFOCUS:
    case WM_KILLFOCUS:
        DirectWindow::Input->Focus = (msg == WM_SETFOCUS);
        if (!DirectWindow::Input->Focus)
        {
            DirectWindow::Input->Clear();
        }
        return 0;

    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        goto end;

    case WM_DESTROY:
        ::PostQuitMessage(0);
        WindowCloseEvent closeEvent;
        DirectWindow::EventDispatcher(closeEvent);
        return 0;
    }

end:
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

DirectWindow::DirectWindow(const Description &description)
{
    Setup(description);
}

void *DirectWindow::Primitive() const
{
    return handle;
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

void DirectWindow::Setup(const Description &description)
{
    desc = description;

    EventDispatcher = desc.EventCallback;

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring title = Utils::s2ws(desc.Title);

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

    auto width = GetSystemMetrics(SM_CXSCREEN);
    auto height = GetSystemMetrics(SM_CYSCREEN);

    handle = ::CreateWindowExW(
        0,
        wc.lpszClassName,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        (width - desc.Width) / 2,
        (height - desc.Height) / 2,
        desc.Width,
        desc.Height,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr);

    Input.reset(new NativeInput{ this });
}

DirectWindow::~DirectWindow()
{
    Shutdown();
}

void DirectWindow::Shutdown()
{
    ::DestroyWindow(handle);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
}
}
