#!/bin/bash
set -euo pipefail
PREFIX=${PREFIX:-/usr/local}
echo "Removing ARWM..."
rm -f "$PREFIX/bin/arwm"
rm -f "$PREFIX/bin/arwm-msg"
rm -f "/usr/share/xsessions/arwm.desktop"
rm -rf "/etc/arwm"
echo "Uninstall complete."
