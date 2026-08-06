#include "state_machine.h"

void StateMachine::Reset() {
    currentState = State::IDLE;
    ctrlInjected = false;
}

bool StateMachine::IsCtrlInjected() const {
    return ctrlInjected;
}

ActionList StateMachine::ProcessEvent(EventType event) {
    ActionList list;

    switch (currentState) {
        case State::IDLE:
            if (event == EventType::LWinDown) {
                currentState = State::PENDING_WIN;
                list.add(Action::Suppress);
                list.add(Action::BufferCurrent);
                list.add(Action::StartTimer);
                return list;
            }
            list.add(Action::Pass);
            return list;

        case State::PENDING_WIN:
            if (event == EventType::LShiftDown) {
                currentState = State::PENDING_WIN_SHIFT;
                list.add(Action::Suppress);
                list.add(Action::BufferCurrent);
                list.add(Action::StartTimer);
                return list;
            }
            if (event == EventType::Timeout) {
                currentState = State::IDLE;
                list.add(Action::StopTimer);
                list.add(Action::InjectBuffered);
                list.add(Action::ClearBuffer);
                return list;
            }
            
            currentState = State::IDLE;
            list.add(Action::StopTimer);
            list.add(Action::InjectBuffered);
            list.add(Action::ClearBuffer);
            list.add(Action::Pass);
            return list;

        case State::PENDING_WIN_SHIFT:
            if (event == EventType::F23Down) {
                currentState = State::COPILOT_ACTIVE;
                ctrlInjected = true;
                list.add(Action::StopTimer);
                list.add(Action::ClearBuffer);
                list.add(Action::Suppress);
                list.add(Action::InjectCtrlDown);
                return list;
            }
            if (event == EventType::Timeout) {
                currentState = State::IDLE;
                list.add(Action::StopTimer);
                list.add(Action::InjectBuffered);
                list.add(Action::ClearBuffer);
                return list;
            }
            
            currentState = State::IDLE;
            list.add(Action::StopTimer);
            list.add(Action::InjectBuffered);
            list.add(Action::ClearBuffer);
            list.add(Action::Pass);
            return list;

        case State::COPILOT_ACTIVE:
            if (event == EventType::F23Down || event == EventType::LWinDown || event == EventType::LShiftDown) {
                list.add(Action::Suppress);
                return list;
            }
            if (event == EventType::LWinUp || event == EventType::LShiftUp) {
                // Do not release on the first modifier-up event.
                list.add(Action::Suppress);
                return list;
            }
            if (event == EventType::F23Up) {
                currentState = State::IDLE;
                ctrlInjected = false;
                list.add(Action::InjectCtrlUp);
                list.add(Action::Suppress);
                return list;
            }
            list.add(Action::Pass);
            return list;
    }

    list.add(Action::Pass);
    return list;
}
