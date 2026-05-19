# OrbitalWM

A unique X11 window manager where windows orbit around a central point in a pseudo-3D "open world" environment.

## Features

- **Orbital Layout**: Windows are arranged in a circular orbit.
- **Pseudo-3D Projection**: Windows scale and shift based on their position in the orbit, creating a sense of depth.
- **Smooth Animations**: Camera rotation and window movements are smooth thanks to interpolation.
- **Orbit Control**: Use the mouse to rotate the camera around the orbit.
- **Window Management**: Drag windows to move them manually, or let them snap back to their orbital positions.

## Controls

- **Mod4 (Super) + Left Click Drag**: Move a window manually.
- **Right Click Drag (on desktop)**: Rotate the camera (Orbit).
- **Mod4 + Q**: Close the focused window.
- **Mod4 + Tab**: Focus next window and bring it to front.
- **Mod4 + R**: Redistribute windows evenly across the orbit.
- **Mod4 + E**: Reset camera view.

## Configuration

Settings like radius, colors, and speeds can be adjusted in `config.h`.

## Build

```bash
gcc main.c -o orbitalwm -lX11 -lm
```
