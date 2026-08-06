#pragma once
#include <array>
#include <type_traits>
#include <cstddef>

enum class EventType {
    LWinDown,
    LWinUp,
    LShiftDown,
    LShiftUp,
    F23Down,
    F23Up,
    OtherDown,
    OtherUp,
    Timeout
};

enum class Action {
    Pass,
    Suppress,
    BufferCurrent,
    ClearBuffer,
    InjectBuffered,
    InjectCurrent,
    InjectCtrlDown,
    InjectCtrlUp,
    StartTimer,
    StopTimer
};

static_assert(std::is_trivially_copyable_v<EventType>);
static_assert(std::is_trivially_copyable_v<Action>);

struct ActionList {
    std::array<Action, 8> actions;
    size_t count = 0;

    void add(Action a) {
        if (count < actions.size()) {
            actions[count++] = a;
        }
    }
};

class StateMachine {
public:
    void Reset();
    ActionList ProcessEvent(EventType event);
    bool IsCtrlInjected() const;

private:
    enum class State {
        IDLE,
        PENDING_WIN,
        PENDING_WIN_SHIFT,
        COPILOT_ACTIVE
    };

    State currentState = State::IDLE;
    bool ctrlInjected = false;
};
