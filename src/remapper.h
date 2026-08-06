#pragma once

#include <windows.h>

bool HandleKeyboardEvent(
    const KBDLLHOOKSTRUCT& kb,
    bool keyDown);