/*
 * Klavye Kilidi
 *
 * Açıkken klavyeden gelen TÜM tuş basışlarını yutar; böylece klavyeyi
 * temizlerken yanlışlıkla bir tuşa basmak hiçbir şeyi etkilemez.
 *
 * Kilidi açmak için fareyle "Kilidi Aç ve Çık" düğmesine tıklayın
 * (ya da pencereyi kapatın). Güvenlik için varsayılan olarak 15 dakika
 * sonra kilit kendiliğinden açılır.
 *
 * Komut satırı: KlavyeKilidi.exe [dakika]
 *   dakika = otomatik kilit açma süresi, 0 = süresiz
 *
 * Not: Ctrl+Alt+Del Windows tarafından özel olarak işlenir ve hiçbir
 * program tarafından engellenemez; bu bilinçli bir güvenlik ağıdır.
 */

#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>

#define ID_BTN_UNLOCK   101
#define ID_TXT_TITLE    201
#define ID_TXT_INFO     202
#define ID_TXT_COUNT    203
#define ID_TXT_TIMER    204
#define TIMER_ID        1
#define WM_KEY_BLOCKED  (WM_APP + 1)

#define CLR_BG     RGB(28, 32, 44)
#define CLR_TITLE  RGB(255, 183, 64)
#define CLR_INFO   RGB(214, 218, 228)
#define CLR_COUNT  RGB(255, 255, 255)
#define CLR_TIMER  RGB(150, 158, 175)

static HHOOK         g_hook    = NULL;
static HWND          g_hwnd    = NULL;
static HWND          g_txtCount = NULL;
static HWND          g_txtTimer = NULL;
static volatile LONG g_blocked = 0;
static BOOL          g_timed   = TRUE;
static int           g_remaining = 15 * 60;   /* saniye */
static int           g_dpi     = 96;
static HFONT         g_fTitle, g_fBody, g_fCount;
static HBRUSH        g_bgBrush;

#define S(x) MulDiv((x), g_dpi, 96)

/* Düşük seviyeli klavye kancası: her tuş olayını yutar. Hızlı olmalı. */
static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            InterlockedIncrement(&g_blocked);
            if (g_hwnd) PostMessageW(g_hwnd, WM_KEY_BLOCKED, 0, 0);
        }
        return 1;  /* 0 olmayan değer: olay hiçbir yere iletilmez */
    }
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

static void UpdateCount(void)
{
    wchar_t buf[64];
    wsprintfW(buf, L"Engellenen tuş basışı: %ld", (long)g_blocked);
    SetWindowTextW(g_txtCount, buf);
}

static void UpdateTimer(void)
{
    wchar_t buf[96];
    if (g_timed)
        wsprintfW(buf, L"Güvenlik için otomatik kilit açma: %02d:%02d",
                  g_remaining / 60, g_remaining % 60);
    else
        lstrcpyW(buf, L"Otomatik kilit açma kapalı");
    SetWindowTextW(g_txtTimer, buf);
}

static HWND MakeStatic(HWND parent, HINSTANCE hi, int id, const wchar_t *text,
                       int x, int y, int w, int h, HFONT font)
{
    HWND c = CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE | SS_CENTER,
                             S(x), S(y), S(w), S(h), parent, (HMENU)(INT_PTR)id, hi, NULL);
    SendMessageW(c, WM_SETFONT, (WPARAM)font, TRUE);
    return c;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        HINSTANCE hi = ((LPCREATESTRUCTW)lp)->hInstance;
        MakeStatic(hwnd, hi, ID_TXT_TITLE, L"KLAVYE KİLİTLİ", 20, 24, 420, 52, g_fTitle);
        MakeStatic(hwnd, hi, ID_TXT_INFO,
                   L"Şu anda klavyedeki hiçbir tuş çalışmıyor.\r\n"
                   L"Klavyenizi rahatça temizleyebilirsiniz.",
                   20, 88, 420, 52, g_fBody);
        g_txtCount = MakeStatic(hwnd, hi, ID_TXT_COUNT, L"", 20, 152, 420, 30, g_fCount);
        g_txtTimer = MakeStatic(hwnd, hi, ID_TXT_TIMER, L"", 20, 188, 420, 24, g_fBody);

        HWND btn = CreateWindowExW(0, L"BUTTON", L"Kilidi Aç ve Çık",
                                   WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                   S(125), S(232), S(210), S(54), hwnd,
                                   (HMENU)(INT_PTR)ID_BTN_UNLOCK, hi, NULL);
        SendMessageW(btn, WM_SETFONT, (WPARAM)g_fCount, TRUE);

        UpdateCount();
        UpdateTimer();
        if (g_timed) SetTimer(hwnd, TIMER_ID, 1000, NULL);
        return 0;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wp;
        switch (GetDlgCtrlID((HWND)lp)) {
        case ID_TXT_TITLE: SetTextColor(hdc, CLR_TITLE); break;
        case ID_TXT_COUNT: SetTextColor(hdc, CLR_COUNT); break;
        case ID_TXT_TIMER: SetTextColor(hdc, CLR_TIMER); break;
        default:           SetTextColor(hdc, CLR_INFO);  break;
        }
        SetBkColor(hdc, CLR_BG);
        return (LRESULT)g_bgBrush;
    }

    case WM_KEY_BLOCKED:
        UpdateCount();
        return 0;

    case WM_TIMER:
        if (g_timed) {
            if (--g_remaining <= 0) DestroyWindow(hwnd);
            else UpdateTimer();
        }
        return 0;

    case WM_COMMAND:
        if (LOWORD(wp) == ID_BTN_UNLOCK) DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ID);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

static HFONT MakeFont(int px, int weight)
{
    return CreateFontW(-S(px), 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                       DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmd, int nShow)
{
    (void)hPrev; (void)nShow;

    /* Aynı anda yalnızca tek kopya çalışsın. */
    HANDLE mutex = CreateMutexW(NULL, TRUE, L"KlavyeKilidi_TekOrnek");
    if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;

    /* İsteğe bağlı: otomatik kilit açma süresi (dakika). 0 = süresiz. */
    if (cmd && *cmd) {
        char *end = NULL;
        long minutes = strtol(cmd, &end, 10);
        if (end != cmd && minutes >= 0) {
            g_timed = minutes > 0;
            g_remaining = (int)(minutes * 60);
        }
    }

    SetProcessDPIAware();
    HDC dc = GetDC(NULL);
    g_dpi = GetDeviceCaps(dc, LOGPIXELSX);
    ReleaseDC(NULL, dc);

    INITCOMMONCONTROLSEX icc = { sizeof icc, ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);

    g_bgBrush = CreateSolidBrush(CLR_BG);
    g_fTitle  = MakeFont(34, FW_BOLD);
    g_fBody   = MakeFont(17, FW_NORMAL);
    g_fCount  = MakeFont(20, FW_SEMIBOLD);

    WNDCLASSEXW wc = { 0 };
    wc.cbSize        = sizeof wc;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    wc.hIcon         = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
    wc.hbrBackground = g_bgBrush;
    wc.lpszClassName = L"KlavyeKilidiWnd";
    RegisterClassExW(&wc);

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    RECT r = { 0, 0, S(460), S(310) };
    AdjustWindowRect(&r, style, FALSE);
    int w = r.right - r.left, h = r.bottom - r.top;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    g_hwnd = CreateWindowExW(WS_EX_TOPMOST, L"KlavyeKilidiWnd", L"Klavye Kilidi",
                             style, x, y, w, h, NULL, NULL, hInst, NULL);
    if (!g_hwnd) return 1;

    g_hook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandleW(NULL), 0);
    if (!g_hook) {
        MessageBoxW(g_hwnd, L"Klavye kilidi kurulamadı.", L"Klavye Kilidi", MB_ICONERROR);
        return 1;
    }

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    MSG m;
    while (GetMessageW(&m, NULL, 0, 0) > 0) {
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }

    UnhookWindowsHookEx(g_hook);
    if (mutex) CloseHandle(mutex);
    return 0;
}
