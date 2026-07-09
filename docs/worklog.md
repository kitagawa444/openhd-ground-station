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
