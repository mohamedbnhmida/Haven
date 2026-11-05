// Haven — a Wi-Fi bridge for Bluetooth-only smart devices (AwoX SmartPLUG, Shada lamp),
// with a web app, schedules, power monitoring and Alexa / Google Home via Sinric Pro.
// Protocols: docs/awox_smartplug.md, PLAN.md §1.
//
// First boot (or Wi-Fi unreachable): opens AP "Haven-Setup" (captive portal); browse
// http://haven.local, create a password and pick the home Wi-Fi. Then http://haven.local (API in web.h).
// BOOT button: hold 3 s = reset Wi-Fi + password, 10 s = erase everything.
//
// Build: arduino-cli compile -b esp32:esp32:esp32:PartitionScheme=huge_app

#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <SinricPro.h>
#include <SinricProLight.h>
#include <SinricProSwitch.h>
#include <WebServer.h>
#include <WiFi.h>
#include <mbedtls/sha256.h>

#include "secrets.h"

Preferences prefs;
DNSServer dns;

#include "devices.h"
#include "schedules.h"
#include "cloud.h"
#include "web.h"

bool joinWifi() {
  String ssid = prefs.getString("ssid");
  if (ssid.isEmpty()) return false;
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("haven");
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid.c_str(), prefs.getString("pass").c_str());
  for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) delay(500);
  return WiFi.status() == WL_CONNECTED;
}

constexpr int kBootPin = 0, kLedPin = 2;  // DevKit BOOT button, blue LED

void clearSetup() {  // Wi-Fi + Haven password; devices and schedules stay
  for (const char* k : {"ssid", "pass", "pwd", "token"}) prefs.remove(k);
}

// BOOT button, acted on at release: hold 3 s = reset setup (Wi-Fi + password),
// 10 s = erase everything. The LED blinks slowly, then fast after 3 s, then stays on after 10 s.
void checkBootButton() {
  static uint32_t pressedAt = 0;
  bool down = digitalRead(kBootPin) == LOW;
  if (down && !pressedAt) pressedAt = millis() | 1;
  if (!pressedAt) return;
  uint32_t held = millis() - pressedAt;
  if (down) {
    digitalWrite(kLedPin, held >= 10000 ? HIGH : (millis() / (held >= 3000 ? 100 : 400)) % 2);
    return;
  }
  pressedAt = 0;
  digitalWrite(kLedPin, LOW);
  if (held < 3000) return;
  if (held >= 10000) prefs.clear(), Serial.println("BOOT: everything erased");
  else clearSetup(), Serial.println("BOOT: Wi-Fi + password reset");
  delay(200);
  ESP.restart();
}

// Serial console: "status", "wifi-reset" (forget Wi-Fi + password; devices and schedules stay).
void serialCommands() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  if (cmd == "status") {
    Serial.printf("wifi %s rssi %d  cloud %s  heap %u (min %u, max block %u)\n", online ? WiFi.SSID().c_str() : "(setup)",
                  WiFi.RSSI(), cloudConnected ? "connected" : "offline", ESP.getFreeHeap(), ESP.getMinFreeHeap(),
                  ESP.getMaxAllocHeap());
    for (int i = 0; i < deviceCount; i++)
      Serial.printf("  %s %s %s\n", devices[i].name.c_str(), devices[i].linked() ? "linked" : "out of reach",
                    devices[i].on ? "on" : "off");
  } else if (cmd.startsWith("add ")) {  // add plug|lamp <address> [name…]
    int sp1 = cmd.indexOf(' ', 4), sp2 = cmd.indexOf(' ', sp1 + 1);
    String kind = cmd.substring(4, sp1), addr = cmd.substring(sp1 + 1, sp2 < 0 ? cmd.length() : sp2);
    const char* err = sp1 < 0 ? "usage: add plug|lamp <address> [name]"
                              : addDevice(addr, 0, kind == "lamp", sp2 < 0 ? "" : cmd.substring(sp2 + 1));
    Serial.println(err ? err : "added, restarting");
    if (!err) delay(200), ESP.restart();
  } else if (cmd.startsWith("dev ")) {  // dev <n> on|off|white|rrggbb
    int sp = cmd.indexOf(' ', 4), n = cmd.substring(4, sp).toInt();
    String what = sp < 0 ? "" : cmd.substring(sp + 1);
    if (sp < 0 || n < 0 || n >= deviceCount) return void(Serial.println("usage: dev <n> on|off|white|rrggbb"));
    Device& d = devices[n];
    if (what.length() == 6 && !d.isPlug()) {
      long c = strtol(what.c_str(), nullptr, 16);
      d.r = c >> 16, d.g = c >> 8, d.b = c, d.white = false;
    } else if (what == "white") {
      d.white = true;
    }
    Serial.printf("%s %s: %s\n", d.name.c_str(), what.c_str(), setOn(d, what != "off") ? "sent" : "out of reach");
  } else if (cmd == "scan") {
    Serial.println(scanJson());
  } else if (cmd == "factory-reset") {  // same as holding BOOT 10 s
    prefs.clear();
    Serial.println("everything cleared, restarting into Haven-Setup");
    delay(200);
    ESP.restart();
  } else if (cmd == "wifi-reset") {
    clearSetup();
    Serial.println("wifi + password cleared, restarting into Haven-Setup");
    delay(200);
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(kBootPin, INPUT_PULLUP);
  pinMode(kLedPin, OUTPUT);
  prefs.begin("bridge");
  loadDevices();
  loadRules();

  online = joinWifi();
  if (online) {
    WiFi.onEvent([](WiFiEvent_t, WiFiEventInfo_t) { wifiReconnects++; }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    configTzTime(kTimeZone, "pool.ntp.org", "time.google.com");
    MDNS.begin("haven");
    Serial.printf("wifi ok http://haven.local  http://%s\n", WiFi.localIP().toString().c_str());
    if (cloudConfigured()) startCloud();
  } else {
    WiFi.setAutoReconnect(false);  // a station still retrying a wrong password makes every scan fail
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    bootScan = scanWifiJson();
    WiFi.mode(WIFI_AP_STA);  // STA side lets the setup page list networks
    WiFi.softAP("Haven-Setup");
    // Captive portal: every name resolves to the bridge, so phones pop the setup page up
    // by themselves and http://haven.local works here too.
    dns.start(53, "*", WiFi.softAPIP());
    MDNS.begin("haven");
    Serial.println("setup AP: Haven-Setup -> http://haven.local or http://192.168.4.1");
  }
  startWeb();
  NimBLEDevice::init("Haven");
}

void loop() {
  web.handleClient();
  checkBootButton();
  if (!online) dns.processNextRequest();
  serialCommands();
  keepLinked();
  runSchedules();
  if (online && cloudConfigured()) syncCloud();
}
