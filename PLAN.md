# Home Bridge: ESP32 for the Shada bulb + AwoX SmartPLUG → Alexa / Google Home

Plan only, nothing built yet (2026-10-12). Both devices are Bluetooth LE only and have no cloud; one ESP32 bridges them to Wi-Fi and to the voice assistants.

```
                       ┌────────────── ESP32 "Home Bridge" ──────────────┐
Alexa ─┐               │                                                  │   BLE   ┌─────────────────────┐
       ├─ Matter/Wi-Fi ┤  Matter node                BLE central          ├────────▶│ Shada LED's light   │
Google ┘               │   • endpoint 1: Colour light ──▶ ShadaBulb       │         │ bulb (art. 600750)  │
                       │   • endpoint 2: On/Off plug  ──▶ AwoxPlug        │   BLE   ├─────────────────────┤
Phone / browser ─ HTTP ┤  Local web page + REST API (testing, fallback)    ├────────▶│ AwoX SmartPLUG      │
                       └──────────────────────────────────────────────────┘         │ (Revogi SMP-B16-FR) │
                                                                                     └─────────────────────┘
```

## 1. What we already know

Both protocols are recovered and need **no pairing and no encryption**.

### Shada LED's light bulb (tested, works)
Full details: [Shada-Led-s-Light repo](https://github.com/mohamedbnhmida/Shada-Led-s-Light), `lib/protocol.dart`.

| Item | Value |
|---|---|
| Advertised service | `ffffcc02-0000-1000-8000-00805f9b34fb` or `0000cc02-…` |
| Unlock | after connecting, write ASCII `quintic` to `0000ee02-…` (once per connection) |
| Control | write 10 bytes to `0000ee03-…`, write without response |
| Packet | `[on, G, white, cold, on, B, on, R, white, warm]` |
| Colour | three `on` bytes = 1, `white` bytes = 0, R/G/B ≥ 1 (brightness = scale RGB) |
| White | both `white` bytes = 1, `warm` = brightness (warm-white 2700 K LEDs only; `cold` unused) |
| Off | all flags 0 |
| State | notifications on `0000ee01-…`, same layout |
| Known lamp | `FC:58:FA:01:00:06` (name "LED") |

### AwoX SmartPLUG: SMP-B16-FR, firmware 1.0.7, hardware 1.0 (model confirmed; protocol decoded from the app, not yet tested)
Full details: [docs/awox_smartplug.md](docs/awox_smartplug.md); test tool `tools/revogi.py`.

| Item | Value |
|---|---|
| Service | `0000fff0-…`; write `fff3`, notify `fff4`, firmware version `fff1` |
| Packet | `0F, len+3, cmd, 00, data…, (cmd+1+Σdata)&FF, FF, FF`, 20-byte chunks |
| ON / OFF | `0F 06 03 00 01 00 00 05 FF FF` / `0F 06 03 00 00 00 00 04 FF FF` |
| State + power | send `0F 05 04 00 00 00 05 FF FF` → reply cmd `04`: `data[0]` on/off, `data[2..5]` power (int32 BE, unit TBC) |

## 2. Hardware

| Part | Notes |
|---|---|
| ESP32 board | **ESP32-C3, C6 or S3** recommended (Matter-ready, BLE 5). A classic ESP32 also works (Matter over Wi-Fi or Sinric Pro). |
| Power | USB 5 V adapter |
| Placement | Within ~5–10 m of both devices (BLE range); ideally between them |

Tooling already on the Mac: `arduino-cli` 1.5.1, ESP32 Arduino core **3.0.1** (needs **≥ 3.1** for the Matter library → `arduino-cli core upgrade esp32:esp32`).

## 3. Firmware design

### Modules
| Module | Job |
|---|---|
| `ShadaBulb` | find by service `ffffcc02`/`cc02` (or pinned MAC), connect, send `quintic`, write colour/white/off packets, coalesce rapid updates |
| `AwoxPlug` | find by service `fff0`, connect, subscribe `fff4`, send on/off, poll state + power every ~10 s |
| `BleManager` | one scan at boot, reconnect with back-off, keep both connections (ESP32 allows several BLE client links) |
| `WebApi` | `GET /api/state`, `POST /api/bulb {on, hue, sat, brightness, white}`, `POST /api/plug {on}` + a small control page |
| `MatterBridge` | endpoint 1 **Matter Color Light** → `ShadaBulb`; endpoint 2 **Matter On/Off Plugin** → `AwoxPlug`; push state changes back |
| `Config` | Wi-Fi credentials + device MACs in NVS (`Preferences`), Wi-Fi setup via AP portal on first boot, OTA updates |

### Connection policy
- Keep both BLE links open for instant response; reconnect automatically when a device is power-cycled.
- The bulb and the plug accept **one connection at a time**: while the bridge is connected, the phone apps (Shada LED Light, AwoX) cannot connect. Option: an "idle release" setting that disconnects after N minutes without commands, or simply use the bridge from then on.
- The Shada app's background schedules would compete with the bridge. Once the bridge is live, move schedules to Alexa/Google routines (or to the bridge) and disable them in the app.

### Mapping to Matter
| Matter attribute | Shada bulb | AwoX plug |
|---|---|---|
| OnOff | on flag | cmd `03` |
| Level (brightness) | RGB scale, or `warm` in white mode | — |
| Hue / Saturation | colour mode (saturation < ~15 % → white mode) | — |
| Colour temperature | report fixed 2700 K (bulb has warm white only) | — |
| Electrical power (optional) | — | cmd `04` power value |

## 4. Steps

1. **Test the plug from the Mac**: plug it in, close the AwoX app, run `tools/revogi.py scan / status / on / off`; confirm the power unit with a known load.
2. **Choose the board** (C3/C6/S3 preferred) and upgrade the ESP32 core to ≥ 3.1.
3. **Phase 1, local bridge**: BLE drivers for both devices + web page/REST API; test from a phone browser on the same Wi-Fi.
4. **Phase 2, Matter**: add the two Matter endpoints, flash, read the pairing QR code from the serial monitor.
5. **Add to Google Home**: Google Home app → Add → Matter device → scan the QR → two devices appear (light + plug).
6. **Share to Alexa**: in Google Home, *Linked Matter apps & services* → generate a pairing code → Alexa app → Add device → Matter → enter the code. Both assistants then control the same bridge locally.
7. **Polish**: Wi-Fi portal, OTA, plug button state sync, bridge status LED, auto-recovery after power cuts.

### Fallbacks if Matter is not an option
- **Sinric Pro** (free tier, 3 devices): ready Alexa skill + Google action, ESP32 library; works on a classic ESP32.
- **Home Assistant + ESPHome**: `ble_client` for both devices, then HA's Alexa/Google integrations.
- **Espalexa** (Alexa only, local Hue emulation): quickest demo, no Google Home.

## 5. Open questions

- Plug (SMP-B16-FR, fw 1.0.7, hw 1.0): real advertised name, write type (with/without response), power unit, meaning of `data[1]`.
- Bulb: read current state on connect via `ee01` notifications so the bridge starts in sync.
- Which ESP32 board is available?
- Keep the phone apps usable alongside the bridge (idle release), or bridge only?
