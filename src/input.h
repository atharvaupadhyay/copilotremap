#pragma once
#include <windows.h>

namespace Input {

    // Converts a captured hook event back into an INPUT structure for injection.
    INPUT ToInput(const KBDLLHOOKSTRUCT& ev);

    // Creates an INPUT structure for Left Ctrl.
    INPUT CreateCtrlInput(bool isDown);

}
