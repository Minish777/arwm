# ARWM (Another Robust Window Manager) — Canvas Edition

[English](#english) | [Русский](#русский)

---

## English

ARWM is a high-performance tiling window manager for X11 that introduces a revolutionary **Virtual Canvas** concept. It allows applications to draw directly to a virtual device `/dev/canvas`, while ARWM handles the composition and isolation using Linux Namespaces.

### ✨ Revolutionary Features
- **Virtual Canvas**: Apps write drawing commands to a POSIX file, ARWM renders them natively.
- **Isolation**: Each app gets its own mount namespace, thinking it has full screen access.
- **Zero-Copy Intent**: Optimized event loop using `poll()` for ultra-low latency.
- **BSP Tiling**: Automatic window management in a balanced binary tree.
- **Lightweight**: Pure C and Xlib, minimal resource footprint.

### 🚀 Developer API: /dev/canvas
To draw on the canvas, an app writes binary commands:
1. **Clear**: `[0x00][color:4]`
2. **Rectangle**: `[0x01][x:2][y:2][w:2][h:2][color:4]`
3. **Point**: `[0x02][x:2][y:2][color:4]`

### 🛠️ Production Installation

#### Prerequisites
- Xlib headers (`libx11-dev`, `libX11-devel`).
- Linux Kernel with Namespace support (`CONFIG_NAMESPACES`).

#### Build
```bash
make
sudo make install
```

---

## Русский

ARWM — это высокопроизводительный тайловый оконный менеджер для X11, внедряющий революционную концепцию **Виртуального Холста (Virtual Canvas)**. Он позволяет приложениям рисовать напрямую в виртуальное устройство `/dev/canvas`, в то время как ARWM берет на себя композицию и изоляцию с помощью Linux Namespaces.

### ✨ Революционные особенности
- **Виртуальный Холст**: Приложения записывают команды рисования в POSIX-файл, ARWM отрисовывает их нативно.
- **Изоляция**: Каждое приложение получает свой namespace монтирования, считая, что оно имеет доступ ко всему экрану.
- **Низкая задержка**: Оптимизированный цикл событий на базе `poll()` для мгновенного отклика.
- **Тайлинг BSP**: Автоматическое управление окнами в сбалансированном бинарном дереве.
- **Легковесность**: Чистый C и Xlib, минимальное потребление ресурсов.

### 🚀 API для разработчиков: /dev/canvas
Чтобы рисовать на холсте, приложение записывает бинарные команды:
1. **Очистка**: `[0x00][color:4]`
2. **Прямоугольник**: `[0x01][x:2][y:2][w:2][h:2][color:4]`
3. **Точка**: `[0x02][x:2][y:2][color:4]`

### 🛠️ Руководство по установке

#### Требования
- Заголовочные файлы Xlib (`libx11-dev`, `libX11-devel`).
- Ядро Linux с поддержкой Namespace (`CONFIG_NAMESPACES`).

#### Сборка
```bash
make
sudo make install
```

#### Запуск с изоляцией
Для запуска приложения с доступом к виртуальному холсту ARWM использует `unshare`. Обычные приложения продолжают работать через стандартный протокол X11.

### ⌨️ Горячие клавиши (Mod = Super/Win)
- **Mod + Enter**: Открыть терминал
- **Mod + D**: Запустить dmenu
- **Mod + R**: Перезагрузить конфиг
- **Mod + Q**: Выйти из ARWM
