# openhd-ground-station

OpenHD ground station setup for Ubuntu 22.04 without git submodules.

This repository tracks:

- local configuration files
- helper scripts
- install and verification notes
- ground-side UI architecture notes
- Codex work log for this machine

This repository does not vendor upstream source trees. Those are kept separately in `~/wifi/` and pinned by commit hash in [docs/versions.md](docs/versions.md).

## Host

- OS: Ubuntu 22.04.5 LTS
- Kernel: `6.8.0-111-generic`
- Architecture: `x86_64`
- OpenHD ground backend: `2.7.1-evo`
- QOpenHD: `2.7.1-03-18-2026--21-41-37-fee63eb4`
- USB Wi-Fi: TP-Link `2357:0138` with `rtl88x2bu_ohd`

## Layout

- [configs](configs): local configuration files used for this setup
- [scripts](scripts): helper scripts to start and inspect the ground station
- [logs](logs): captured runtime logs
- [docs/versions.md](docs/versions.md): upstream repo URLs, branches, and pinned commits
- [docs/worklog.md](docs/worklog.md): Codex work summary and verification notes
- [docs/ui-architecture-v0.md](docs/ui-architecture-v0.md): revised v0 architecture for a new ground UI
- [ground_ui](ground_ui): Qt6/QML flight UI with live OpenHD telemetry and RTP health integration

## Ground UI Prototype

Build:

```bash
cmake -S ~/openhd-ground-station/ground_ui -B ~/openhd-ground-station/ground_ui/build
cmake --build ~/openhd-ground-station/ground_ui/build -j"$(nproc)"
```

Run from a desktop GUI session:

```bash
~/openhd-ground-station/ground_ui/build/openhd_ground_ui
```

The UI connects to OpenHD Ground's multi-client MAVLink TCP server at `127.0.0.1:5760`, so it can run alongside QOpenHD. It decodes the additional RTP forwarding path at UDP `5800` without consuming QOpenHD's normal UDP `5600` feed.

Its default mode is live and read-only. Use `--demo` only for UI-only work:

```bash
~/openhd-ground-station/ground_ui/build/openhd_ground_ui --demo
```

The UI has six live, read-only pages:

- `FPV`: decoded RTP video and flight HUD
- `FLIGHT`: autopilot, position, attitude, GPS, battery, RC, and home telemetry
- `LINK`: OpenHD radio-card, telemetry, video transport, and FEC diagnostics
- `SYSTEM`: OpenHD Air/Ground core, camera, power, networking, radio-mode, and channel data
- `MESSAGES`: MAVLink `STATUSTEXT` messages
- `PROTOCOL`: every received MAVLink/OpenHD frame, including complete payload data

Click any diagnostic card to expand its full value. This includes long channel lists, RTSP configuration, and raw protocol payloads. Values only appear when the connected Air, Ground, or flight controller actually publishes their corresponding MAVLink message.

The `FPV` page follows the QOpenHD OSD composition: video is the back layer and a separate HUD layer shows downlink quality/RSSI, video rate and loss, battery, GPS, RC state, compass, artificial horizon, speed, altitude, flight mode, position, vertical speed, home distance, and alerts. The reference source is pinned in [docs/versions.md](docs/versions.md).

## Start OpenHD Ground

Run from a shell with sudo access:

```bash
~/openhd-ground-station/scripts/start-ground-openhd.sh
```

This script:

- disables any stale OpenHD hotspot connection
- switches `wlx20e15d2bfa3c` to monitor mode
- sets channel `36` / `5180 MHz`
- starts `/usr/local/bin/openhd -g`

## Check State

```bash
~/openhd-ground-station/scripts/check-ground-openhd.sh
```

This prints:

- `iw dev`
- `iwconfig`
- active NetworkManager connections
- UDP sockets relevant to OpenHD
- current `txrx.key` hash

## GUI Note

`QOpenHD` must be started from a desktop GUI session. It will not launch from a plain `tty` shell because `DISPLAY` is unset there.

## txrx.key Check

Ground-side hash:

```bash
sha256sum /usr/local/share/openhd/txrx.key
```

Compare that with the Air unit. If the hashes differ, copy one side's key to the other before link testing.
