#include "state_machine.h"

void StateMachine::Reset() {
    currentState = State::IDLE;
    physicalLWinDown = false;
    physicalLShiftDown = false;
    copilotActive = false;
    syntheticCtrlDown = false;
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

    // Update physical state independently
    if (event == EventType::LWinDown) physicalLWinDown = true;
    else if (event == EventType::LWinUp) physicalLWinDown = false;
    else if (event == EventType::LShiftDown) physicalLShiftDown = true;
    else if (event == EventType::LShiftUp) physicalLShiftDown = false;

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
            if (event == EventType::Timeout) {
                currentState = State::IDLE;
                d.stopTimer = true;
                d.replayBuffer = true;
                d.clearBuffer = true;
                return d;
            }
            
            currentState = State::IDLE;
            d.stopTimer = true;
            d.bufferCurrent = true;
            d.suppress = true;
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
            if (event == EventType::Timeout) {
                currentState = State::IDLE;
                d.stopTimer = true;
                d.replayBuffer = true;
                d.clearBuffer = true;
                return d;
            }
            
            currentState = State::IDLE;
            d.stopTimer = true;
            d.bufferCurrent = true;
            d.suppress = true;
            d.replayBuffer = true;
            d.clearBuffer = true;
            return d;

        case State::PENDING_BOTH:
            if (event == EventType::F23Down) {
                currentState = State::COPILOT_ACTIVE;
                copilotActive = true;
                d.stopTimer = true;
                d.clearBuffer = true;
                d.suppress = true;
                d.injectCtrlDown = true;
                syntheticCtrlDown = true;
                return d;
            }
            if (event == EventType::Timeout) {
                currentState = State::IDLE;
                d.stopTimer = true;
                d.replayBuffer = true;
                d.clearBuffer = true;
                return d;
            }
            
            currentState = State::IDLE;
            d.stopTimer = true;
            d.bufferCurrent = true;
            d.suppress = true;
            d.replayBuffer = true;
            d.clearBuffer = true;
            return d;

        case State::COPILOT_ACTIVE:
            if (event == EventType::Timeout) {
                return d;
            }
            if (event == EventType::F23Down || event == EventType::LWinDown || event == EventType::LShiftDown ||
                event == EventType::LWinUp || event == EventType::LShiftUp) {
                d.suppress = true;
                return d;
            }
            if (event == EventType::F23Up) {
                d.suppress = true;
                d.injectCtrlUp = true;
                syntheticCtrlDown = false;
                copilotActive = false;

                if (!physicalLWinDown && !physicalLShiftDown) {
                    currentState = State::IDLE;
                } else {
                    currentState = State::WAIT_RELEASE;
                }
                return d;
            }
            return d;

        case State::WAIT_RELEASE:
            if (event == EventType::Timeout) {
                return d;
            }
            if (event == EventType::LWinUp || event == EventType::LShiftUp || 
                event == EventType::LWinDown || event == EventType::LShiftDown) {
                d.suppress = true;
            }

            if (!physicalLWinDown && !physicalLShiftDown) {
                currentState = State::IDLE;
            }
            return d;

        default:
            currentState = State::IDLE;
            d.stopTimer = true;
            d.clearBuffer = true;
            if (syntheticCtrlDown) {
                d.injectCtrlUp = true;
                syntheticCtrlDown = false;
            }
            copilotActive = false;
            return d;
    }
}