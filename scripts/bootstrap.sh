#!/bin/bash
set -euo pipefail

echo "[ARWM] Detecting distribution..."
if [ -f /etc/arch-release ]; then
    sudo pacman -S --needed --noconfirm rust xorg-server xorg-xinit libxcb xcb-util xcb-util-wm alacritty dmenu picom feh
elif [ -f /etc/debian_version ]; then
    sudo apt update
    sudo apt install -y cargo rustc libxcb1-dev libxcb-util-dev libxcb-keysyms1-dev libxcb-icccm4-dev libxcb-ewmh-dev alacritty dmenu picom feh
elif [ -f /etc/fedora-release ]; then
    sudo dnf install -y gcc make pkg-config libxcb-devel xcb-util-wm-devel xcb-util-keysyms-devel xcb-util-devel alacritty dmenu picom feh
elif [ -f /etc/void-release ]; then
    sudo xbps-install -Sy rust cargo xorg-minimal xinit libxcb-devel xcb-util-devel xcb-util-wm-devel xcb-util-keysyms-devel alacritty dmenu picom feh
fi

./scripts/install.sh

echo "[ARWM] Bootstrap complete. Please log out and select ARWM."
