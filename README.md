# ARWM (Another Robust Window Manager) — Ultimate Edition

[English](#english) | [Русский](#русский)

---

## English

ARWM is a highly optimized, production-ready tiling window manager for X11. It adheres to **EWMH/ICCCM** standards and provides a modern, "native-like" experience with mouse management, IPC control, and a robust BSP layout.

### ✨ Master-Level Features
- **EWMH Standards**: Full compatibility with Polybar, Tint2, and other system panels.
- **Mouse Management**: Mod + Left Click to move, Mod + Right Click to resize windows.
- **IPC Control**: Control ARWM via Unix Socket (`/tmp/arwm.sock`) using simple CLI commands.
- **Floating/Tiling**: Toggle windows between tiled and floating mode with ease.
- **Auto-Detection**: Recognizes dialog windows and opens them in floating mode automatically.

### 🚀 Production Installation Guide

#### 1. Dependencies
```bash
# Debian / Ubuntu / Kali / Mint:
sudo apt update
sudo apt install build-essential libx11-dev x11-utils dmenu feh picom
# Arch Linux:
sudo pacman -S base-devel libx11 xorg-utils dmenu feh picom
# Fedora:
sudo dnf groupinstall "Development Tools"
sudo dnf install libX11-devel xset dmenu feh picom
```

#### 2. Build and Install
```bash
git clone https://github.com/yourusername/arwm.git
cd arwm
make
sudo make install
```

#### 3. Professional Setup
ARWM installs a `.desktop` entry for login managers. To configure your session, create `~/.config/arwm/autostart.sh`:
```bash
#!/bin/sh
# Composite manager for transparency/shadows
picom &
# Set wallpaper
feh --bg-fill /path/to/wallpaper.jpg &
# System bar
polybar main &
```
`chmod +x ~/.config/arwm/autostart.sh`

### ⌨️ Default Keybindings (Mod = Super/Win key)
- **Mod + Enter**: Terminal (`alacritty` -> `xterm`)
- **Mod + D**: dmenu_run
- **Mod + B**: Firefox Browser
- **Mod + F**: File Manager
- **Mod + Tab**: Cycle Focus
- **Mod + Space**: Toggle Floating/Tiled Mode
- **Mod + H**: Vertical Split Prep
- **Mod + V**: Horizontal Split Prep
- **Mod + R**: Live Reload Config
- **Mod + Q**: Quit ARWM

---

## Русский

ARWM — это высокооптимизированный тайловый оконный менеджер для X11, готовый к промышленному использованию. Он полностью соответствует стандартам **EWMH/ICCCM** и предоставляет современный пользовательский опыт с управлением мышью, IPC-контролем и надежной структурой BSP.

### ✨ Ключевые возможности
- **Стандарты EWMH**: Полная совместимость с Polybar, Tint2 и другими системными панелями.
- **Управление мышью**: Mod + ЛКМ для перемещения, Mod + ПКМ для изменения размера окон.
- **Управление через IPC**: Управляйте ARWM через Unix-сокет (`/tmp/arwm.sock`) простыми командами.
- **Тайлинг и плавающие окна**: Мгновенное переключение между режимами для любого окна.
- **Авто-определение**: Автоматически открывает диалоговые окна в плавающем режиме.

### 🚀 Руководство по установке

#### 1. Установка зависимостей
```bash
# Debian / Ubuntu:
sudo apt install build-essential libx11-dev x11-utils dmenu feh picom
# Arch Linux:
sudo pacman -S base-devel libx11 xorg-utils dmenu feh picom
```

#### 2. Сборка и установка
```bash
make
sudo make install
```

#### 3. Профессиональная настройка
ARWM устанавливает файл сессии для менеджеров входа. Создайте файл `~/.config/arwm/autostart.sh`:
```bash
#!/bin/sh
picom &
feh --bg-fill /путь/к/обоям.jpg &
polybar main &
```
Не забудьте `chmod +x ~/.config/arwm/autostart.sh`.

### ⌨️ Горячие клавиши (Mod = Клавиша Super/Win)
- **Mod + Enter**: Открыть терминал
- **Mod + D**: Запустить dmenu
- **Mod + B**: Открыть браузер
- **Mod + F**: Файловый менеджер
- **Mod + Tab**: Переключить фокус
- **Mod + Space**: Переключить режим (Плавающий/Тайловый)
- **Mod + H**: Подготовить вертикальное разделение
- **Mod + V**: Подготовить горизонтальное разделение
- **Mod + R**: Перезагрузить конфиг
- **Mod + Q**: Выход из ARWM
