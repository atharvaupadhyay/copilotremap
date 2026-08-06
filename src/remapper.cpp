#include "remapper.h"

#include <windows.h>

namespace
{
    bool g_leftWinDown = false;
    bool g_leftShiftDown = false;
    bool g_ctrlHeld = false;

    void SendCtrl(bool down)
    {
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_CONTROL;

        if (!down)
            input.ki.dwFlags = KEYEVENTF_KEYUP;

        SendInput(1, &input, sizeof(INPUT));
    }
}

bool HandleKeyboardEvent(const KBDLLHOOKSTRUCT& kb, bool keyDown)
{
    switch (kb.vkCode)
    {
        case VK_LWIN:
            g_leftWinDown = keyDown;
            break;

        case VK_LSHIFT:
            g_leftShiftDown = keyDown;
            break;

        case VK_F23:
        {
            // Copilot = Win + Shift + F23
            if (g_leftWinDown && g_leftShiftDown)
            {
                if (keyDown)
                {
                    if (!g_ctrlHeld)
                    {
                        SendCtrl(true);
                        g_ctrlHeld = true;
                    }
                }
                else
                {
                    if (g_ctrlHeld)
                    {
                        SendCtrl(false);
                        g_ctrlHeld = false;
                    }
                }

                // Eat the F23 event
                return true;
            }

            break;
        }

        default:
            break;
    }

    return false;
}