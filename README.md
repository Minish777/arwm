# ARWM (Another Robust Window Manager)

[English](#english) | [Русский](#русский)

---

## English

ARWM is a high-performance, minimal tiling window manager for X11. Designed for real-world use on physical hardware, it combines the speed of pure C and Xlib with the logical layout of Binary Space Partitioning (BSP).

### ✨ Key Features
- **BSP Tiling**: Automatic window placement in a balanced binary tree.
- **Ultra-Lightweight**: Consumes < 5MB of RAM, zero CPU overhead.
- **Production Ready**: Optimized for stability, handles focus and borders correctly.
- **Customizable**: Live-reload configuration for gaps, borders, and colors.
- **Modern Look**: One Dark inspired default theme with elegant window borders.

### 🚀 Production Installation Guide

#### 1. Install Dependencies
**Debian / Ubuntu / Kali / Mint:**
```bash
sudo apt update
sudo apt install build-essential libx11-dev x11-utils dmenu xterm
```

**Arch Linux:**
```bash
sudo pacman -S base-devel libx11 xorg-utils dmenu xterm
```

**Fedora:**
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install libX11-devel xset dmenu xterm
```

#### 2. Build and Install
```bash
git clone https://github.com/yourusername/arwm.git
cd arwm
make
sudo make install
```

#### 3. Setup Session
ARWM installs a desktop entry, so you can select it directly from your login manager (GDM, LightDM, SDDM).

If you use `startx`, add this to your `~/.xinitrc`:
```bash
# Optional: composite manager for transparency/shadows
picom &
# Optional: wallpaper
feh --bg-fill /path/to/wallpaper.jpg &
# Start ARWM
exec arwm
```

### ⌨️ Keybindings (Mod = Super/Win key)
- **Mod + Enter**: Open Terminal (`alacritty` or `xterm`)
- **Mod + D**: Open dmenu
- **Mod + B**: Open Browser (`firefox`)
- **Mod + H**: Prepare Vertical Split
- **Mod + V**: Prepare Horizontal Split
- **Mod + R**: Reload Configuration
- **Mod + Q**: Quit ARWM

---

## Русский

ARWM — это высокопроизводительный минималистичный тайловый оконный менеджер для X11. Разработан для реального использования на физическом железе, сочетает в себе скорость чистого C и Xlib с логичной структурой бинарного разбиения пространства (BSP).

### ✨ Основные особенности
- **Тайлинг BSP**: Автоматическое размещение окон в сбалансированном бинарном дереве.
- **Ультра-легкость**: Потребляет менее 5 МБ ОЗУ, нулевая нагрузка на процессор.
- **Готов к работе**: Оптимизирован для стабильности, корректно обрабатывает фокус и границы окон.
- **Настройка**: Конфигурация отступов, границ и цветов с поддержкой горячей перезагрузки.
- **Современный вид**: Тема по умолчанию в стиле One Dark с элегантными рамками окон.

### 🚀 Руководство по установке

#### 1. Установка зависимостей
**Debian / Ubuntu / Kali / Mint:**
```bash
sudo apt update
sudo apt install build-essential libx11-dev x11-utils dmenu xterm
```

**Arch Linux:**
```bash
sudo pacman -S base-devel libx11 xorg-utils dmenu xterm
```

**Fedora:**
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install libX11-devel xset dmenu xterm
```

#### 2. Сборка и установка
```bash
git clone https://github.com/yourusername/arwm.git
cd arwm
make
sudo make install
```

#### 3. Настройка сессии
ARWM устанавливает файл desktop-сессии, поэтому вы можете выбрать его прямо в окне входа в систему (GDM, LightDM, SDDM).

Если вы используете `startx`, добавьте в ваш `~/.xinitrc`:
```bash
# Опционально: композитный менеджер (прозрачность, тени)
picom &
# Опционально: обои
feh --bg-fill /путь/к/обоям.jpg &
# Запуск ARWM
exec arwm
```

### ⌨️ Горячие клавиши (Mod = Клавиша Super/Win)
- **Mod + Enter**: Открыть терминал (`alacritty` или `xterm`)
- **Mod + D**: Открыть dmenu
- **Mod + B**: Открыть браузер (`firefox`)
- **Mod + H**: Подготовить вертикальное разделение
- **Mod + V**: Подготовить горизонтальное разделение
- **Mod + R**: Перезагрузить конфигурацию
- **Mod + Q**: Выйти из ARWM
