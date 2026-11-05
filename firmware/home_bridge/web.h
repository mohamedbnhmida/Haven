// HTTP API + the Haven web app (web_app.h). Everything but login needs the session cookie.
//   POST /api/login password=…            POST /api/logout
//   GET  /api/state                        GET  /api/history?id=N
//   POST /api/device?id=N&on=1|0[&white=1&color=ff8800&brightness=0-100]
//   POST /api/timer?id=N&min=30            (0 cancels)
//   POST /api/rename?id=N&name=…&sinric=…  POST /api/remove?id=N
//   GET  /api/scan                         POST /api/add?address=…&type=…&kind=plug|lamp&name=…
//   POST /api/rules json=[…]               (see schedules.h)
//   GET  /api/networks                     POST /api/wifi ssid=…&pass=…
#pragma once

#include "web_app.h"

WebServer web(80);
bool online = false;
uint16_t wifiReconnects = 0;

String sha256Hex(const String& s) {
  uint8_t out[32];
  mbedtls_sha256((const uint8_t*)s.c_str(), s.length(), out, 0);
  char hex[65];
  for (int i = 0; i < 32; i++) sprintf(hex + 2 * i, "%02x", out[i]);
  return hex;
}
String hashPassword(const String& pw) { return sha256Hex(WiFi.macAddress() + ":" + pw); }

// Session = random token in an HttpOnly cookie, kept in NVS so it survives reboots.
bool authed() {
  String tok = prefs.getString("token");
  return !tok.isEmpty() && web.header("Cookie").indexOf("hb=" + tok) >= 0;
}

bool guard() {
  if (authed()) return true;
  web.send(401, "application/json", prefs.getString("pwd").isEmpty() ? "{\"auth\":\"create\"}" : "{\"auth\":\"login\"}");
  return false;
}

String bootScan = "[]";  // networks seen at boot, before the setup AP started

String scanWifiJson() {
  int n = WiFi.scanNetworks();
  JsonDocument doc;
  JsonArray out = doc.to<JsonArray>();
  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    bool dup = false;
    for (JsonObject o : out) dup |= o["ssid"] == ssid;
    if (ssid.isEmpty() || dup) continue;  // hidden / same network on another channel
    JsonObject o = out.add<JsonObject>();
    o["ssid"] = ssid, o["rssi"] = WiFi.RSSI(i), o["secure"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
  }
  WiFi.scanDelete();
  String s;
  serializeJson(doc, s);
  return s;
}

void sendJson(JsonDocument& doc, int code = 200) {
  String s;
  serializeJson(doc, s);
  web.send(code, "application/json", s);
}

void sendError(int code, const char* msg) {
  JsonDocument doc;
  doc["error"] = msg;
  sendJson(doc, code);
}

void restartSoon() {
  web.client().flush();
  delay(600);
  ESP.restart();
}

// Device id from ?id=, or sends 404.
Device* argDevice() {
  int i = web.arg("id").toInt();
  if (web.arg("id").isEmpty() || i < 0 || i >= deviceCount) return sendError(404, "Unknown device"), nullptr;
  return &devices[i];
}

void sendState() {
  JsonDocument doc;
  doc["mode"] = online ? "ok" : "setup";
  JsonObject sys = doc["system"].to<JsonObject>();
  sys["ssid"] = online ? WiFi.SSID() : "";
  sys["rssi"] = online ? WiFi.RSSI() : 0;
  sys["ip"] = online ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
  sys["host"] = "haven.local";
  sys["uptime"] = millis() / 1000;
  sys["reconnects"] = wifiReconnects;
  sys["cloud"] = cloudConnected;
  sys["cloudConfigured"] = cloudConfigured();
  sys["time"] = timeValid() ? (long)time(nullptr) : 0;
  sys["maxDevices"] = kMaxDevices;

  JsonArray list = doc["devices"].to<JsonArray>();
  for (int i = 0; i < deviceCount; i++) {
    Device& d = devices[i];
    JsonObject o = list.add<JsonObject>();
    o["id"] = i, o["name"] = d.name, o["address"] = d.address, o["kind"] = d.isPlug() ? "plug" : "lamp";
    o["linked"] = d.linked(), o["on"] = d.on, o["reconnects"] = d.reconnects, o["sinric"] = d.sinricId;
    o["linkedFor"] = d.linked() ? (millis() - d.linkedAt) / 1000 : 0;
    o["offAt"] = (long)d.offAt;
    if (d.isPlug()) {
      o["powerMw"] = d.powerMw, o["todayWh"] = d.todayWh, o["yesterdayWh"] = d.yesterdayWh;
    } else {
      char c[7];
      snprintf(c, sizeof c, "%02x%02x%02x", d.r, d.g, d.b);
      o["white"] = d.white, o["color"] = c, o["brightness"] = d.brightness;
    }
  }
  rulesJson(doc["rules"].to<JsonArray>());
  sendJson(doc);
}

void startWeb() {
  const char* headers[] = {"Cookie"};
  web.collectHeaders(headers, 1);

  web.on("/", [] {
    web.sendHeader("Cache-Control", "no-cache");
    web.send_P(200, "text/html; charset=utf-8", kApp);
  });

  auto asset = [](const char* type, const char* body) {
    return [=] {
      web.sendHeader("Cache-Control", "public, max-age=604800");
      web.send_P(200, type, body);
    };
  };
  web.on("/icon.svg", asset("image/svg+xml", kIcon));
  web.on("/favicon.ico", asset("image/svg+xml", kIcon));
  web.on("/og.svg", asset("image/svg+xml", kOgImage));
  web.on("/manifest.webmanifest", asset("application/manifest+json", kManifest));

  web.on("/api/login", HTTP_POST, [] {
    String pw = web.arg("password"), stored = prefs.getString("pwd");
    if (stored.isEmpty()) {
      if (pw.length() < 6) return sendError(400, "Use at least 6 characters");
      prefs.putString("pwd", hashPassword(pw));
    } else if (hashPassword(pw) != stored) {
      delay(1000);  // slows down guessing
      return sendError(403, "Wrong password");
    }
    char tok[33];
    for (int i = 0; i < 4; i++) sprintf(tok + 8 * i, "%08lx", (unsigned long)esp_random());
    prefs.putString("token", tok);
    web.sendHeader("Set-Cookie", String("hb=") + tok + "; Path=/; HttpOnly; SameSite=Strict; Max-Age=31536000");
    web.send(200, "application/json", "{}");
  });

  web.on("/api/logout", HTTP_POST, [] {
    prefs.remove("token");  // signs out every browser
    web.sendHeader("Set-Cookie", "hb=; Path=/; Max-Age=0");
    web.send(200, "application/json", "{}");
  });

  web.on("/api/state", HTTP_GET, [] {
    if (guard()) sendState();
  });

  // 24 h of 15-min averages (W), oldest first; the last slot is the current, live average.
  web.on("/api/history", HTTP_GET, [] {
    if (!guard()) return;
    Device* d = argDevice();
    if (!d) return;
    JsonDocument doc;
    doc["bucketSec"] = kBucketSec;
    doc["end"] = timeValid() ? (long)time(nullptr) : 0;
    JsonArray w = doc["w"].to<JsonArray>();
    for (int k = 1; k <= kBuckets; k++) {
      uint32_t key = d->bucket + k;
      w.add(k == kBuckets ? (d->bucketN ? d->bucketSum / d->bucketN : 0) : d->hist[key % kBuckets] / 10.0);
    }
    sendJson(doc);
  });

  web.on("/api/device", HTTP_POST, [] {
    if (!guard()) return;
    Device* d = argDevice();
    if (!d) return;
    if (!d->linked()) return sendError(503, "Device is out of reach");
    if (!d->isPlug()) {
      if (web.hasArg("white")) d->white = web.arg("white") == "1";
      if (web.hasArg("brightness")) d->brightness = constrain(web.arg("brightness").toInt(), 1, 100);
      if (web.hasArg("color")) {
        long c = strtol(web.arg("color").c_str(), nullptr, 16);
        d->r = c >> 16, d->g = c >> 8, d->b = c, d->white = false;
      }
    }
    setOn(*d, web.hasArg("on") ? web.arg("on") == "1" : true);
    sendState();
  });

  web.on("/api/timer", HTTP_POST, [] {
    if (!guard()) return;
    Device* d = argDevice();
    if (!d) return;
    if (!timeValid()) return sendError(503, "Clock not synced yet");
    int min = web.arg("min").toInt();
    d->offAt = min > 0 ? time(nullptr) + min * 60 : 0;
    sendState();
  });

  web.on("/api/rename", HTTP_POST, [] {
    if (!guard()) return;
    Device* d = argDevice();
    if (!d) return;
    String name = web.arg("name");
    name.trim();
    if (name.isEmpty() || name.length() > 32) return sendError(400, "Name must be 1–32 characters");
    d->name = name;
    bool cloudChanged = web.hasArg("sinric") && web.arg("sinric") != d->sinricId;
    if (cloudChanged) d->sinricId = web.arg("sinric");
    saveDevices();
    sendState();
    if (cloudChanged) restartSoon();  // Sinric Pro devices are registered at boot
  });

  web.on("/api/remove", HTTP_POST, [] {
    if (!guard()) return;
    Device* d = argDevice();
    if (!d) return;
    if (d->client && d->client->isConnected()) d->client->disconnect();
    prefs.remove(energyKey(*d).c_str());
    int i = d - devices;
    for (int k = i; k < deviceCount - 1; k++) devices[k] = devices[k + 1];
    deviceCount--;
    saveDevices();
    web.send(200, "application/json", "{}");
    restartSoon();  // simplest way to drop the BLE client cleanly
  });

  web.on("/api/scan", HTTP_GET, [] {
    if (guard()) web.send(200, "application/json", scanJson());
  });

  web.on("/api/add", HTTP_POST, [] {
    if (!guard()) return;
    const char* err = addDevice(web.arg("address"), web.arg("type").toInt(), web.arg("kind") == "lamp", web.arg("name"));
    if (err) return sendError(400, err);
    sendState();
    if (!devices[deviceCount - 1].sinricId.isEmpty()) restartSoon();  // Sinric Pro devices are registered at boot
  });

  web.on("/api/rules", HTTP_POST, [] {
    if (!guard()) return;
    if (!setRules(web.arg("json"))) return sendError(400, "Invalid schedule");
    sendState();
  });

  web.on("/api/networks", HTTP_GET, [] {
    if (!guard()) return;
    String fresh = scanWifiJson();
    web.send(200, "application/json", fresh != "[]" ? fresh : bootScan);  // scans can fail while the AP is busy
  });

  web.on("/api/wifi", HTTP_POST, [] {
    if (!guard()) return;
    if (web.arg("ssid").isEmpty()) return sendError(400, "Pick a network");
    prefs.putString("ssid", web.arg("ssid"));
    prefs.putString("pass", web.arg("pass"));
    web.send(200, "application/json", "{}");
    restartSoon();
  });

  // Setup hotspot: any other URL (phones' captive-portal checks) lands on the app.
  web.onNotFound([] {
    if (online) return web.send(404, "text/plain", "Not found");
    web.sendHeader("Location", "http://192.168.4.1/");
    web.send(302, "text/plain", "");
  });

  web.begin();
}
