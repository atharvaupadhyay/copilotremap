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

struct Decision {
    bool suppress = false;
    bool bufferCurrent = false;
    bool replayBuffer = false;
    bool clearBuffer = false;
    bool injectCtrlDown = false;
    bool injectCtrlUp = false;
    bool startTimer = false;
    bool stopTimer = false;
};

static_assert(std::is_trivially_copyable_v<EventType>);
static_assert(std::is_trivially_copyable_v<Decision>);

class StateMachine {
public:
    void Reset();
    Decision ProcessEvent(EventType event);
    const char* GetStateName() const;

private:
    enum class State {
        IDLE,
        PENDING_LWIN,
        PENDING_LSHIFT,
        PENDING_BOTH,
        COPILOT_ACTIVE,
        WAIT_RELEASE
    };

    State currentState = State::IDLE;
    bool seenWinUp = false;
    bool seenShiftUp = false;
};
