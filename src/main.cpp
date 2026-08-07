#include <windows.h>
#include "hook.h"
#include "config.h"

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TIMER:
            if (wParam == TIMER_ID) {
                Hook::HandleTimer();
            }
            return 0;
            
        case WM_QUERYENDSESSION:
            return TRUE;
            
        case WM_ENDSESSION:
            if (wParam == TRUE) {
                Hook::CleanUp();
            }
            return 0;
            
        case WM_KILLFOCUS:
        case WM_CANCELMODE:
            Hook::CleanUp();
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    HANDLE hMutex = CreateMutex(nullptr, TRUE, L"CtrlPilot_SingleInstance_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return 0; // Already running
    }

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"CtrlPilot_MessageWindow";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, wc.lpszClassName, L"CtrlPilot", 
        0, 0, 0, 0, 0, 
        HWND_MESSAGE, nullptr, hInstance, nullptr
    );

    if (!Hook::InstallKeyboardHook(hwnd)) {
        MessageBox(nullptr, L"Failed to install keyboard hook.", L"CtrlPilot Error", MB_ICONERROR);
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Hook::CleanUp();
    Hook::RemoveKeyboardHook();

    if (hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }

    return 0;
}