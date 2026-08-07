#include "state_machine.h"

void StateMachine::Reset() {
    currentState = State::IDLE;
    seenWinUp = false;
    seenShiftUp = false;
}

const char* StateMachine::GetStateName() const {
    switch (currentState) {
        case State::IDLE: return "IDLE";
        case State::PENDING_LWIN: return "PENDING_LWIN";
        case State::PENDING_LSHIFT: return "PENDING_LSHIFT";
        case State::PENDING_BOTH: return "PENDING_BOTH";
        case State::COPILOT_ACTIVE: return "COPILOT_ACTIVE";
        case State::WAIT_RELEASE: return "WAIT_RELEASE";
        default: return "UNKNOWN";
    }
}

Decision StateMachine::ProcessEvent(EventType event) {
    Decision d;

    switch (currentState) {
        case State::IDLE:
            if (event == EventType::LWinDown) {
                currentState = State::PENDING_LWIN;
                d.suppress = true;
                d.bufferCurrent = true;
                d.startTimer = true;
                return d;
            }
            if (event == EventType::LShiftDown) {
                currentState = State::PENDING_LSHIFT;
                d.suppress = true;
                d.bufferCurrent = true;
                d.startTimer = true;
                return d;
            }
            return d;

        case State::PENDING_LWIN:
            if (event == EventType::LShiftDown) {
                currentState = State::PENDING_BOTH;
                d.suppress = true;
                d.bufferCurrent = true;
                d.startTimer = true;
                return d;
            }
            
            currentState = State::IDLE;
            d.stopTimer = true;
            d.replayBuffer = true;
            d.clearBuffer = true;
            return d;

        case State::PENDING_LSHIFT:
            if (event == EventType::LWinDown) {
                currentState = State::PENDING_BOTH;
                d.suppress = true;
                d.bufferCurrent = true;
                d.startTimer = true;
                return d;
            }
            
            currentState = State::IDLE;
            d.stopTimer = true;
            d.replayBuffer = true;
            d.clearBuffer = true;
            return d;

        case State::PENDING_BOTH:
            if (event == EventType::F23Down) {
                currentState = State::COPILOT_ACTIVE;
                d.stopTimer = true;
                d.clearBuffer = true;
                d.suppress = true;
                d.injectCtrlDown = true;
                return d;
            }
            
            currentState = State::IDLE;
            d.stopTimer = true;
            d.replayBuffer = true;
            d.clearBuffer = true;
            return d;

        case State::COPILOT_ACTIVE:
            if (event == EventType::F23Down || event == EventType::LWinDown || event == EventType::LShiftDown) {
                d.suppress = true;
                return d;
            }
            if (event == EventType::LWinUp || event == EventType::LShiftUp) {
                // Do not release on the first modifier-up event.
                d.suppress = true;
                return d;
            }
            if (event == EventType::F23Up) {
                currentState = State::WAIT_RELEASE;
                seenWinUp = false;
                seenShiftUp = false;
                d.injectCtrlUp = true;
                d.suppress = true;
                return d;
            }
            return d;

        case State::WAIT_RELEASE:
            if (event == EventType::LWinUp) {
                seenWinUp = true;
                d.suppress = true;
            } else if (event == EventType::LShiftUp) {
                seenShiftUp = true;
                d.suppress = true;
            } else {
                currentState = State::IDLE;
            }

            if (seenWinUp && seenShiftUp) {
                currentState = State::IDLE;
            }
            return d;

        default:
            // Emergency fallback
            currentState = State::IDLE;
            seenWinUp = false;
            seenShiftUp = false;
            d.stopTimer = true;
            d.clearBuffer = true;
            d.injectCtrlUp = true;
            return d;
    }
}