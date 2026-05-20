#!/bin/bash
set -euo pipefail
PREFIX=${PREFIX:-/usr/local}
COLORS_GREEN='\033[0;32m'
COLORS_RESET='\033[0m'
log() { echo -e "${COLORS_GREEN}[ARWM]${COLORS_RESET} $1"; }
log "Starting installation..."
make clean
make
log "Installing binaries..."
install -Dm755 arwm "$PREFIX/bin/arwm"
install -Dm755 arwm-msg "$PREFIX/bin/arwm-msg"
log "Installing desktop session..."
install -Dm644 share/xsessions/arwm.desktop "/usr/share/xsessions/arwm.desktop"
log "Creating configuration directories..."
mkdir -p /etc/arwm
install -Dm644 config/default.conf /etc/arwm/default.conf
install -Dm755 config/autostart.sh /etc/arwm/autostart.sh
log "Installation complete."
