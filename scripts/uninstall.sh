#!/bin/bash
set -euo pipefail

PREFIX=${PREFIX:-/usr/local}

echo "[ARWM] Uninstalling..."
sudo rm -f "$PREFIX/bin/arwm"
sudo rm -f /usr/share/xsessions/arwm.desktop

echo "[ARWM] Done."
