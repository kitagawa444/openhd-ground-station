#!/usr/bin/env bash
set -euo pipefail

CARD="${1:-wlx20e15d2bfa3c}"

sudo pkill openhd 2>/dev/null || true
sudo nmcli con delete ohd_wifi_hotspot 2>/dev/null || true
sudo nmcli dev set "$CARD" managed no || true
sudo ip link set "$CARD" down
sudo iw dev "$CARD" set type monitor
sudo ip link set "$CARD" up
sudo iwconfig "$CARD" channel 36

exec sudo /usr/local/bin/openhd -g
