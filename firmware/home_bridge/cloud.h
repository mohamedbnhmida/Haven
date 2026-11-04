// Alexa / Google Home through Sinric Pro: each device with a Sinric device id becomes a
// switch (plug) or a colour light (lamp). Keys live in secrets.h.
#pragma once

bool cloudConnected = false;

bool cloudConfigured() { return strncmp(SINRIC_APP_KEY, "YOUR", 4) != 0; }

Device* bySinricId(const String& id) {
  for (int i = 0; i < deviceCount; i++)
    if (devices[i].sinricId == id) return &devices[i];
  return nullptr;
}

void startCloud() {
  for (int i = 0; i < deviceCount; i++) {
    Device& d = devices[i];
    if (d.sinricId.isEmpty()) continue;
    auto power = [](const String& id, bool& on) {
      Device* d = bySinricId(id);
      bool ok = d && setOn(*d, on);
      Serial.printf("cloud: %s -> %s: %s\n", d ? d->name.c_str() : id.c_str(), on ? "on" : "off",
                    ok ? "done" : d ? "device out of reach" : "unknown device id");
      return ok;
    };
    if (d.isPlug()) {
      SinricProSwitch& sw = SinricPro[d.sinricId];
      sw.onPowerState(power);
      continue;
    }
    SinricProLight& light = SinricPro[d.sinricId];
    light.onPowerState(power);
    light.onBrightness([](const String& id, int& v) {
      Device* d = bySinricId(id);
      if (!d) return false;
      d->brightness = constrain(v, 1, 100), d->on = true;
      return lampSend(*d), d->linked();
    });
    light.onColor([](const String& id, byte& r, byte& g, byte& b) {
      Device* d = bySinricId(id);
      if (!d) return false;
      d->r = r, d->g = g, d->b = b, d->white = false, d->on = true;
      return lampSend(*d), d->linked();
    });
    light.onColorTemperature([](const String& id, int&) {  // any white = the bulb's 2700 K
      Device* d = bySinricId(id);
      if (!d) return false;
      d->white = true, d->on = true;
      return lampSend(*d), d->linked();
    });
  }
  SinricPro.onConnected([] { cloudConnected = true, Serial.println("cloud: connected"); });
  SinricPro.onDisconnected([] { cloudConnected = false, Serial.println("cloud: disconnected"); });
  for (int i = 0; i < deviceCount; i++)
    Serial.printf("cloud: %s = %s\n", devices[i].name.c_str(), devices[i].sinricId.isEmpty() ? "(no Sinric id)" : devices[i].sinricId.c_str());
  SinricPro.begin(SINRIC_APP_KEY, SINRIC_APP_SECRET);
}

// Changes made outside Alexa (web app, schedules, the plug's own button) go back to the cloud.
void syncCloud() {
  SinricPro.handle();
  for (int i = 0; i < deviceCount; i++) {
    Device& d = devices[i];
    if (d.sinricId.isEmpty() || !d.linked() || d.on == d.reported) continue;
    d.reported = d.on;
    if (d.isPlug()) {
      SinricProSwitch& sw = SinricPro[d.sinricId];
      sw.sendPowerStateEvent(d.on);
    } else {
      SinricProLight& light = SinricPro[d.sinricId];
      light.sendPowerStateEvent(d.on);
    }
  }
}

// Adds a device (web app or serial console). Returns an error message, or nullptr.
const char* addDevice(String addr, uint8_t type, bool lamp, String name) {
  addr.toLowerCase(), name.trim();
  if (deviceCount == kMaxDevices) return "This bridge holds up to 3 devices";
  if (addr.length() != 17 || deviceIndex(addr) >= 0) return "Unknown or already added device";
  Device& d = devices[deviceCount++];
  d = Device();
  d.address = addr, d.name = name.isEmpty() ? (lamp ? "Lamp" : "Smart plug") : name.substring(0, 32);
  d.addrType = type, d.kind = lamp ? Kind::Lamp : Kind::Plug;
  // Hand out the Sinric Pro ids from secrets.h to the first plug / lamp that needs one.
  const char* id = lamp ? SINRIC_LAMP_ID : SINRIC_PLUG_ID;
  if (strncmp(id, "YOUR", 4) != 0 && !bySinricId(id)) d.sinricId = id;
  saveDevices();
  return nullptr;
}
