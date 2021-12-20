#include "impch.h"
#include "DirectWindow.h"
#include <d3d12.h>
#include <dxgi1_4.h>

#include <string>
#include <locale>
#include <codecvt>

#ifndef _UNICODE
#define _UNICODE
#endif
#include <backends/imgui_impl_win32.cpp>

#include "Framework/Utils.h"
#include "Render/Frame.h"
#include "Event/ApplicationEvent.h"
#include "Event/KeyEvent.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Immortal
{

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

    case WM_KEYDOWN:
    {
        KeyPressedEvent e{
            (int)wParam,
            LOWORD(lParam & ~1)
        };
        DirectWindow::EventDispatcher(e);
        return 0;
    }

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
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
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
        GetModuleHandle(nullptr),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        title.c_str(),
        nullptr
    };

    ::RegisterClassEx(&wc);

    handle = ::CreateWindow(
        wc.lpszClassName,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        100,
        100,
        desc.Width,
        desc.Height,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr);
}

DirectWindow::~DirectWindow()
{
    Shutdown();
}

void DirectWindow::Shutdown()
{
    ::DestroyWindow(handle);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
}
}
