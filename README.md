# ARWM (Another Robust Window Manager)

ARWM is a modern, professional-grade tiling window manager for X11 written in C17 using the XCB library. It is designed to be lightweight, stable, and highly hackable.

## Features
- **BSP Tiling Engine**: Recursive binary space partitioning for intelligent window placement.
- **EWMH/ICCCM Compliant**: Seamless integration with bars like Polybar and launchers like Rofi.
- **Unix Socket IPC**: Control your window manager from scripts or the command line.
- **Floating Support**: Toggle individual windows between tiled and floating modes.
- **Distro-Aware Setup**: Automated bootstrap script for major Linux distributions.

## Installation

### Automatic (Recommended)
The bootstrap script installs dependencies, compiles, and sets up a default session:
```bash
git clone https://github.com/Minish777/arwm.git
cd arwm
./scripts/bootstrap.sh
```

### Manual
```bash
make
sudo make install
```

## Quick Start
1. Log out of your current session.
2. Select **ARWM** from your display manager (GDM, LightDM, SDDM, etc.).
3. Default keys:
   - **Mod4 + Enter**: Terminal (`alacritty` or `xterm`)
   - **Mod4 + D**: Launcher (`dmenu_run`)
   - **Mod4 + Shift + C**: Close Window
   - **Mod4 + Shift + Q**: Quit WM

## Configuration
Configuration is stored in `~/.config/arwm/config.conf`. You can reload it without restarting the WM:
```bash
arwm-msg reload
```

## License
MIT
