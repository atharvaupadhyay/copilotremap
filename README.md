# Copilot Remap

If you have a new keyboard with a Copilot key and just want your Ctrl key back, this is for you.

This is a tiny, invisible background app that turns the Copilot key into a normal `Left Ctrl` key. It's lightweight, fast, and stays completely out of your way.

## How to download and use

You don't need to install anything, and there are no settings to configure.

1. Download the latest version by clicking **[here](https://github.com/atharvaupadhyay/copilotremap/releases/latest/download/CopilotRemap.exe)**.
2. Double-click the downloaded file. 
3. That's it. Nothing will pop up on your screen because the app runs invisibly in the background. Your Copilot key is now a Ctrl key.

### Make it run on startup
If you want this running all the time, you can set it to start with Windows automatically:
1. Press `Win + R` on your keyboard to open the Run dialog.
2. Type `shell:startup` and press Enter. This will open your Startup folder.
3. Right-click the `CopilotRemap.exe` file you downloaded and select "Create shortcut".
4. Drag that new shortcut into the Startup folder. 

### How to stop the app
Because the app runs invisibly to save resources, there is no system tray icon or exit button. If you ever want to stop it:
1. Press `Ctrl + Shift + Esc` to open Task Manager.
2. Go to the Details or Processes tab.
3. Find `CopilotRemap.exe`, right-click it, and select "End Task".

## How does it work?

Fun fact: the Copilot key isn't actually a single key. 

When you press it, your physical keyboard actually fires off a very fast, hardcoded combination of keys: `Left Windows` + `Left Shift` + `F23`. 

Because it's a combination of keys rather than a single key, normal key-remapping tools (like PowerToys) struggle to remap it properly without breaking your regular Shift or Windows keys. 

Copilot Remap is custom-built specifically to listen for this exact sequence. When you press the Copilot key, this app jumps in before Windows can see it, swallows the secret combo, and tells Windows that you simply pressed `Left Ctrl` instead. 

## Technical details

For developers wondering how it's built:

- **Lightweight:** Written in pure Win32 C++20. No heavy frameworks, no .NET, no Node.js. Just raw Windows API.
- **Zero latency:** It uses a highly optimized `WH_KEYBOARD_LL` hook with a custom deterministic state machine. There is no perceptible latency during normal use.
- **Flawless state management:** It properly tracks out-of-order modifier releases (`LSHIFT_UP`, `LWIN_UP`) without them leaking into Windows, preventing stuck modifiers or accidental Start Menu triggers.
- **Zero CPU & RAM:** Purely event-driven with zero background polling and zero heap allocations during the event loop.

### Building from source

If you want to compile it yourself:
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```
This generates `CopilotRemap.exe` in your build folder.
