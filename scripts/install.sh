#!/bin/bash
set -euo pipefail

PREFIX=${PREFIX:-/usr/local}

echo "[ARWM] Compiling..."
cargo build --release

echo "[ARWM] Installing binary..."
sudo install -Dm755 target/release/arwm "$PREFIX/bin/arwm"

echo "[ARWM] Installing desktop entry..."
sudo install -Dm644 share/xsessions/arwm.desktop /usr/share/xsessions/arwm.desktop

echo "[ARWM] Success."
