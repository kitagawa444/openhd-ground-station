# Work Log

## Scope

Install OpenHD ground on this Ubuntu 22.04 x86_64 device, configure the TP-Link `2357:0138` adapter for OpenHD monitor mode, verify local backend startup, and prepare QOpenHD.

## Actions Performed

### 1. Environment inspection

- Confirmed current user: `ros2deck`
- Confirmed host OS: Ubuntu 22.04.5 LTS
- Confirmed kernel: `6.8.0-111-generic`
- Confirmed USB Wi-Fi adapter: TP-Link `2357:0138`
- Confirmed the adapter was initially bound to `rtw88_8822bu`

### 2. Dependencies installed

Installed build and runtime packages for:

- DKMS
- GStreamer
- libpcap
- libsodium
- libusb
- V4L2
- NetworkManager
- OpenHD build dependencies
- QOpenHD package dependencies

### 3. OpenHD-specific Wi-Fi driver

- Cloned `rtl88x2bu-ohd`
- Built and installed DKMS package `rtl88x2bu/5.13.1-git`
- Added blacklist file for competing `rtw88` and generic `88x2bu` modules
- Rebound the TP-Link adapter to `rtl88x2bu_ohd`

Verification:

- `dkms status` shows the module as installed
- `lsusb -t` showed `Driver=rtl88x2bu_ohd`
- `iwconfig` showed `Nickname:"rtl88x2bu_ohd"`

### 4. OpenHD-SysUtils

- Cloned `OpenHD-SysUtils`
- Built `openhd_sys_utils`
- Installed it to `/usr/local/bin/openhd_sys_utils`
- Created and enabled `openhd-sys-utils.service`

Verification:

- service state: `active`
- socket created: `/run/openhd/openhd_sys.sock`

### 5. OpenHD backend

- Cloned `OpenHD` on branch `2.7-evo`
- Pulled submodules
- Installed missing GStreamer development headers
- Built `openhd`
- Installed binary to `/usr/local/bin/openhd`

### 6. Ground configuration

Created and applied:

- `hardware.conf`
- `networking_settings.json`
- `wifibroadcast_settings.json`
- `openhd-sys-utils.service`
- helper start and check scripts

Ground-specific choices:

- `WIFI_WB_LINK_CARDS=wlx20e15d2bfa3c`
- hotspot disabled via `wifi_hotspot_mode=1`
- frequency fixed to `5180 MHz`
- channel width fixed to `20 MHz`
- network forwarding to localhost kept enabled

### 7. QOpenHD

- Added OpenHD Cloudsmith repository
- Installed `QOpenHD`
- Initially received a `3.0-alpha` package
- Downgraded it to `2.7.1-03-18-2026--21-41-37-fee63eb4` to match the OpenHD 2.7.x backend generation

## Verification Results

### Backend startup

Ground backend was started with the adapter manually switched to monitor mode and channel 36.

Observed in [logs/openhd_ground_live.log](../logs/openhd_ground_live.log):

- banner `2.7.1-evo`
- `Ground Unit`
- `OpenHD was successfully started.`

### Interface state during verification

Observed during runtime:

- interface `wlx20e15d2bfa3c`
- mode `monitor`
- channel `36`
- frequency `5180 MHz`

### GUI limitation observed

`QOpenHD` does not launch from a plain `tty` shell because `DISPLAY` is unset there. It must be started from a desktop GUI session.

## Remaining Steps

### Air link test

Still needed:

- boot Air side
- compare `txrx.key` hashes
- start Air backend with matching channel and monitor mode
- launch `QOpenHD` from GUI and confirm telemetry and video

### Optional cleanup

- move helper scripts from `~/openhd_setup/` entirely to this repository path
- commit this repo after review

## UI Architecture Follow-Up

### 8. Ground UI v0 architecture draft

Created [ui-architecture-v0.md](ui-architecture-v0.md) to turn the initial ground-station UI idea into a buildable Ubuntu 22.04 plan.

Key decisions captured there:

- keep OpenHD integration behind a small backend adapter
- split large manager-style classes into focused services
- expose read-only QML models for vehicle, link, video, alerts, and settings
- route write actions through narrow command objects instead of direct manager calls
- keep v0 focused on one strong flight screen before mission or RC-heavy features

This was done to reduce early coupling between protocol logic, runtime state, and QML bindings.

### 9. Ground UI prototype scaffold

Created a new `ground_ui/` Qt6 project inside this repository.

Implemented:

- CMake-based Qt6 application scaffold
- layered `services -> models -> commands -> QML` structure
- dummy telemetry, link, video, and alert feeds via `DemoDataService`
- first-pass `FlightScreen.qml` with HUD, status bar, minimap, action strip, and warning banner
- demo operator actions for arm, mode cycle, record toggle, video restart, and reconnect

Installed additional UI dependencies on Ubuntu 22.04:

- `qt6-base-dev`
- `qt6-declarative-dev`
- `qml6-module-qtquick`
- `qml6-module-qtquick-controls`
- `qml6-module-qtquick-layouts`
- `qml6-module-qtquick-window`
- `qml6-module-qtqml-workerscript`
- `qml6-module-qtquick-templates`

Verification:

- `cmake -S ground_ui -B ground_ui/build`
- `cmake --build ground_ui/build -j"$(nproc)"`
- `QT_QPA_PLATFORM=offscreen ./ground_ui/build/openhd_ground_ui`

The offscreen launch stayed alive until `timeout` stopped it, which confirms the QML scene loaded and the event loop started without runtime errors.

### 10. Live OpenHD UI integration

Replaced the default dummy-only behavior with a live, read-only `OpenHDBackend`.

Implemented:

- a resilient MAVLink v1/v2 TCP stream parser
- automatic connection and reconnection to OpenHD Ground's multi-client TCP server on `127.0.0.1:5760`
- vehicle updates from standard MAVLink: heartbeat, system/battery status, GPS, global position, VFR HUD, and home position
- OpenHD-specific link, video, and camera-stat message handling
- UDP `5800` RTP decoding into a thread-safe QML image provider, kept separate from QOpenHD's normal UDP `5600` consumer path
- live-mode read-only UI behavior so demo buttons cannot change displayed vehicle state
- optional `--demo` mode for synthetic UI development

Verification:

- `ctest --test-dir ground_ui/build --output-on-failure`
- MAVLink parser test passed
- TCP backend integration test passed, including flight mode, arm state, GPS, battery, camera resolution, frame rate, and codec mapping
- offscreen live launch remained healthy until stopped by `timeout`

Live Air-link verification is pending physical reconnection of the TP-Link `2357:0138` OpenHD adapter. At this point the host only exposes its internal `wlp6s0`; the prior monitor-mode interface `wlx20e15d2bfa3c` and its USB device are absent, so `start-ground-openhd.sh` correctly cannot start the radio backend.

### 11. Live RTP decode verification

With the TP-Link adapter reconnected, OpenHD Ground started on channel 36 and the Air video stream was observed on both local forwarding paths:

- QOpenHD consumer: `127.0.0.1:5600`
- new ground UI consumer: `127.0.0.1:5800`

Packet capture showed continuous 1440-byte RTP packets arriving on both ports. The pipeline below received and decoded a real H.264 frame successfully:

```bash
gst-launch-1.0 udpsrc address=127.0.0.1 port=5800 \
  caps='application/x-rtp,media=video,encoding-name=H264,payload=96' \
  ! rtpjitterbuffer latency=0 drop-on-latency=true \
  ! rtph264depay ! h264parse ! avdec_h264 ! identity eos-after=1 ! fakesink
```

Installed `gstreamer1.0-libav` to provide the H.264/H.265 software decoders used by the Qt6 UI. The actual OpenHD MAVLink TCP endpoint was also verified as `5760`; the old `1445` comment in OpenHD source is stale.
