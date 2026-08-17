#include "DirectWindow.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include <shellapi.h>
#include <dbt.h>
#include <windowsx.h>
#include <wincodec.h>

#include "Framework/Utils.h"
#include "Event/ApplicationEvent.h"

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"

namespace Immortal
{

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

static constexpr double kRgnPi = 3.14159265358979323846;

static HRGN CreateRoundedWindowRgnPoly(int w, int h, int radiusLogical, int segmentsPerCorner)
{
	if (w <= 1 || h <= 1)
	{
		return nullptr;
	}
	int r = radiusLogical;
	r = (std::min)(r, (std::max)(1, w / 2 - 1));
	r = (std::min)(r, (std::max)(1, h / 2 - 1));
	r = (std::max)(r, 1);

	const int n = (std::max)(6, segmentsPerCorner);
	std::vector<POINT> pt;
	pt.reserve((size_t)(8 + n * 4));

	const double hpi = kRgnPi * 0.5;

	pt.push_back({ r, 0 });
	pt.push_back({ w - r, 0 });

	for (int i = 1; i <= n; ++i)
	{
		const double t = hpi * (double)i / (double)n;
		pt.push_back({
		    (LONG)std::lround((double)(w - r) + (double)r * std::sin(t)),
		    (LONG)std::lround((double)r - (double)r * std::cos(t)),
		});
	}

	pt.push_back({ w, h - r });

	for (int i = 1; i <= n; ++i)
	{
		const double t = hpi * (double)i / (double)n;
		pt.push_back({
		    (LONG)std::lround((double)(w - r) + (double)r * std::cos(t)),
		    (LONG)std::lround((double)(h - r) + (double)r * std::sin(t)),
		});
	}

	pt.push_back({ r, h });

	for (int i = 1; i <= n; ++i)
	{
		const double t = hpi * (double)i / (double)n;
		pt.push_back({
		    (LONG)std::lround((double)r - (double)r * std::sin(t)),
		    (LONG)std::lround((double)(h - r) + (double)r * std::cos(t)),
		});
	}

	pt.push_back({ 0, r });

	for (int i = 1; i <= n; ++i)
	{
		const double t = hpi * (double)i / (double)n;
		pt.push_back({
		    (LONG)std::lround((double)r - (double)r * std::sin(t)),
		    (LONG)std::lround((double)r - (double)r * std::cos(t)),
		});
	}

	return ::CreatePolygonRgn(pt.data(), (int)pt.size(), WINDING);
}

static HRESULT SetDwmCornerPreference(HWND hwnd, DWORD pref)
{
	if (!hwnd)
	{
		return E_INVALIDARG;
	}
	typedef HRESULT(WINAPI *PFN_DwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);
	static HMODULE dwmDll = ::LoadLibraryW(L"dwmapi.dll");
	if (!dwmDll)
	{
		return E_FAIL;
	}
	auto fn = (PFN_DwmSetWindowAttribute)::GetProcAddress(dwmDll, "DwmSetWindowAttribute");
	if (!fn)
	{
		return E_FAIL;
	}

	return fn(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &pref, sizeof(pref));
}

static void ApplyImmortalWindowRounding(HWND hwnd, bool borderless, bool exclusiveFullscreen)
{
	if (!hwnd)
	{
		return;
	}

	if (exclusiveFullscreen)
	{
		(void)SetDwmCornerPreference(hwnd, 1u);
		::SetWindowRgn(hwnd, nullptr, TRUE);
		return;
	}

	const HRESULT hrRound = SetDwmCornerPreference(hwnd, 2u);

	if (!borderless)
	{
		return;
	}

	if (::IsZoomed(hwnd))
	{
		::SetWindowRgn(hwnd, nullptr, TRUE);
		return;
	}

	if (SUCCEEDED(hrRound))
	{
		::SetWindowRgn(hwnd, nullptr, TRUE);
		return;
	}

	RECT wr{};
	::GetWindowRect(hwnd, &wr);
	const int w = (int)(wr.right - wr.left);
	const int h = (int)(wr.bottom - wr.top);
	if (w <= 1 || h <= 1)
	{
		return;
	}
	const UINT dpi = ::GetDpiForWindow(hwnd);
	/** 小圆角（约 4px@96DPI）；角上用多段折线减轻 CreateRoundRectRgn 的锯齿感 */
	const int radius = (std::max)(2, ::MulDiv(4, (int)dpi, USER_DEFAULT_SCREEN_DPI));
	const int segs = (std::min)(24, (std::max)(10, radius + 8));
	HRGN rgn = CreateRoundedWindowRgnPoly(w, h, radius, segs);
	if (!rgn)
	{
		const int ell = (std::max)(radius * 2, 2);
		rgn = ::CreateRoundRectRgn(0, 0, w, h, ell, ell);
	}
	if (rgn)
	{
		::SetWindowRgn(hwnd, rgn, TRUE);
	}
}

std::unique_ptr<NativeInput> DirectWindow::Input = nullptr;

void EmptyEventCallback(Event &event)
{

}

Window::EventCallbackFunc DirectWindow::EventDispatcher = &EmptyEventCallback;

namespace
{

bool g_BorderlessCaptionPreferClient = false;

/** WM_NCHITTEST for WS_POPUP borderless: edge resize + menu strip behaves like caption (drag / dbl-click maximize). */
LRESULT HitTestBorderlessFrame(HWND hWnd, LPARAM lParam)
{
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	::ScreenToClient(hWnd, &pt);
	RECT rc{};
	::GetClientRect(hWnd, &rc);
	const int cw = rc.right - rc.left;
	const int ch = rc.bottom - rc.top;

	UINT dpi = ::GetDpiForWindow(hWnd);
	const int border = (std::max)(MulDiv(8, (int)dpi, USER_DEFAULT_SCREEN_DPI),
		MulDiv(GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER), (int)dpi, USER_DEFAULT_SCREEN_DPI));

	if (!::IsZoomed(hWnd))
	{
		const bool left = pt.x < border;
		const bool right = pt.x >= cw - border;
		const bool top = pt.y < border;
		const bool bottom = pt.y >= ch - border;
		if (top && left)
		{
			return HTTOPLEFT;
		}
		if (top && right)
		{
			return HTTOPRIGHT;
		}
		if (bottom && left)
		{
			return HTBOTTOMLEFT;
		}
		if (bottom && right)
		{
			return HTBOTTOMRIGHT;
		}
		if (top)
		{
			return HTTOP;
		}
		if (bottom)
		{
			return HTBOTTOM;
		}
		if (left)
		{
			return HTLEFT;
		}
		if (right)
		{
			return HTRIGHT;
		}
	}

	const int menuStripH = MulDiv(28, (int)dpi, USER_DEFAULT_SCREEN_DPI);
	const int captionBottom = border + menuStripH;
	if (pt.x >= border && pt.x < cw - border && pt.y >= border && pt.y < captionBottom)
	{
		if (g_BorderlessCaptionPreferClient)
		{
			return HTCLIENT;
		}
		return HTCAPTION;
	}
	return HTCLIENT;
}

static RECT MaximizeAreaForBorderless(const MONITORINFO &mi)
{
	RECT area = mi.rcWork;
	if (mi.rcWork.bottom < mi.rcMonitor.bottom && mi.rcWork.top <= mi.rcMonitor.top + 1)
	{
		area.left = mi.rcMonitor.left;
		area.top = mi.rcMonitor.top;
		area.right = mi.rcMonitor.right;
	}
	return area;
}

static bool LoadPngResizeToSquare(const std::wstring &path, UINT targetSize, std::vector<uint8_t> &bgra, UINT &outW, UINT &outH)
{
	IWICImagingFactory *factory = nullptr;
	if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void **)&factory)))
	{
		return false;
	}

	IWICBitmapDecoder *decoder = nullptr;
	if (FAILED(factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)))
	{
		factory->Release();
		return false;
	}

	IWICBitmapFrameDecode *frame = nullptr;
	if (FAILED(decoder->GetFrame(0, &frame)))
	{
		decoder->Release();
		factory->Release();
		return false;
	}

	IWICFormatConverter *converter = nullptr;
	if (FAILED(factory->CreateFormatConverter(&converter)))
	{
		frame->Release();
		decoder->Release();
		factory->Release();
		return false;
	}

	if (FAILED(converter->Initialize(frame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeMedianCut)))
	{
		converter->Release();
		frame->Release();
		decoder->Release();
		factory->Release();
		return false;
	}
	frame->Release();

	IWICBitmapScaler *scaler = nullptr;
	if (FAILED(factory->CreateBitmapScaler(&scaler)))
	{
		converter->Release();
		decoder->Release();
		factory->Release();
		return false;
	}

	if (FAILED(scaler->Initialize(converter, targetSize, targetSize, WICBitmapInterpolationModeFant)))
	{
		scaler->Release();
		converter->Release();
		decoder->Release();
		factory->Release();
		return false;
	}
	converter->Release();
	decoder->Release();

	UINT w = 0;
	UINT h = 0;
	scaler->GetSize(&w, &h);
	outW = w;
	outH = h;
	const UINT stride = w * 4;
	const UINT bufSize = stride * h;
	bgra.resize(bufSize);
	const HRESULT hr = scaler->CopyPixels(nullptr, stride, bufSize, bgra.data());
	scaler->Release();
	factory->Release();
	return SUCCEEDED(hr);
}

static HICON CreateIconFrom32bppPBGRA(int w, int h, const uint8_t *bgraTopDown)
{
	const int maskRowBytes = ((w + 31) / 32) * 4;
	std::vector<uint8_t> maskBits((size_t)maskRowBytes * (size_t)h, 0);
	std::vector<uint32_t> colorPixelsBottomUp((size_t)w * (size_t)h);

	for (int y = 0; y < h; ++y)
	{
		const uint8_t *srcRow = bgraTopDown + (size_t)y * (size_t)w * 4;
		uint32_t *dstRow = &colorPixelsBottomUp[(size_t)(h - 1 - y) * (size_t)w];
		uint8_t *maskRow = &maskBits[(size_t)(h - 1 - y) * (size_t)maskRowBytes];
		for (int x = 0; x < w; ++x)
		{
			const uint8_t *p = srcRow + (size_t)x * 4;
			const uint32_t bgra = (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
			dstRow[x] = bgra;
			const bool opaque = p[3] > 128;
			const int bitIndex = x;
			const int byteIndex = bitIndex >> 3;
			const int bitOffset = 7 - (bitIndex & 7);
			if (opaque)
			{
				maskRow[byteIndex] = (uint8_t)(maskRow[byteIndex] | (1 << bitOffset));
			}
		}
	}

	BITMAPINFOHEADER bih = {};
	bih.biSize = sizeof(bih);
	bih.biWidth = w;
	bih.biHeight = h;
	bih.biPlanes = 1;
	bih.biBitCount = 32;
	bih.biCompression = BI_RGB;

	void *dibBits = nullptr;
	HDC hdc = ::GetDC(nullptr);
	HBITMAP hbmColor = ::CreateDIBSection(hdc, (BITMAPINFO *)&bih, DIB_RGB_COLORS, &dibBits, nullptr, 0);
	if (!hbmColor)
	{
		::ReleaseDC(nullptr, hdc);
		return nullptr;
	}
	memcpy(dibBits, colorPixelsBottomUp.data(), (size_t)w * (size_t)h * 4u);

	HBITMAP hbmMask = ::CreateBitmap(w, h, 1, 1, maskBits.data());
	if (!hbmMask)
	{
		::DeleteObject(hbmColor);
		::ReleaseDC(nullptr, hdc);
		return nullptr;
	}
	::ReleaseDC(nullptr, hdc);

	ICONINFO ii = {};
	ii.fIcon = TRUE;
	ii.xHotspot = (DWORD)(w / 2);
	ii.yHotspot = (DWORD)(h / 2);
	ii.hbmMask = hbmMask;
	ii.hbmColor = hbmColor;

	HICON icon = ::CreateIconIndirect(&ii);
	::DeleteObject(hbmMask);
	::DeleteObject(hbmColor);
	return icon;
}

}

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
        case WM_ERASEBKGND:
            return 1;

        case WM_NCHITTEST:
        {
            DirectWindow *dw = (DirectWindow *)::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
            if (dw && dw->IsBorderless())
            {
                return HitTestBorderlessFrame(hWnd, lParam);
            }
            return ::DefWindowProcW(hWnd, msg, wParam, lParam);
        }

        case WM_GETMINMAXINFO:
        {
            DirectWindow *dw = (DirectWindow *)::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
            if (dw && dw->IsBorderless())
            {
                MINMAXINFO *mmi = (MINMAXINFO *)lParam;
                HMONITOR mon = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
                MONITORINFO mi{};
                mi.cbSize = sizeof(mi);
                if (::GetMonitorInfoW(mon, &mi))
                {
                    RECT area = MaximizeAreaForBorderless(mi);
                    mmi->ptMaxPosition.x = area.left;
                    mmi->ptMaxPosition.y = area.top;
                    mmi->ptMaxSize.x = area.right - area.left;
                    mmi->ptMaxSize.y = area.bottom - area.top;
                }
                return 0;
            }
            break;
        }

        case WM_NCCALCSIZE:
        {
            DirectWindow *dw = (DirectWindow *)::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
            if (dw && dw->IsBorderless() && wParam == TRUE && ::IsZoomed(hWnd))
            {
                NCCALCSIZE_PARAMS *p = (NCCALCSIZE_PARAMS *)lParam;
                p->rgrc[2] = p->rgrc[0];
                return 0;
            }
            break;
        }

        case WM_SIZE:
        {
            DirectWindow *dw = (DirectWindow *)::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
            if (dw)
            {
                ApplyImmortalWindowRounding(hWnd, dw->IsBorderless(), dw->IsFullscreen());
            }
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
        case WM_DEVICECHANGE:
        {
            DeviceChangedEvent deviceChangedEvent{ static_cast<uint64_t>(wParam) };
            DirectWindow::EventDispatcher(deviceChangedEvent);
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
				if (IsVirtualKeyDown(VK_LCONTROL) == down)
                {
					keyCode = KeyCode::LeftControl;
                }
				if (IsVirtualKeyDown(VK_RCONTROL) == down)
				{
					keyCode = KeyCode::RightControl;
                }
            }
            else if (wParam == VK_SHIFT)
            {
				if (IsVirtualKeyDown(VK_LSHIFT) == down)
				{
					keyCode = KeyCode::LeftShift;
				}
				if (IsVirtualKeyDown(VK_RSHIFT) == down)
				{
					keyCode = KeyCode::RightShift;
				}
            }
            else if (wParam == VK_MENU)
            {
				if (IsVirtualKeyDown(VK_LMENU) == down)
				{
					keyCode = KeyCode::LeftAlt;
				}
				if (IsVirtualKeyDown(VK_RMENU) == down)
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

        case WM_MOUSEMOVE:
        {
            const float x = (float)(short)LOWORD(lParam);
            const float y = (float)(short)HIWORD(lParam);
            MouseMoveEvent e{ x, y };
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
            if (msg == WM_SETFOCUS)
            {
                WindowFocusEvent e;
                DirectWindow::EventDispatcher(e);
            }
            else
            {
                WindowLostFocusEvent e;
                DirectWindow::EventDispatcher(e);
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
            HDROP hDrop = (HDROP)wParam;

            WindowDragDropEvent dragDropEvent;
            const UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
            for (UINT i = 0; i < fileCount; i++)
            {
				const UINT length = DragQueryFileW(hDrop, i, nullptr, 0);
                if (length == 0)
                {
					continue;
                }

                std::wstring path(length + 1, L'\0');
                const UINT copied = DragQueryFileW(hDrop, i, path.data(), static_cast<UINT>(path.size()));
                if (copied == 0)
                {
                    continue;
                }
                path.resize(copied);
				String droppedPath{ path };
                LOG::DEBUG("Window received dropped file: {}", droppedPath);
				dragDropEvent.AddFilePath(std::move(droppedPath));
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
    owned{},
    fullscreen{ false },
    zoomedBeforeExclusiveFullscreen{ false },
    windowedStyle{ 0 },
    windowedExStyle{ 0 },
    windowedPlacement{},
    borderless{ false }
{
	type = Type::Win32;
    CacheWindowedState();
	ApplyImmortalWindowRounding(this->handle, false, false);
}

DirectWindow::DirectWindow(const std::string &title, uint32_t width, uint32_t height, bool borderlessWindow) :
    Window{},
    wc{},
    handle{},
    owned{ true },
    fullscreen{ false },
    zoomedBeforeExclusiveFullscreen{ false },
    windowedStyle{ 0 },
    windowedExStyle{ 0 },
    windowedPlacement{},
    borderless{ borderlessWindow }
{
	Construct(title, width, height);
}

void DirectWindow::SetBorderlessCaptionPreferClient(bool preferClientArea)
{
	g_BorderlessCaptionPreferClient = preferClientArea;
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
	ApplyImmortalWindowRounding(handle, borderless, IsFullscreen());
}

void DirectWindow::CaptionButtonMinimize()
{
	if (!handle || fullscreen)
	{
		return;
	}
	::SendMessageW(handle, WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

void DirectWindow::CaptionButtonMaximizeOrRestore()
{
	if (!handle || fullscreen)
	{
		return;
	}
	const BOOL z = ::IsZoomed(handle);
	::SendMessageW(handle, WM_SYSCOMMAND, z ? SC_RESTORE : SC_MAXIMIZE, 0);
}

void DirectWindow::SetFullscreen(bool value)
{
    if (!handle || fullscreen == value)
    {
        return;
    }

    if (value)
    {
        zoomedBeforeExclusiveFullscreen = (::IsZoomed(handle) != FALSE);
        CacheWindowedState();

        HMONITOR monitor = ::MonitorFromWindow(handle, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitorInfo{};
        monitorInfo.cbSize = sizeof(monitorInfo);
        if (!::GetMonitorInfoW(monitor, &monitorInfo))
        {
            return;
        }

        LONG_PTR style = ::GetWindowLongPtrW(handle, GWL_STYLE);
        LONG_PTR exStyle = ::GetWindowLongPtrW(handle, GWL_EXSTYLE);
        style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
        exStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);

        ::SetWindowLongPtrW(handle, GWL_STYLE, style | WS_POPUP);
        ::SetWindowLongPtrW(handle, GWL_EXSTYLE, exStyle);
        ::SetWindowPos(
            handle,
            HWND_TOPMOST,
            monitorInfo.rcMonitor.left,
            monitorInfo.rcMonitor.top,
            monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
            monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        fullscreen = true;
		ApplyImmortalWindowRounding(handle, borderless, true);
    }
    else
    {
        const bool restoreZoomed = zoomedBeforeExclusiveFullscreen;
        zoomedBeforeExclusiveFullscreen = false;

        ::SetWindowLongPtrW(handle, GWL_STYLE, windowedStyle);
        ::SetWindowLongPtrW(handle, GWL_EXSTYLE, windowedExStyle);
        ::SetWindowPlacement(handle, &windowedPlacement);
        ::SetWindowPos(
            handle,
            HWND_NOTOPMOST,
            0,
            0,
            0,
            0,
            SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

        if (restoreZoomed)
        {
            ::ShowWindow(handle, SW_SHOWMAXIMIZED);
            if (borderless)
            {
                HMONITOR mon = ::MonitorFromWindow(handle, MONITOR_DEFAULTTONEAREST);
                MONITORINFO mi{};
                mi.cbSize = sizeof(mi);
                if (::GetMonitorInfoW(mon, &mi))
                {
                    RECT wr{};
                    ::GetWindowRect(handle, &wr);
                    const bool spansMonitor =
                        wr.left <= mi.rcMonitor.left + 1 && wr.top <= mi.rcMonitor.top + 1 &&
                        wr.right >= mi.rcMonitor.right - 1 && wr.bottom >= mi.rcMonitor.bottom - 1;
                    const bool workSmallerThanMonitor =
                        (mi.rcWork.right - mi.rcWork.left) < (mi.rcMonitor.right - mi.rcMonitor.left) ||
                        (mi.rcWork.bottom - mi.rcWork.top) < (mi.rcMonitor.bottom - mi.rcMonitor.top);
                    if (spansMonitor && workSmallerThanMonitor)
                    {
                        ::ShowWindow(handle, SW_RESTORE);
                        ::ShowWindow(handle, SW_SHOWMAXIMIZED);
                    }
                }
            }
        }
        fullscreen = false;
		ApplyImmortalWindowRounding(handle, borderless, false);
    }
}

bool DirectWindow::IsFullscreen() const
{
    return fullscreen;
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

void DirectWindow::ReleaseWindowIcon()
{
	if (windowIcon)
	{
		::DestroyIcon(windowIcon);
		windowIcon = nullptr;
	}
}

void DirectWindow::SetIcon(const std::string &filepath)
{
	if (!handle || filepath.empty())
	{
		return;
	}
	ReleaseWindowIcon();

	const HRESULT hrCom = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	const bool coinitCleanup = (hrCom == S_OK);

	std::vector<uint8_t> bgra;
	UINT iw = 0;
	UINT ih = 0;
	const bool ok = LoadPngResizeToSquare(ToWString(filepath), 32, bgra, iw, ih);

	if (coinitCleanup)
	{
		CoUninitialize();
	}

	if (!ok || iw == 0 || ih == 0)
	{
		return;
	}

	windowIcon = CreateIconFrom32bppPBGRA((int)iw, (int)ih, bgra.data());
	if (windowIcon)
	{
		::SendMessageW(handle, WM_SETICON, ICON_BIG, (LPARAM)windowIcon);
		::SendMessageW(handle, WM_SETICON, ICON_SMALL, (LPARAM)windowIcon);
	}
}

void DirectWindow::Construct(const std::string &_title, uint32_t width, uint32_t height)
{
	Input.reset(new NativeInput{this});
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

    const auto screenW = GetSystemMetrics(SM_CXSCREEN);
    const auto screenH = GetSystemMetrics(SM_CYSCREEN);

    const DWORD styleBorderless = WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    const DWORD style = borderless ? styleBorderless : (DWORD)WS_OVERLAPPEDWINDOW;

    uint32_t clientW = width;
    uint32_t clientH = height;
    if (borderless && (clientW == 0 || clientH == 0))
    {
        RECT work{};
        ::SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
        const int aw = work.right - work.left;
        const int ah = work.bottom - work.top;
        if (clientW == 0)
        {
            clientW = (uint32_t)::MulDiv(aw, 85, 100);
        }
        if (clientH == 0)
        {
            clientH = (uint32_t)::MulDiv(ah, 85, 100);
        }
    }

    int posX;
    int posY;
    int winW;
    int winH;

    if (borderless)
    {
        RECT cr = { 0, 0, (LONG)clientW, (LONG)clientH };
        AdjustWindowRectEx(&cr, style, FALSE, 0);
        winW = cr.right - cr.left;
        winH = cr.bottom - cr.top;
        posX = (int)((screenW - winW) / 2);
        posY = (int)((screenH - winH) / 2);
    }
    else
    {
        posX = width == 0 ? CW_USEDEFAULT : (int)((screenW - (LONG)width) / 2);
        posY = height == 0 ? CW_USEDEFAULT : (int)((screenH - (LONG)height) / 2);
        winW = width == 0 ? CW_USEDEFAULT : (int)width;
        winH = height == 0 ? CW_USEDEFAULT : (int)height;
    }

    handle = ::CreateWindowExW(
        0,
        wc.lpszClassName,
        title.c_str(),
	    style,
	    posX,
	    posY,
	    winW,
	    winH,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr);

    if (!handle)
    {
        LOG::ERR("DirectWindow: CreateWindowExW failed");
        return;
    }

    ::SetWindowLongPtrW(handle, GWLP_USERDATA, (LONG_PTR)this);

    if (width == 0 || height == 0)
    {
        PostMessageW(handle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    }

    DragAcceptFiles(handle, TRUE);
    CacheWindowedState();
	ApplyImmortalWindowRounding(handle, borderless, false);
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
	ReleaseWindowIcon();
    ::DestroyWindow(handle);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

void DirectWindow::CacheWindowedState()
{
    if (!handle)
    {
        return;
    }

    windowedStyle = (DWORD)::GetWindowLongPtrW(handle, GWL_STYLE);
    windowedExStyle = (DWORD)::GetWindowLongPtrW(handle, GWL_EXSTYLE);
    windowedPlacement = {};
    windowedPlacement.length = sizeof(windowedPlacement);
    ::GetWindowPlacement(handle, &windowedPlacement);
}

}
