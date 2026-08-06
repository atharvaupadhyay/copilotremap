#include "hook.h"
#include "state_machine.h"
#include "input.h"
#include "log.h"
#include "config.h"
#include <array>

namespace Hook {

    static HHOOK g_hHook = nullptr;
    static HWND g_hWnd = nullptr;
    static StateMachine g_StateMachine;
    
    // Fixed size buffer on the data segment.
    struct EventBuffer {
        std::array<KBDLLHOOKSTRUCT, 8> events;
        size_t count = 0;
        
        void push_back(const KBDLLHOOKSTRUCT& ev) {
            if (count < events.size()) {
                events[count++] = ev;
            }
        }
        
        void clear() {
            count = 0;
        }
    };
    
    static EventBuffer g_Buffer;

    static EventType TranslateEvent(const KBDLLHOOKSTRUCT& kbd, bool isDown) {
        if (kbd.vkCode == VK_LWIN) return isDown ? EventType::LWinDown : EventType::LWinUp;
        if (kbd.vkCode == VK_LSHIFT) return isDown ? EventType::LShiftDown : EventType::LShiftUp;
        if (kbd.vkCode == VK_F23) return isDown ? EventType::F23Down : EventType::F23Up;
        return isDown ? EventType::OtherDown : EventType::OtherUp;
    }

    static void ExecuteActions(const ActionList& actions, const KBDLLHOOKSTRUCT* currentEvent, bool& outSuppress) {
        std::array<INPUT, 16> inputsToInject{};
        size_t injectCount = 0;

        for (size_t i = 0; i < actions.count; ++i) {
            switch (actions.actions[i]) {
                case Action::Pass:
                    break;
                case Action::Suppress:
                    outSuppress = true;
                    break;
                case Action::BufferCurrent:
                    if (currentEvent) {
                        g_Buffer.push_back(*currentEvent);
                    }
                    break;
                case Action::ClearBuffer:
                    g_Buffer.clear();
                    break;
                case Action::InjectBuffered:
                    for (size_t j = 0; j < g_Buffer.count; ++j) {
                        if (injectCount < inputsToInject.size()) {
                            inputsToInject[injectCount++] = Input::ToInput(g_Buffer.events[j]);
                        }
                    }
                    break;
                case Action::InjectCurrent:
                    if (currentEvent && injectCount < inputsToInject.size()) {
                        inputsToInject[injectCount++] = Input::ToInput(*currentEvent);
                    }
                    break;
                case Action::InjectCtrlDown:
                    if (injectCount < inputsToInject.size()) {
                        inputsToInject[injectCount++] = Input::CreateCtrlInput(true);
                    }
                    break;
                case Action::InjectCtrlUp:
                    if (injectCount < inputsToInject.size()) {
                        inputsToInject[injectCount++] = Input::CreateCtrlInput(false);
                    }
                    break;
                case Action::StartTimer:
                    SetTimer(g_hWnd, TIMER_ID, CopilotTimeoutMs, nullptr);
                    break;
                case Action::StopTimer:
                    KillTimer(g_hWnd, TIMER_ID);
                    break;
            }
        }

        if (injectCount > 0) {
            SendInput(static_cast<UINT>(injectCount), inputsToInject.data(), sizeof(INPUT));
        }
    }

    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode == HC_ACTION) {
            auto* kbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

            if ((kbd->flags & LLKHF_INJECTED) == 0) {
                bool isDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
                EventType eventType = TranslateEvent(*kbd, isDown);
                
                ActionList actions = g_StateMachine.ProcessEvent(eventType);
                
                bool suppress = false;
                ExecuteActions(actions, kbd, suppress);
                
                if (suppress) {
                    return 1;
                }
            }
        }
        return CallNextHookEx(g_hHook, nCode, wParam, lParam);
    }

    bool InstallKeyboardHook(HWND hwnd) {
        g_hWnd = hwnd;
        g_StateMachine.Reset();
        g_Buffer.clear();
        g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(nullptr), 0);
        return g_hHook != nullptr;
    }

    void RemoveKeyboardHook() {
        if (g_hHook) {
            UnhookWindowsHookEx(g_hHook);
            g_hHook = nullptr;
        }
    }

    void HandleTimer() {
        KillTimer(g_hWnd, TIMER_ID); // Enforce one-shot
        ActionList actions = g_StateMachine.ProcessEvent(EventType::Timeout);
        bool suppress = false;
        ExecuteActions(actions, nullptr, suppress);
    }

    void CleanUp() {
        if (g_StateMachine.IsCtrlInjected()) {
            INPUT in = Input::CreateCtrlInput(false);
            SendInput(1, &in, sizeof(INPUT));
        }
        g_StateMachine.Reset();
        g_Buffer.clear();
        KillTimer(g_hWnd, TIMER_ID);
    }

}