// Haven — a Wi-Fi bridge for Bluetooth-only smart devices (AwoX SmartPLUG, Shada lamp),
// with a web app, schedules, power monitoring and Alexa / Google Home via Sinric Pro.
// Protocols: docs/awox_smartplug.md, PLAN.md §1.
//
// First boot (or Wi-Fi unreachable): opens AP "Haven-Setup"; browse http://192.168.4.1,
// create a password and pick the home Wi-Fi. Then http://haven.local (API in web.h).
// Hold BOOT 5 s = factory reset (Wi-Fi, password, devices, schedules).
//
// Build: arduino-cli compile -b esp32:esp32:esp32:PartitionScheme=huge_app

#include <ArduinoJson.h>
#include <NimBLEDevice.h>
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

// Forgotten password or wrong Wi-Fi: hold BOOT for 5 s to wipe settings.
void checkFactoryReset() {
  static uint32_t pressedAt = 0;
  if (digitalRead(0) == HIGH) return void(pressedAt = 0);
  if (!pressedAt) pressedAt = millis();
  if (millis() - pressedAt > 5000) {
    Serial.println("factory reset");
    prefs.clear();
    ESP.restart();
  }
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
  } else if (cmd == "scan") {
    Serial.println(scanJson());
  } else if (cmd == "wifi-reset") {
    for (const char* k : {"ssid", "pass", "pwd", "token"}) prefs.remove(k);
    Serial.println("wifi + password cleared, restarting into Haven-Setup");
    delay(200);
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(0, INPUT_PULLUP);  // BOOT button
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
    Serial.println("setup AP: Haven-Setup -> http://192.168.4.1");
  }
  startWeb();
  NimBLEDevice::init("Haven");
}

void loop() {
  web.handleClient();
  checkFactoryReset();
  serialCommands();
  keepLinked();
  runSchedules();
  if (online && cloudConfigured()) syncCloud();
}
