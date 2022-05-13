#define WIN32_LEAN_AND_MEAN
#define UNICODE

#include <windows.h>

#include "wimer.h"


#include <codecvt>
#include <locale>
#include <string>
using std::string;
using std::wstring;
using std::to_string;

#include <sstream>
using std::stringstream;
using std::wstringstream;

#include <vector>
using std::vector;

using std::exception;


const LPCWSTR WC_NAME = L"wimer-main-cls";
const LPCWSTR W_NAME = L"wimer-main";

const LPWSTR NL = L"\r\n";

const DWORD ID_EDITLOG = 101;

const DWORD ID_TIMER1 = 1;
const DWORD TIMER1_INTERVAL = 1000;

const int WIDTH = 30;

const int PERIOD_DEFAULT = 300;
const int PERIOD_DELTA = 60;

const COLORREF BG_COLOR = 0x00af91ff;    // Baker-Miller pink
const COLORREF BRUSH_COLOR = 0x00a0e4a8; // Granny Smith apple
const COLORREF LINE_COLOR = 0x0089858b;  // Taupe gray
const COLORREF TEXT_COLOR = 0x00080808;  // Vampire black

typedef struct {
    HINSTANCE hInstance;
    HWND hwndMain;
    HWND hwndLog;
    DWORD timer1;

    INT period;
    INT elapsed;

    HBRUSH brush;
    HPEN pen;
} instance_data_t;

static instance_data_t gdata;

HMONITOR current_monitor_handle()
{
    POINT pt;
    GetCursorPos(&pt);
    return MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);
}

MONITORINFOEX get_monitor_info(HMONITOR hmon)
{
    MONITORINFOEX mi;
    mi.cbSize = sizeof(MONITORINFOEX);

    if (hmon == NULL) {
        hmon = current_monitor_handle();
    }

    if (!GetMonitorInfo(hmon, &mi)) {
        throw exception("failed to get monitor info");
    }

    return mi;
}


string w2s(const wstring &var)
{
   static std::locale loc("");
   auto &facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
   return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).to_bytes(var);
}

wstring s2w(const string &var)
{
   static std::locale loc("");
   auto &facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
   return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).from_bytes(var);
}

// trim from start (in place)
static inline void ltrim(wstring &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(wstring &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(wstring &s) {
    ltrim(s);
    rtrim(s);
}

wstring join_strings(const vector<wstring>& parts, wstring delim=L" ")
{
    wstringstream ss;
    bool first = true;
    for(auto it = parts.begin(), eit=parts.end(); it != eit; ++it) {
        if (first) {
            first = false;
        } else {
            ss << delim;
        }
        ss << *it;
    }
    return ss.str();
}

void append_log(const wstring& txt, HWND hwnd)
{
    int index = GetWindowTextLength(hwnd);
    SendMessage(hwnd, EM_SETSEL, index, index);
    SendMessage(hwnd, EM_REPLACESEL, 0, (LPARAM) txt.c_str());
}

void log(const wstring& msg, HWND hwnd)
{
    append_log(msg, hwnd);
    append_log(NL, hwnd);
}

void log_debug(LPCWSTR txt)
{
    log(txt, gdata.hwndLog);
}
void log_debug(const wstring& txt)
{
    log(txt, gdata.hwndLog);
}

void log_info(const wstring& txt)
{
    log(txt, gdata.hwndLog);
}

template<typename T>
wstring _w(T in)
{
    stringstream ss;
    ss << in;
    return s2w(ss.str());
}

void show_hide_window(HWND hwnd, bool show)
{
    ShowWindow(hwnd, show ? SW_SHOW : SW_HIDE);
}

wstring get_last_error_message()
{
    DWORD err = GetLastError();
    LPWSTR messageBuffer = nullptr;
    size_t size = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&messageBuffer,
            0,
            NULL);

    wstring msg(messageBuffer, size);
    LocalFree(messageBuffer);
    return msg;
}


void log_error(LPCWSTR txt)
{
    wstring err = get_last_error_message();
    wstring msg = txt;
    msg += L": ";
    msg += err;
    log(msg, gdata.hwndLog);
}

LRESULT CALLBACK WindowProc(
  _In_ HWND   hwnd,
  _In_ UINT   uMsg,
  _In_ WPARAM wParam,
  _In_ LPARAM lParam
)
{
    switch(uMsg) {
        case WM_KEYUP:
            switch (wParam) {
                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    break;

                default:
                    break;
            }
            break;

        case WM_MOUSEWHEEL:
            {
                auto d = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
                auto flags = GET_KEYSTATE_WPARAM(wParam);
                if (flags & MK_CONTROL) {
                    gdata.period += d * PERIOD_DELTA;
                    if (gdata.period < 0) {
                        gdata.period = 0;
                    }
                    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
                }
            }
            break;

        case WM_LBUTTONDBLCLK:
            {
                auto flags = GET_KEYSTATE_WPARAM(wParam);
                if (flags & MK_CONTROL) {
                    gdata.period = PERIOD_DEFAULT;
                    gdata.elapsed = 0;
                    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
                }

            }
            break;

        case WM_TIMER:
            gdata.elapsed += 1;
            RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            struct {
                HPEN pen;
                COLORREF color;
            } old = {
                static_cast<HPEN>(SelectObject(hdc, gdata.pen)),
                SetTextColor(hdc, TEXT_COLOR),
            };

            auto total = max(1, gdata.period);
            auto elapsed = max(0, gdata.elapsed);

            auto rc = ps.rcPaint;
            auto H = rc.bottom - rc.top;
            auto h = H * (1. * elapsed / total);
            rc.top += h;

            FillRect(hdc, &rc, gdata.brush);
            MoveToEx(hdc, rc.left, rc.top, NULL);
            LineTo(hdc, rc.right, rc.top);

            rc.top = ps.rcPaint.top;
            SetBkMode(hdc, TRANSPARENT);
            auto info = s2w(to_string(gdata.period / 60));

            auto rh = DrawText(hdc, info.c_str(), info.size(), &rc, DT_SINGLELINE | DT_NOCLIP | DT_CENTER) ;

            MoveToEx(hdc, rc.left, rh, NULL);
            LineTo(hdc, rc.right, rh);

            auto left = max(0, total - elapsed);
            info = s2w(to_string(left / 60));
            rc.top += rh;
            rh += DrawText(hdc, info.c_str(), info.size(), &rc, DT_SINGLELINE | DT_NOCLIP | DT_CENTER) ;

            MoveToEx(hdc, rc.left, rh, NULL);
            LineTo(hdc, rc.right, rh);

            SelectObject(hdc, old.pen);
            SetTextColor(hdc, old.color);
            EndPaint(hwnd, &ps);
        }

        default:
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

instance_data_t create_gui(HINSTANCE hInstance)
{
    WNDCLASSEX wce = {
        sizeof(wce),
        CS_DBLCLKS,
        WindowProc,
        0,
        0,
        hInstance,
        LoadIcon(NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW),
        CreateSolidBrush(BG_COLOR),
        NULL,
        WC_NAME,
        NULL,
    };

    ATOM wc = RegisterClassEx(&wce);
    if (!wc) {
        throw exception("failed to register window class");
    }

    DWORD dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;

    auto W = WIDTH;
    auto mi = get_monitor_info(NULL);

    HWND hwndMain = CreateWindowEx(
            dwExStyle,
            MAKEINTATOM(wc),
            W_NAME,
            dwStyle,
            mi.rcMonitor.right - W,
            0,
            W,
            mi.rcMonitor.bottom - mi.rcMonitor.top,
            NULL, NULL, NULL, NULL);
    if (!hwndMain) {
        throw exception("failed to register main window");
    }
    HWND hwndLog = CreateWindowEx(
            0, L"EDIT", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
            0, 0, 0, 0,
            hwndMain,
            // reinterpret_cast<HMENU>(ID_EDITLOG),
            NULL, NULL, NULL);
    if (!hwndLog) {
        throw exception("failed to register log window");
    }
    SendMessage(hwndLog, EM_SETREADONLY, TRUE, 0);

    DWORD tmr = SetTimer(hwndMain, ID_TIMER1, TIMER1_INTERVAL, NULL);
    if (!tmr) {
        throw exception("failed to register log window");
    }

    HBRUSH brush = CreateSolidBrush(BRUSH_COLOR);
    HPEN pen = CreatePen(PS_SOLID, 1, LINE_COLOR);
    return {hInstance, hwndMain, hwndLog, tmr, PERIOD_DEFAULT, 0, brush, pen};
}

LIBWIMER_EXPORT int CALLBACK libwimer_main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    try {
        gdata = create_gui(hInstance);
    } catch (const exception& e) {
        log_error(s2w(e.what()).c_str());
        wstring err = L"Error ";
        err += s2w(e.what()) + L": " + get_last_error_message();
        MessageBox(NULL, err.c_str(), L"", MB_OK);
        return 1;
    }

    show_hide_window(gdata.hwndMain, true);

    BOOL ok;
    MSG msg = {0};
    while((ok = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (ok == -1) {
            // idk, maybe exit?
            // log_error(L"exit?");
            break;
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}
