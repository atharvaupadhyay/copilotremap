#include "hook.h"
#include "state_machine.h"
#include "input.h"
#include "log.h"
#include "config.h"
#include <array>
#include <cassert>

namespace Hook {

    static HHOOK g_hHook = nullptr;
    static HWND g_hWnd = nullptr;
    static StateMachine g_StateMachine;
    static bool g_ctrlIsInjected = false;
    
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

#ifdef _DEBUG
    static const char* GetEventName(EventType event) {
        switch (event) {
            case EventType::LWinDown: return "LWIN_DOWN";
            case EventType::LWinUp: return "LWIN_UP";
            case EventType::LShiftDown: return "LSHIFT_DOWN";
            case EventType::LShiftUp: return "LSHIFT_UP";
            case EventType::F23Down: return "F23_DOWN";
            case EventType::F23Up: return "F23_UP";
            case EventType::OtherDown: return "OTHER_DOWN";
            case EventType::OtherUp: return "OTHER_UP";
            case EventType::Timeout: return "TIMEOUT";
            default: return "UNKNOWN";
        }
    }
#endif

    static void ExecuteDecision(const Decision& decision, const KBDLLHOOKSTRUCT* currentEvent) {
        std::array<INPUT, 16> inputsToInject{};
        size_t injectCount = 0;

        if (decision.bufferCurrent && currentEvent) {
            g_Buffer.push_back(*currentEvent);
        }

        // 1. Replay buffered events
        if (decision.replayBuffer) {
            for (size_t j = 0; j < g_Buffer.count; ++j) {
                if (injectCount < inputsToInject.size()) {
                    INPUT in = Input::ToInput(g_Buffer.events[j]);
                    inputsToInject[injectCount++] = in;
                    LOG("SendInput: Replayed physical VK=0x%02X (Down=%d)\n", in.ki.wVk, (in.ki.dwFlags & KEYEVENTF_KEYUP) == 0);
                }
            }
        }

        // 2. Inject Ctrl
        if (decision.injectCtrlDown && !g_ctrlIsInjected) {
            if (injectCount < inputsToInject.size()) {
                inputsToInject[injectCount++] = Input::CreateCtrlInput(true);
                g_ctrlIsInjected = true;
                LOG("SendInput: Injected synthetic Ctrl DOWN\n");
            }
        }
        if (decision.injectCtrlUp && g_ctrlIsInjected) {
            if (injectCount < inputsToInject.size()) {
                inputsToInject[injectCount++] = Input::CreateCtrlInput(false);
                g_ctrlIsInjected = false;
                LOG("SendInput: Injected synthetic Ctrl UP\n");
            }
        }

        if (injectCount > 0) {
            UINT sent = SendInput(static_cast<UINT>(injectCount), inputsToInject.data(), sizeof(INPUT));
#ifdef _DEBUG
            LOG("SendInput returned %u/%zu\n", sent, injectCount);
#else
            (void)sent;
#endif
        }

        // 3. Timer operations
        if (decision.stopTimer) {
            LOG("KillTimer\n");
            KillTimer(g_hWnd, TIMER_ID);
        }
        if (decision.startTimer) {
            LOG("SetTimer(%lu ms)\n", CopilotTimeoutMs);
            SetTimer(g_hWnd, TIMER_ID, CopilotTimeoutMs, nullptr);
        }

        // 4. Clear buffer
        if (decision.clearBuffer) {
            g_Buffer.clear();
        }
    }

    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode == HC_ACTION) {
            auto* kbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
#ifdef _DEBUG
            LOG(
                "RAW  WPARAM=0x%llX  VK=0x%02X  SC=0x%02X  FLAGS=0x%X  EXTRA=0x%llX\n",
                static_cast<unsigned long long>(wParam),
                kbd->vkCode,
                kbd->scanCode,
                kbd->flags,
                static_cast<unsigned long long>(kbd->dwExtraInfo)
            );
#endif
            if ((kbd->flags & LLKHF_INJECTED) == 0) {
                bool isDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
                EventType eventType = TranslateEvent(*kbd, isDown);
                
#ifdef _DEBUG
                const char* stateBefore = g_StateMachine.GetStateName();
#endif
                Decision decision = g_StateMachine.ProcessEvent(eventType);
#ifdef _DEBUG
                const char* stateAfter = g_StateMachine.GetStateName();
                
                const char* msgName = (wParam == WM_KEYDOWN) ? "WM_KEYDOWN" : (wParam == WM_SYSKEYDOWN) ? "WM_SYSKEYDOWN" : (wParam == WM_KEYUP) ? "WM_KEYUP" : (wParam == WM_SYSKEYUP) ? "WM_SYSKEYUP" : "UNKNOWN";
                
                LOG("[%lu] EVENT: %s | %s | VK=0x%02X SC=0x%02X FLAGS=0x%X\n"
                    "  PhysLWin: %d | PhysLShift: %d | SynthCtrl: %d\n"
                    "  TRANSITION: %s -> %s\n"
                    "  DECISION: suppress=%d replay=%d buffer=%d clear=%d ctrlDown=%d ctrlUp=%d\n"
                    "  Buffer Size Before: %zu\n\n",
                    GetTickCount(),
                    msgName,
                    GetEventName(eventType),
                    kbd->vkCode, kbd->scanCode, kbd->flags,
                    g_StateMachine.physicalLWinDown, g_StateMachine.physicalLShiftDown, g_StateMachine.syntheticCtrlDown,
                    stateBefore, stateAfter,
                    decision.suppress, decision.replayBuffer, decision.bufferCurrent,
                    decision.clearBuffer, decision.injectCtrlDown, decision.injectCtrlUp,
                    g_Buffer.count
                );
#endif
                
                assert(g_Buffer.count <= g_Buffer.events.size());

                ExecuteDecision(decision, kbd);
                
                if (decision.suppress) {
                    return 1;
                }
            }
        }

        return CallNextHookEx(g_hHook, nCode, wParam, lParam);
    }

    bool InstallKeyboardHook(HWND hwnd) {
        g_hWnd = hwnd;
        g_ctrlIsInjected = false;
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
        LOG("KillTimer (HandleTimer)\n");
        KillTimer(g_hWnd, TIMER_ID); // Enforce one-shot
        
#ifdef _DEBUG
        const char* stateBefore = g_StateMachine.GetStateName();
#endif
        Decision decision = g_StateMachine.ProcessEvent(EventType::Timeout);
#ifdef _DEBUG
        const char* stateAfter = g_StateMachine.GetStateName();
        
        LOG("[%lu] EVENT: TIMEOUT\n"
            "  PhysLWin: %d | PhysLShift: %d | SynthCtrl: %d\n"
            "  TRANSITION: %s -> %s\n"
            "  DECISION: suppress=%d replay=%d buffer=%d clear=%d ctrlDown=%d ctrlUp=%d\n"
            "  Buffer Size Before: %zu\n\n",
            GetTickCount(),
            g_StateMachine.physicalLWinDown, g_StateMachine.physicalLShiftDown, g_StateMachine.syntheticCtrlDown,
            stateBefore, stateAfter,
            decision.suppress, decision.replayBuffer, decision.bufferCurrent,
            decision.clearBuffer, decision.injectCtrlDown, decision.injectCtrlUp,
            g_Buffer.count
        );
#endif

        ExecuteDecision(decision, nullptr);
    }

    void CleanUp() {
        if (g_ctrlIsInjected) {
            INPUT in = Input::CreateCtrlInput(false);
            UINT sent = SendInput(1, &in, sizeof(INPUT));
            g_ctrlIsInjected = false;
#ifdef _DEBUG
            LOG("SendInput: Injected synthetic Ctrl UP (CleanUp)\n");
            LOG("SendInput returned %u/1\n", sent);
#else
            (void)sent;
#endif
        }
        g_StateMachine.Reset();
        g_Buffer.clear();
        LOG("KillTimer (CleanUp)\n");
        KillTimer(g_hWnd, TIMER_ID);
    }

}