# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this project is

A door security and monitoring system running on a **Radxa Dragon** SBC (Linux). It displays a Hikvision door-station RTSP stream on a 480×480 HDMI panel, controls a door lock via an RP2040 microcontroller, and reacts to motion and vibration sensors. All user-facing logic is Bash scripts managed by systemd.

## Deployment

```bash
# Deploy everything to the target host (SSH alias "dragon"):
./deploy
```

The `deploy` script rsyncs `etc/door-eye/*.env`, `bin/*`, and `systemd/*` to the host, then enables and restarts every unit in `systemd/`.

## RP2040 firmware

The firmware lives in `rp2040/`. It is built with Docker (the image is called `pico-builder` and must exist locally):

```bash
cd rp2040
make clean && make compile   # builds inside Docker → produces build/*.uf2
make install                 # scp uf2 + upload script to dragon, then flashes
```

To build the Docker image first: `docker build -t pico-builder rp2040/`.

To flash directly from the Radxa Dragon: `./rp2040/upload-firmware.sh` (after copying the files).

## Architecture

### RP2040 ↔ host IPC (`rp-daemon` / `rp-cmd`)

The RP2040 communicates over USB serial (ttyACM0) and understands a simple line-based protocol:

| Command | Description |
|---|---|
| `GET <pin>` | Read input pin (or vpin100/vpin200) → `0` or `1` |
| `SET <pin> <0\|1>` | Write output pin → `OK` |
| `LOCK` / `UNLOCK` | Run close/open lock sequence → `OK` |
| `REBOOT` / `BOOTLOAD` | Reset or enter UF2 bootloader |

`rp-daemon` is the serial port owner. It multiplexes access via a named FIFO (`/var/run/rp.fifo`). Any script that needs to talk to the RP2040 writes `$PID $command` to the FIFO and polls for `/var/run/rp/$PID`.

`rp-cmd` wraps this protocol: `rp-cmd "GET 100"`.

### Virtual pins (vpins)

Two logical sensors are implemented in the RP2040 firmware as *virtual pins* with latching and cooldown, accessed via `GET 100` / `GET 200`:

- **vpin100** — motion detector (radar on PIN26). Latches `1` for 60 s after any motion pulse.
- **vpin200** — vibration/remote relay (PIN28, active-low). Latches `1` for 5 s, then 5 s cooldown.

### Services and their roles

| Service | Script | Purpose |
|---|---|---|
| `rp-daemon` | `bin/rp-daemon` | Owns the serial port; FIFO-based IPC for all RP2040 commands |
| `handle-motion-sensor` | `bin/handle-motion-sensor` | Polls `GET 100`; turns screen on/off; starts/stops `show-door-eye` and `update-door-eye-osd` |
| `handle-vibration-sensor` | `bin/handle-vibration-sensor` | Polls `GET 200`; plays audio alert via pipewire/paplay |
| `show-door-eye` | `bin/show-door-eye` | Runs `mpv` in DRM/KMS mode displaying the RTSP stream |
| `update-door-eye-osd` | `bin/update-door-eye-osd` | Loops every second; pushes time + Open-Meteo weather to mpv OSD via socat/IPC socket |
| `autounlock` | `bin/autounlock` | Polls Hikvision door-station REST API; sends `UNLOCK` to `rp-cmd` on authorized card or indoor button |
| `download-weather-data` | `bin/download-weather-data` | Fetches from Open-Meteo API; writes JSON to `/var/run/door-eye/weather.json` (run by systemd timer) |
| `autoconnect-bt-speaker` | `bin/autoconnect-bt-speaker` | Keeps a Bluetooth speaker connected; sets volume via wpctl |

### Configuration files (`etc/door-eye/`)

Each service sources its own env file. Sample files (`*.env.sample`) show required variables. Key files:

- `rp.env` — serial port, FIFO paths, baud rate, RTSP URL, screen/DRM settings
- `app.env` — shared paths (JSON weather file, mpv IPC socket)
- `mpv.env` / `screen.env` — mpv and display parameters (loaded by `show-door-eye`)
- `door-station.env` — Hikvision host/credentials (see `.sample`)
- `open-meteo.env` — latitude, longitude, timezone
- `bt-speaker.env` — Bluetooth MAC address, volume

### Screen management

`handle-motion-sensor` controls the HDMI connector at the kernel level (`/sys/class/drm/card1-HDMI-A-1/status`) rather than using DPMS, because the display requires an EDID override (custom 480×480 mode). The EDID binary lives in `lib/firmware/edid/` and must be deployed to `/lib/firmware/edid/` on the host with the kernel cmdline parameters described in `monitor.md`.

### Lock hardware (RP2040 side)

The RP2040 drives two motor output pins (PIN0 = open, PIN1 = close) and reads lock-open/lock-closed/door-closed sensors. `close_lock` and `open_lock` in `rp2040/lock.c` poll sensors and apply settle delays; they refuse to act if the door is open. All lock control goes through `rp-cmd "LOCK"` / `rp-cmd "UNLOCK"`.
