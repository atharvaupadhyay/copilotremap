# CtrlPilot

CtrlPilot is a high-performance, native Win32 C++ utility that remaps the Microsoft Copilot key to `Left Ctrl`.

## The Problem
Unlike standard keys, the modern Copilot key does not send a single hardware scancode. Instead, the keyboard controller injects a rapid, hardcoded sequence:
1. `LWIN Down`
2. `LSHIFT Down`
3. `F23 Down`

*(When released, it sends `F23 Up`, `LSHIFT Up`, `LWIN Up`)*

Because it sends real modifier keys (`Win` and `Shift`), intercepting it incorrectly can leak these modifiers to the OS, causing unwanted Start Menu popups or triggering other shortcuts (e.g., `Win+Shift+S` instead of `Ctrl+S`). Relying on generic remapping tools or simple timeout delays often introduces noticeable typing latency when you actually try to use the Windows key normally.

## The Solution
CtrlPilot solves this by utilizing a rigorous, zero-allocation, purely event-driven architecture running on a low-level keyboard hook (`WH_KEYBOARD_LL`). 

### Key Features
- **Event-Driven Resolving:** Normal typing isn't punished. If you press `Win` and then `R`, the utility instantly realizes it's not the Copilot sequence, flushes the buffered `Win` key, and processes the `R` key with absolutely **no perceptible latency**.
- **Atomic Batch Injection:** When replaying buffered keys, CtrlPilot uses a batched `SendInput` payload. This guarantees that interleaved hardware events (e.g. mashing the keyboard) cannot split the injected sequence, ensuring complete deterministic behavior under the Win32 input model.
- **Zero Heap Allocations:** The hot-path (`KeyboardProc`) allocates absolutely nothing on the heap. `std::vector` and dynamic sizing are completely banned; everything operates natively on `std::array` stack buffers.
- **OS Lifecycle Safety:** Includes a hidden message-only window that catches `WM_QUERYENDSESSION` and `WM_ENDSESSION`. If you hold the Copilot key while the system shuts down or the session ends, CtrlPilot will proactively inject a `Left Ctrl Up` event, guaranteeing no stuck keys.
- **Resource Footprint:** 0% CPU usage while idle, zero polling, and an imperceptible RAM footprint. 

## Architecture
The project strictly separates Win32 API calls from the logic:
1. **Pure C++ State Machine:** Evaluates a platform-agnostic `EventType` (`LWinDown`, `LShiftDown`, etc.) against its internal `State` (e.g. `PENDING_WIN`, `COPILOT_ACTIVE`). It returns a fixed-size `ActionList` of primitive instructions (e.g., `Suppress`, `InjectBuffered`, `ClearBuffer`).
2. **Win32 Hook Executor:** Converts OS events into the `EventType`, calls the State Machine, and then iterates over the returned `ActionList` to execute actual OS functions (`SendInput`, buffer manipulation, timer toggles).

## Building
CtrlPilot uses CMake and requires a C++20 compiler. It can be built with MSVC or MinGW/GCC.

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Compiler Optimizations
The `CMakeLists.txt` is pre-configured for rigorous development and highly optimized release binaries:
- Enforces strict warnings (`/W4`, `/WX` or equivalent).
- Generates Link-Time Optimizations (`/GL`, `/LTCG`).
- Eliminates unreferenced code and functions (`/OPT:REF`, `/OPT:ICF`, `/Gy`, `/Gw`).

## Usage
Simply run `CtrlPilot.exe`. It runs completely invisibly in the background. To exit, terminate the process via Task Manager. 
