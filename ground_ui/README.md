# ground_ui

Qt6/QML prototype for a future OpenHD ground station UI on Ubuntu 22.04.

This first implementation is intentionally narrow:

- layered as `services -> models -> commands -> QML`
- live telemetry from the local OpenHD Ground backend
- focused on a single flight screen

## Requirements

- Ubuntu 22.04
- CMake 3.22+
- Qt6 base and declarative development packages

Packages installed on this machine:

```bash
sudo apt-get install -y \
  qt6-base-dev \
  qt6-declarative-dev \
  qml6-module-qtquick \
  qml6-module-qtquick-controls \
  qml6-module-qtquick-layouts \
  qml6-module-qtquick-templates \
  qml6-module-qtquick-window \
  qml6-module-qtqml-workerscript
```

## Build

```bash
cmake -S ground_ui -B ground_ui/build
cmake --build ground_ui/build -j"$(nproc)"
```

## Run

From a desktop GUI session:

```bash
./ground_ui/build/openhd_ground_ui
```

The default mode is live read-only integration. Start OpenHD Ground first:

```bash
~/openhd-ground-station/scripts/start-ground-openhd.sh
```

Use `--demo` only for UI development without an OpenHD backend:

```bash
./ground_ui/build/openhd_ground_ui --demo
```

Headless smoke test:

```bash
QT_QPA_PLATFORM=offscreen ./ground_ui/build/openhd_ground_ui
```

## Current Scope

- connects to OpenHD's multi-client MAVLink TCP server at `127.0.0.1:5760`
- decodes MAVLink vehicle state: flight mode, armed state, GPS, home distance, altitude, speed, heading, and battery
- decodes OpenHD link and camera messages: receive quality, packet loss, received video bitrate, codec, resolution, and frame rate
- decodes the extra OpenHD RTP forwarding path at `127.0.0.1:5800` into the Flight Screen background
- operator-style flight layout
- live mode is deliberately read-only; no arm, RC, recording, or backend control command is sent
- `--demo` retains the synthetic data and demo actions for layout work

The primary stream supports H.264, H.265, and MJPEG RTP. Codec selection follows `OPENHD_CAMERA_STATUS_AIR` metadata once the telemetry connection is established.

## Verify

```bash
ctest --test-dir ground_ui/build --output-on-failure
QT_QPA_PLATFORM=offscreen ./ground_ui/build/openhd_ground_ui
```

The tests cover MAVLink v1/v2 framing and a local TCP integration that verifies the service updates produced by live-style OpenHD telemetry.
