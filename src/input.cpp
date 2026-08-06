#include "input.h"

namespace Input {

    INPUT ToInput(const KBDLLHOOKSTRUCT& ev) {
        INPUT in = {};
        in.type = INPUT_KEYBOARD;
        in.ki.wVk = (WORD)ev.vkCode;
        in.ki.wScan = (WORD)ev.scanCode;
        in.ki.dwFlags = 0;
        
        if (ev.flags & LLKHF_EXTENDED) 
            in.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        if (ev.flags & LLKHF_UP) 
            in.ki.dwFlags |= KEYEVENTF_KEYUP;
            
        in.ki.time = 0; // Let OS provide current time to avoid timing faults
        in.ki.dwExtraInfo = ev.dwExtraInfo;
        return in;
    }

    INPUT CreateCtrlInput(bool isDown) {
        INPUT in = {};
        in.type = INPUT_KEYBOARD;
        in.ki.wVk = VK_LCONTROL;
        in.ki.wScan = (WORD)MapVirtualKey(VK_LCONTROL, MAPVK_VK_TO_VSC);
        in.ki.dwFlags = isDown ? 0 : KEYEVENTF_KEYUP;
        in.ki.time = 0;
        return in;
    }

}
