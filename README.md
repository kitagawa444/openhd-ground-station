# openhd-ground-station

OpenHD ground station setup for Ubuntu 22.04 without git submodules.

This repository tracks:

- local configuration files
- helper scripts
- install and verification notes
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
