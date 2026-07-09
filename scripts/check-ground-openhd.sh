#!/usr/bin/env bash
set -euo pipefail

printf '%s\n' '--- iw dev ---'
iw dev
printf '\n%s\n' '--- iwconfig ---'
iwconfig 2>/dev/null || true
printf '\n%s\n' '--- active connections ---'
nmcli -t -f NAME,DEVICE,TYPE connection show --active
printf '\n%s\n' '--- UDP sockets ---'
ss -ulpn | grep -E '14550|14551|5600|58' || true
printf '\n%s\n' '--- txrx.key ---'
sha256sum /usr/local/share/openhd/txrx.key 2>/dev/null || true
