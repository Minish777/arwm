# ARWM (Another Robust Window Manager)

[English](#english) | [Русский](#русский)

---

## English

ARWM is a minimal, production-ready tiling window manager for X11, written in C using only Xlib. It uses Binary Space Partitioning (BSP) to manage window layouts, similar to bspwm, but with a focus on extreme simplicity and hackability.

### ✨ Features
- **BSP Tiling**: Windows are organized in a binary tree.
- **Lightweight**: Minimal line count, lightning-fast performance.
- **Configurable**: Gaps, border widths, and colors can be tuned via `~/.config/arwm/config.conf`.
- **Robust**: Safe process spawning, graceful X error handling.
- **Bilingual Documentation**: Support for multiple languages.

### 🚀 Getting Started

#### Prerequisites
- Xlib headers (`libx11-dev` on Debian/Ubuntu, `libX11-devel` on Fedora).
- A terminal emulator (defaults to `alacritty`, falls back to `xterm`).

#### Build & Install
```bash
make
sudo make install
```

#### Running
Add the following to your `.xinitrc`:
```bash
exec arwm
```

### ⌨️ Keybindings (Mod = Super/Win key)
- **Mod + Enter**: Spawn Terminal
- **Mod + D**: Spawn dmenu
- **Mod + B**: Spawn Firefox
- **Mod + Q**: Quit ARWM
- **Mod + H**: Vertical Split
- **Mod + V**: Horizontal Split
- **Mod + R**: Reload Configuration

---

## Русский

ARWM — это минималистичный и готовый к работе тайловый оконный менеджер для X11, написанный на C с использованием только Xlib. Он использует бинарное разбиение пространства (BSP) для управления окнами, аналогично bspwm, но ориентирован на предельную простоту и легкость модификации.

### ✨ Особенности
- **Тайлинг BSP**: Окна организованы в бинарное дерево.
- **Легковесность**: Минимальное количество строк кода, молниеносная работа.
- **Настройка**: Отступы (gaps), ширина границ и цвета настраиваются через `~/.config/arwm/config.conf`.
- **Надежность**: Безопасный запуск процессов, корректная обработка ошибок X.
- **Двуязычная документация**: Полная поддержка русского и английского языков.

### 🚀 Начало работы

#### Требования
- Заголовочные файлы Xlib (`libx11-dev` в Debian/Ubuntu, `libX11-devel` в Fedora).
- Эмулятор терминала (по умолчанию `alacritty`, запасной вариант — `xterm`).

#### Сборка и установка
```bash
make
sudo make install
```

#### Запуск
Добавьте следующее в ваш `.xinitrc`:
```bash
exec arwm
```

### ⌨️ Горячие клавиши (Mod = Клавиша Super/Win)
- **Mod + Enter**: Открыть терминал
- **Mod + D**: Запустить dmenu
- **Mod + B**: Запустить Firefox
- **Mod + Q**: Выйти из ARWM
- **Mod + H**: Вертикальное разделение
- **Mod + V**: Горизонтальное разделение
- **Mod + R**: Перезагрузить конфигурацию
