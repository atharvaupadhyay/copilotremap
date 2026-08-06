#pragma once
#include <windows.h>

namespace Hook {

    bool InstallKeyboardHook(HWND hwnd);
    void RemoveKeyboardHook();
    
    // Called when the fallback timer fires.
    void HandleTimer();
    
    // Called on application exit / session end to clean up injected Ctrl keys.
    void CleanUp();

}
