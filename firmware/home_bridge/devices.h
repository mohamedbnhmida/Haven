// Bluetooth devices: AwoX SmartPLUGs (Revogi protocol) and Shada lamps, several at once,
// with 24 h power history and daily energy for plugs.
#pragma once

enum class Kind : uint8_t { Plug, Lamp };
constexpr int kMaxDevices = 3;   // simultaneous BLE links that fit next to Wi-Fi
constexpr int kBucketSec = 900;  // power history: 15-min averages…
constexpr int kBuckets = 96;     // …over 24 h

static NimBLEUUID kPlugService("0000fff0-0000-1000-8000-00805f9b34fb");
static NimBLEUUID kPlugCommand("0000fff3-0000-1000-8000-00805f9b34fb");
static NimBLEUUID kPlugReply("0000fff4-0000-1000-8000-00805f9b34fb");
static NimBLEUUID kLampService("0000cc02-0000-1000-8000-00805f9b34fb");
static NimBLEUUID kLampAdvert("ffffcc02-0000-1000-8000-00805f9b34fb");
static NimBLEUUID kLampUnlock("0000ee02-0000-1000-8000-00805f9b34fb");
static NimBLEUUID kLampColor("0000ee03-0000-1000-8000-00805f9b34fb");

struct Device {
  String name, address, sinricId;
  Kind kind = Kind::Plug;
  uint8_t addrType = BLE_ADDR_PUBLIC;

  NimBLEClient* client = nullptr;
  NimBLERemoteCharacteristic* tx = nullptr;
  uint32_t lastTry = 0, linkedAt = 0, lastPoll = 0;
  uint16_t reconnects = 0;
  bool wasLinked = false;

  bool on = false, reported = false;  // reported = last state pushed to Alexa / Google
  int32_t powerMw = -1;               // plug: probably mW (unit not confirmed yet)
  time_t offAt = 0;                   // sleep timer (epoch s), 0 = none

  bool white = true;  // lamp: what we last sent (not read back)
  uint8_t r = 255, g = 120, b = 0;
  int brightness = 100;

  uint16_t hist[kBuckets] = {};  // average W × 10, slot = (epoch / kBucketSec) % kBuckets
  uint32_t bucket = 0;
  double bucketSum = 0, todayWh = 0, yesterdayWh = 0;
  uint32_t bucketN = 0, lastSample = 0;
  int yday = -1;

  bool linked() { return client && client->isConnected() && tx; }
  bool isPlug() const { return kind == Kind::Plug; }
};

Device devices[kMaxDevices];
int deviceCount = 0;

bool timeValid() { return time(nullptr) > 1700000000; }

// ---------- Persistence ----------

void saveDevices() {
  JsonDocument doc;
  for (int i = 0; i < deviceCount; i++) {
    Device& d = devices[i];
    JsonObject o = doc.add<JsonObject>();
    o["n"] = d.name, o["a"] = d.address, o["t"] = (int)d.addrType, o["k"] = (int)d.kind, o["s"] = d.sinricId;
  }
  String s;
  serializeJson(doc, s);
  prefs.putString("devices", s);
}

// NVS keys max 15 chars: "e" + the 12 hex digits of the address.
String energyKey(const Device& d) {
  String k = "e" + d.address;
  k.replace(":", "");
  return k;
}

void saveEnergy(int i) {
  Device& d = devices[i];
  prefs.putString(energyKey(d).c_str(),
                  String(d.todayWh, 2) + "," + String(d.yesterdayWh, 2) + "," + d.yday);
}

void loadDevices() {
  JsonDocument doc;
  if (deserializeJson(doc, prefs.getString("devices", "[]")) == DeserializationError::Ok) {
    for (JsonObject o : doc.as<JsonArray>()) {
      if (deviceCount == kMaxDevices) break;
      Device& d = devices[deviceCount++];
      d.name = o["n"].as<String>(), d.address = o["a"].as<String>(), d.sinricId = o["s"] | "";
      d.addrType = o["t"] | 0, d.kind = (Kind)(o["k"] | 0);
    }
  }
  // Upgrade from the single-plug firmware: it remembered "plug" = "address/type".
  String old = prefs.getString("plug");
  if (deviceCount == 0 && old.indexOf('/') > 0) {
    Device& d = devices[deviceCount++];
    d.name = "SmartPLUG", d.address = old.substring(0, old.indexOf('/'));
    d.addrType = old.substring(old.indexOf('/') + 1).toInt();
    if (strncmp(SINRIC_PLUG_ID, "YOUR", 4) != 0) d.sinricId = SINRIC_PLUG_ID;
    saveDevices();
    prefs.remove("plug");
  }
  for (int i = 0; i < deviceCount; i++) {
    Device& d = devices[i];
    String e = prefs.getString(energyKey(d).c_str());
    int c1 = e.indexOf(','), c2 = e.lastIndexOf(',');
    if (c1 > 0 && c2 > c1) d.todayWh = e.toFloat(), d.yesterdayWh = e.substring(c1 + 1).toFloat(), d.yday = e.substring(c2 + 1).toInt();
  }
}

int deviceIndex(const String& address) {
  for (int i = 0; i < deviceCount; i++)
    if (devices[i].address.equalsIgnoreCase(address)) return i;
  return -1;
}

// ---------- Monitoring ----------

// Called on every power reading: integrates energy and fills the 24 h history.
void sample(int i) {
  Device& d = devices[i];
  if (d.powerMw < 0) return;
  uint32_t now = millis();
  float w = d.powerMw / 1000.0f;
  if (d.lastSample) d.todayWh += w * min<uint32_t>(now - d.lastSample, 60000) / 3600000.0;
  d.lastSample = now;

  if (!timeValid()) return;
  time_t t = time(nullptr);
  tm lt;
  localtime_r(&t, &lt);
  if (d.yday != lt.tm_yday) {  // midnight: today → yesterday
    if (d.yday >= 0) d.yesterdayWh = d.todayWh, d.todayWh = 0;
    d.yday = lt.tm_yday;
    saveEnergy(i);
  }
  uint32_t key = t / kBucketSec;
  if (key != d.bucket) {
    if (d.bucket) {
      d.hist[d.bucket % kBuckets] = d.bucketN ? d.bucketSum / d.bucketN * 10 : 0;
      for (uint32_t k = d.bucket + 1; k < key && k < d.bucket + kBuckets; k++) d.hist[k % kBuckets] = 0;
      saveEnergy(i);  // every 15 min: bounded flash wear
    }
    d.bucket = key, d.bucketSum = 0, d.bucketN = 0;
  }
  d.bucketSum += w, d.bucketN++;
}

// ---------- AwoX plug protocol ----------

// 0F | len | cmd | 00 | data… | (cmd + 1 + Σdata) & FF | FF FF, written in 20-byte chunks.
void plugSend(Device& d, uint8_t cmd, std::initializer_list<uint8_t> data) {
  if (!d.linked()) return;
  uint8_t buf[32], n = 0, sum = cmd + 1;
  buf[n++] = 0x0F, buf[n++] = data.size() + 3, buf[n++] = cmd, buf[n++] = 0x00;
  for (uint8_t v : data) buf[n++] = v, sum += v;
  buf[n++] = sum, buf[n++] = 0xFF, buf[n++] = 0xFF;
  for (uint8_t k = 0; k < n; k += 20) d.tx->writeValue(buf + k, min<uint8_t>(20, n - k), !d.tx->canWriteNoResponse());
}

// ---------- Shada lamp protocol ----------

// 10 bytes: [on, G, white, cold, on, B, on, R, white, warm]. No brightness byte: channels are
// scaled, never 0 (the original app doesn't send 0). Warm-white LEDs only, cold is unused.
void lampSend(Device& d) {
  if (!d.linked()) return;
  auto lvl = [&](int v) { return (uint8_t)constrain(v * d.brightness / 100, 1, 255); };
  uint8_t o = d.on;
  uint8_t p[10] = {o, lvl(d.g), 0, 0, o, lvl(d.b), o, lvl(d.r), 0, 0};
  if (d.white) {
    uint8_t w[10] = {0, 0, o, 1, 0, 0, 0, 0, o, lvl(255)};
    memcpy(p, w, 10);
  }
  d.tx->writeValue(p, 10, !d.tx->canWriteNoResponse());
}

// ---------- Commands ----------

bool setOn(Device& d, bool on) {
  if (!d.linked()) return false;
  d.on = d.reported = on;
  if (!on) d.offAt = 0;
  if (d.isPlug()) {
    plugSend(d, 0x03, {(uint8_t)on, 0, 0});
    delay(250);
    plugSend(d, 0x04, {0, 0});
  } else {
    lampSend(d);
  }
  return true;
}

// ---------- Links ----------

bool connectDevice(int i) {
  Device& d = devices[i];
  if (!d.client) {
    d.client = NimBLEDevice::createClient();
    d.client->setConnectTimeout(5000);  // ms; an absent device must not stall the loop for long
  }
  if (!d.client->connect(NimBLEAddress(std::string(d.address.c_str()), d.addrType))) return false;
  NimBLERemoteService* svc = d.client->getService(d.isPlug() ? kPlugService : kLampService);
  if (!svc) return d.client->disconnect(), false;

  if (d.isPlug()) {
    d.tx = svc->getCharacteristic(kPlugCommand);
    NimBLERemoteCharacteristic* reply = svc->getCharacteristic(kPlugReply);
    if (!d.tx || !reply) return d.tx = nullptr, d.client->disconnect(), false;
    // Reply to cmd 04: data = raw[4..]; data[0] on/off, data[2..5] power int32 BE.
    reply->subscribe(true, [i](NimBLERemoteCharacteristic*, uint8_t* raw, size_t len, bool) {
      if (len < 10 || raw[0] != 0x0F || raw[2] != 0x04) return;
      devices[i].on = raw[4] != 0;
      devices[i].powerMw = (int32_t)raw[6] << 24 | raw[7] << 16 | raw[8] << 8 | raw[9];
      sample(i);
    }, true);
    plugSend(d, 0x04, {0, 0});
  } else {
    NimBLERemoteCharacteristic* unlock = svc->getCharacteristic(kLampUnlock);
    d.tx = svc->getCharacteristic(kLampColor);
    if (!unlock || !d.tx) return d.tx = nullptr, d.client->disconnect(), false;
    uint8_t word[] = {'q', 'u', 'i', 'n', 't', 'i', 'c'};  // unlock, once per connection
    unlock->writeValue(word, sizeof word, !unlock->canWriteNoResponse());
  }
  return true;
}

// ponytail: fixed 10 s retry per device; add back-off if it fights the phone apps.
void keepLinked() {
  for (int i = 0; i < deviceCount; i++) {
    Device& d = devices[i];
    if (d.linked()) {
      if (d.isPlug() && millis() - d.lastPoll > 10000) {  // also catches the plug's own button
        d.lastPoll = millis();
        plugSend(d, 0x04, {0, 0});
      }
      continue;
    }
    if (d.wasLinked) d.wasLinked = false, d.reconnects++, d.tx = nullptr;
    if (millis() - d.lastTry < 10000) continue;
    d.lastTry = millis();
    if (connectDevice(i)) {
      d.wasLinked = true, d.linkedAt = millis();
      Serial.printf("%s linked\n", d.name.c_str());
    }
  }
}

// Nearby plugs / lamps that aren't added yet, as JSON.
String scanJson() {
  NimBLEScan* scan = NimBLEDevice::getScan();
  scan->setActiveScan(true);
  NimBLEScanResults found = scan->getResults(8000, false);  // weak devices need a few adverts
  JsonDocument doc;
  JsonArray out = doc.to<JsonArray>();
  for (int i = 0; i < found.getCount(); i++) {
    const NimBLEAdvertisedDevice& a = *found.getDevice(i);
    bool plug = a.isAdvertisingService(kPlugService);
    bool lamp = a.isAdvertisingService(kLampAdvert) || a.isAdvertisingService(kLampService);
    String addr = a.getAddress().toString().c_str();
    if ((!plug && !lamp) || deviceIndex(addr) >= 0) continue;
    JsonObject o = out.add<JsonObject>();
    o["address"] = addr, o["type"] = a.getAddress().getType(), o["kind"] = plug ? "plug" : "lamp";
    o["name"] = !a.getName().empty() ? a.getName().c_str() : (plug ? "Smart plug" : "Lamp"), o["rssi"] = a.getRSSI();
  }
  scan->clearResults();
  String s;
  serializeJson(doc, s);
  return s;
}
