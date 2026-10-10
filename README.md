<div align="center">

# ⌨️🔒 Keyboard Lock

**Clean your keyboard without typing gibberish.**

A tiny Windows app that blocks *every* key press while it's open, so you can wipe, brush, and vacuum your keyboard without accidentally sending messages, closing windows, or triggering shortcuts.

![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-0078D6?logo=windows&logoColor=white)
![Language](https://img.shields.io/badge/language-C-A8B9CC?logo=c&logoColor=white)
![Size](https://img.shields.io/badge/size-~80%20KB-success)
![Dependencies](https://img.shields.io/badge/dependencies-none-brightgreen)
![License](https://img.shields.io/badge/license-MIT-blue)

</div>

---

## ✨ Features

- 🚫 **Blocks all keys**, including `Win`, `Alt+Tab`, `Alt+F4`, `Esc` and more
- 🖱️ **Unlock with the mouse**, one click on *Unlock and Exit*
- 🔢 **Live counter** showing how many key presses were blocked
- ⏱️ **Safety auto-unlock** after 15 minutes (configurable)
- 📌 **Always on top**, so you can't lose the window while cleaning
- 🪶 **Tiny and portable**: a single ~80 KB `.exe`, no installer, no dependencies
- 🖥️ **HiDPI aware** and uses modern Windows visual styles

## 🚀 Usage

1. Download `KeyboardLock.exe` from the [**Releases**](../../releases) page.
2. Double-click it. The keyboard is locked immediately. 🔒
3. Clean your keyboard to your heart's content. 🧽
4. Click **Unlock and Exit** with your mouse. 🔓

### ⚙️ Command line

```
KeyboardLock.exe [minutes]
```

| Command                | Behavior                                  |
| ---------------------- | ----------------------------------------- |
| `KeyboardLock.exe`     | Auto-unlock after the default 15 minutes  |
| `KeyboardLock.exe 5`   | Auto-unlock after 5 minutes               |
| `KeyboardLock.exe 0`   | No timeout, unlock manually with the mouse |

## ⚠️ Good to know

- 🛡️ **`Ctrl+Alt+Del` cannot be blocked** by any program. This is a Windows security feature and works as your emergency exit: open Task Manager from there and end *Keyboard Lock*.
- 💻 Laptop **power buttons** and some **Fn hardware keys** are handled by firmware and can't be blocked in software. The touchpad and mouse keep working.
- 👑 Windows running as **administrator** (elevated windows) may still receive keystrokes. If that matters, run Keyboard Lock as administrator too.
- 🔔 The `.exe` is **unsigned** and uses a keyboard hook, so **Windows SmartScreen** or some antivirus tools may show a warning. Click *More info → Run anyway*, or build it yourself from source (below). The code is short and easy to review.

## 🔧 How it works

The app installs a [low-level keyboard hook](https://learn.microsoft.com/en-us/windows/win32/winmsg/about-hooks) (`WH_KEYBOARD_LL`) and returns a non-zero value from the hook procedure for every event, which tells Windows to drop the key press before any application sees it. When the app exits, the hook is removed and your keyboard works normally again.

No data is recorded, stored, or sent anywhere. The only thing the app keeps is a counter of blocked key presses, shown in the window.

## 🛠️ Build from source

You only need a C compiler that can target Windows. The easiest cross-platform option is [Zig](https://ziglang.org/), which works on Windows, Linux, and macOS:

```bash
pip install ziglang
python -m ziglang cc -target x86_64-windows-gnu -O2 -s -Wl,--subsystem,windows KeyboardLock.c app.rc -o KeyboardLock.exe -luser32 -lgdi32 -lcomctl32
```

| File               | Purpose                                              |
| ------------------ | ---------------------------------------------------- |
| `KeyboardLock.c`   | The entire application                               |
| `app.manifest`     | Enables modern visual styles and HiDPI support       |
| `app.rc`           | Embeds the manifest into the executable              |

## 🤝 Contributing

Issues and pull requests are welcome! Ideas: custom timeout in the UI, a tray icon, a "lock mouse too" option, translations. 🌍

## 📄 License

Released under the [MIT License](LICENSE).

---

<div align="center">

Made with ☕ for everyone who has ever spilled crumbs into their keyboard.

</div>
