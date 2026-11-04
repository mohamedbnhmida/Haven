# AwoX SmartPLUG → ESP32 bridge → Alexa / Google Home

Investigation notes, 2026-10-12. Status: **model confirmed (SMP-B16-FR, firmware 1.0.7, hardware 1.0); protocol recovered from the app, not yet tested on the plug** (the plug was unplugged during the session).

## Goal

```
Alexa / Google Home ──(Wi-Fi, Matter)──▶ ESP32 bridge ──(Bluetooth LE)──▶ AwoX SmartPLUG
```

## The device

| | |
|---|---|
| Product | AwoX **SmartPLUG**, French socket (shown as "SmartPLUG" in room "Living room") |
| Real hardware | Rebranded **Revogi** BLE plug, model **SMP-B16-FR**, **firmware 1.0.7, hardware 1.0** (confirmed from the device). Siblings: SMP-B13-UK, SMP-B16-GR |
| App | AwoX Smart CONTROL `com.awox.smart.control` **7.1.9**, APK saved in `apk/` |
| Code that drives it | `com.awox.core.impl.RevogiSmartPlugController` (chosen in `com.awox.core.model.Device` for device types 23/24/34) |
| Security | **None**: no pairing, no unlock step, no encryption |

Not to be confused with AwoX **mesh** plugs (`PlugMeshFR`, `PlugBridgeFR`, …), which use the encrypted Telink mesh (`TelinkMeshController`). The confirmed model SMP-B16-FR is the Revogi type, so the mesh section below does not apply to this plug.

## Bluetooth LE protocol

| Item | UUID |
|---|---|
| Service | `0000fff0-0000-1000-8000-00805f9b34fb` |
| Commands (write) | `0000fff3-0000-1000-8000-00805f9b34fb` |
| Replies (notify, enable on connect) | `0000fff4-0000-1000-8000-00805f9b34fb` |
| Firmware version (read) | `0000fff1-0000-1000-8000-00805f9b34fb` |

The app writes packets in **20-byte chunks**, queued, one at a time (~1 s timeout per packet).

### Packet format

```
0F | len | cmd | 00 | data… | checksum | FF FF
len      = len(data) + 3
checksum = (cmd + 1 + sum(data)) & 0xFF
```

Replies on `fff4` use the same framing: `cmd = raw[2]`, `data = raw[4 : len-3]`.

### Commands

| Cmd | Purpose | Data sent | Example |
|---|---|---|---|
| `03` | **Power on/off** | `[state, 0, 0]` (1 = on, 0 = off) | ON `0F 06 03 00 01 00 00 05 FF FF`, OFF `0F 06 03 00 00 00 00 04 FF FF` |
| `04` | **Read state + instant power** | `[0, 0]` | `0F 05 04 00 00 00 05 FF FF` |
| `0A` | Hourly consumption | `[0, 0]` | reply: list of int16 |
| `0B` | Daily consumption | `[0, 0]` | reply: list of int32 |
| `07` | Read schedules | `[0, 0]` | reply: 5 × 22-byte rules |
| `06` | Write schedules | 5 × 22-byte rules (110 bytes) | |
| `01` | Set clock | `[sec, min, hour, day, month, yearHi, yearLo, 0, 0, 0, 0]` | |
| `02` | Rename | 20-byte ASCII name | |
| `0F` | LED on/off: `[1, led, 0]` · factory reset: `[0, 0, 0]` | | |
| `10` | Read LED state | `[0, 0]` | reply `data[0] == 1` → LED on |

### Reply to `04` (state + power)

```
data[0]      on/off (≠0 = on)
data[1]      unknown
data[2..5]   instant power, int32 big-endian (unit not confirmed, probably mW)
```

### Schedule rule (22 bytes)

```
[0]      1 = slot used
[1..16]  name (ASCII, zero padded)
[17]     bits 0-6 = days of week, bit 7 = active
[18,19]  on time  hour, minute  (FF FF = none)
[20,21]  off time hour, minute  (FF FF = none)
```

## Session findings / diagnostics

- The plug did **not appear** in BLE scans from the Mac (only 1 unrelated device seen) because it was **unplugged**.
- Plugs like this stop advertising while a phone is connected; close the AwoX app (or `adb shell am force-stop com.awox.smart.control`) before scanning.
- The advertised name is unknown yet; look for service `fff0` or a name containing `SMP`/`plug`.
- Power unit, `data[1]` in the status reply, and the plug's write type (with/without response) still need confirming on the real device.
- The firmware version (1.0.7) should be readable from characteristic `fff1`; compare it with what `revogi.py status` prints to confirm the connection is to the right device.

## Next steps

1. **Plug it in**, close the AwoX app, and run the test tool from the Mac:
   ```bash
   cd tools && python3 -m venv .venv && .venv/bin/pip install bleak
   .venv/bin/python revogi.py scan
   .venv/bin/python revogi.py status <address>
   .venv/bin/python revogi.py on <address>   # then: off
   ```
2. Confirm the power unit by plugging in a known load (e.g. a 60 W bulb).
3. **ESP32 firmware** (Arduino, NimBLE-Arduino): scan for `fff0`, connect, subscribe to `fff4`, send `03`/`04`, poll `04` every ~10 s, auto-reconnect.
4. **Smart-home side**, depending on the board:
   - ESP32-C3 / S3 / C6 → **Matter On/Off Plug** (Arduino-ESP32 3.x `Matter` library, example `MatterOnOffPlugin`); the on/off callback sends cmd `03`. Commission in Google Home (scan the QR from serial), then share to Alexa (multi-admin).
   - Classic ESP32 → **Sinric Pro** (free, 3 devices, Alexa + Google Home).
   - Home Assistant user → ESPHome `ble_client` + template switch.
5. Extras: Wi-Fi provisioning, OTA updates, expose instant power as a sensor, sync state when the plug's own button is pressed (poll `04`).
6. Note: the plug accepts **one BLE connection** at a time, so the AwoX app cannot connect while the bridge holds it (disconnect when idle if both are needed).

## If it is a mesh plug

If the scan shows service `0x1910` / Telink mesh instead of `fff0`, the device is an AwoX mesh plug: commands are encrypted with a session key derived from the mesh name/password (factory default `unpaired` / `1234`). The open-source Home Assistant `awox` integration and `awoxmeshlight` library implement it; port that login + encryption to the ESP32.
