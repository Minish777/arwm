# ARWM (Another Rust Window Manager)

![License](https://img.shields.io/badge/license-MIT-blue.svg)

ARWM is a minimal, modern tiling window manager for X11 written in **Rust** using the **XCB** library. It is designed to be lightweight, stable, and highly hackable while providing a smooth daily-driver experience.

## ✨ Features
- **BSP-like Tiling**: Intelligent window placement with a master-stack layout.
- **Floating Support**: Toggle windows between tiling and floating modes.
- **Fullscreen Mode**: Dedicated fullscreen support for focused work.
- **Gaps & Borders**: Aesthetic spacing and focus highlighting.
- **Modern Configuration**: Simple TOML-based configuration at `~/.config/arwm/arwm.toml`.
- **Autostart Support**: Scriptable startup via `~/.config/arwm/autostart.sh`.
- **Ultra-Lightweight**: Minimal resource footprint and fast startup.

## 🚀 Installation

### ⚡ Automatic (Bootstrap)
The easiest way to install ARWM on Arch, Debian, Fedora, or Void Linux is to use the provided bootstrap script. This will install all dependencies, compile the project, and set up the desktop session.

```bash
git clone https://github.com/Minish777/arwm.git
cd arwm
./scripts/bootstrap.sh
```

### 🛠️ Manual Installation
If you prefer to do it yourself:

1. **Compile**:
   ```bash
   cargo build --release
   ```

2. **Install**:
   ```bash
   sudo ./scripts/install.sh
   ```

## ⌨️ Keybindings (Mod = Super/Win key)
- **Mod + Enter**: Open Terminal (`alacritty`)
- **Mod + D**: Open Launcher (`dmenu`)
- **Mod + Q**: Close Window
- **Mod + Shift + Q**: Quit ARWM
- **Mod + F**: Toggle Fullscreen
- **Mod + Space**: Toggle Floating
- **Mod + H/J/K/L**: Focus Navigation

## ⚙️ Configuration
ARWM creates a default configuration file at `~/.config/arwm/arwm.toml` on first run.

Example `arwm.toml`:
```toml
terminal = "alacritty"
launcher = "dmenu_run"
gap = 10
border_width = 2
border_color_focus = 6401007
border_color_normal = 4080722
```

## 🖥️ Usage
### startx
Add the following to your `~/.xinitrc`:
```bash
exec arwm
```

### Display Managers
ARWM installs a `arwm.desktop` file to `/usr/share/xsessions/`, making it available in GDM, LightDM, SDDM, etc.

## 🔧 Troubleshooting
- **No Terminal?**: Ensure `alacritty` or the terminal defined in your config is installed.
- **Black Screen?**: This can happen if no applications are started. ARWM auto-spawns a terminal and runs your autostart script.
- **XCB Errors?**: Check that no other window manager is running.

## 📜 License
This project is licensed under the **MIT License**.
