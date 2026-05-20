#!/bin/bash
set -euo pipefail
log() { echo -e "\033[0;32m[BOOTSTRAP]\033[0m $1"; }
log "Detecting distribution..."
if [ -f /etc/arch-release ]; then
    log "Arch Linux detected."
    sudo pacman -S --needed --noconfirm base-devel libxcb xcb-util xcb-util-wm xcb-util-keysyms xcb-util-image xcb-util-cursor dmenu picom alacritty feh
elif [ -f /etc/debian_version ]; then
    log "Debian/Ubuntu detected."
    sudo apt update
    sudo apt install -y build-essential libxcb1-dev libxcb-ewmh-dev libxcb-icccm4-dev libxcb-keysyms1-dev libxcb-util0-dev dmenu picom alacritty feh
elif [ -f /etc/fedora-release ]; then
    log "Fedora detected."
    sudo dnf install -y gcc make pkg-config libxcb-devel xcb-util-wm-devel xcb-util-keysyms-devel xcb-util-devel dmenu picom alacritty feh
fi
log "Compiling and installing ARWM..."
sudo ./scripts/install.sh
log "Setting up local user config..."
mkdir -p ~/.config/arwm
cp -n /etc/arwm/default.conf ~/.config/arwm/config.conf || true
cp -n /etc/arwm/autostart.sh ~/.config/arwm/autostart.sh || true
chmod +x ~/.config/arwm/autostart.sh
log "Bootstrap complete! You can now log into ARWM from your display manager."
