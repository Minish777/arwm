# ARWM (Another Robust Window Manager)

ARWM is a modern, high-performance tiling window manager for X11 written in C17 using the XCB library.

## Features
- **BSP Tiling Engine**: Intelligent window placement.
- **EWMH/ICCCM Support**: Compatible with system bars and pagers.
- **IPC System**: Scriptable control via unix sockets.
- **Minimalist**: Pure C, lightweight, and fast.

## Installation

### Automatic (Bootstrap)
Run the bootstrap script on a fresh system to install dependencies, compile, and install ARWM:
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

## Configuration
Configs are located in `~/.config/arwm/config.conf`.

## Keybindings
- **Mod4 + Enter**: Terminal
- **Mod4 + D**: Launcher
- **Mod4 + Q**: Quit

## License
MIT
