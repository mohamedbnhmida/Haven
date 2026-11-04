// Weekly on/off rules and sleep timers, run on the bridge with NTP time.
#pragma once

constexpr const char* kTimeZone = "CET-1";  // Tunisia: UTC+1, no daylight saving
constexpr int kMaxRules = 16;

struct Rule {
  String address;  // device
  bool enabled = true, on = true;
  uint8_t days = 0x7F;  // bit 0 = Monday … bit 6 = Sunday
  uint8_t hour = 0, minute = 0;
};

Rule rules[kMaxRules];
int ruleCount = 0;

void saveRules() {
  JsonDocument doc;
  for (int i = 0; i < ruleCount; i++) {
    Rule& r = rules[i];
    JsonObject o = doc.add<JsonObject>();
    o["a"] = r.address, o["e"] = r.enabled, o["o"] = r.on, o["d"] = r.days, o["h"] = r.hour, o["m"] = r.minute;
  }
  String s;
  serializeJson(doc, s);
  prefs.putString("rules", s);
}

// Replaces all rules from the web app's JSON: [{"address","enabled","on","days","hour","minute"}].
bool setRules(const String& json) {
  JsonDocument doc;
  if (deserializeJson(doc, json) != DeserializationError::Ok || !doc.is<JsonArray>()) return false;
  ruleCount = 0;
  for (JsonObject o : doc.as<JsonArray>()) {
    if (ruleCount == kMaxRules) break;
    Rule& r = rules[ruleCount++];
    r.address = o["address"].as<String>(), r.enabled = o["enabled"] | true, r.on = o["on"] | true;
    r.days = (o["days"] | 0x7F) & 0x7F, r.hour = (o["hour"] | 0) % 24, r.minute = (o["minute"] | 0) % 60;
  }
  saveRules();
  return true;
}

void loadRules() {
  JsonDocument doc;
  if (deserializeJson(doc, prefs.getString("rules", "[]")) != DeserializationError::Ok) return;
  for (JsonObject o : doc.as<JsonArray>()) {
    if (ruleCount == kMaxRules) break;
    Rule& r = rules[ruleCount++];
    r.address = o["a"].as<String>(), r.enabled = o["e"], r.on = o["o"], r.days = o["d"], r.hour = o["h"], r.minute = o["m"];
  }
}

void rulesJson(JsonArray out) {
  for (int i = 0; i < ruleCount; i++) {
    Rule& r = rules[i];
    JsonObject o = out.add<JsonObject>();
    o["address"] = r.address, o["enabled"] = r.enabled, o["on"] = r.on, o["days"] = r.days, o["hour"] = r.hour,
    o["minute"] = r.minute;
  }
}

void runSchedules() {
  static int lastMinute = -1;
  if (!timeValid()) return;
  time_t t = time(nullptr);

  for (int i = 0; i < deviceCount; i++)  // sleep timers
    if (devices[i].offAt && t >= devices[i].offAt) setOn(devices[i], false), devices[i].offAt = 0;

  tm lt;
  localtime_r(&t, &lt);
  int minuteOfWeek = lt.tm_wday * 1440 + lt.tm_hour * 60 + lt.tm_min;
  if (minuteOfWeek == lastMinute) return;  // once per minute
  lastMinute = minuteOfWeek;
  uint8_t today = 1 << ((lt.tm_wday + 6) % 7);  // tm_wday: 0 = Sunday
  for (int i = 0; i < ruleCount; i++) {
    Rule& r = rules[i];
    int d = deviceIndex(r.address);
    if (r.enabled && d >= 0 && (r.days & today) && r.hour == lt.tm_hour && r.minute == lt.tm_min) {
      bool ok = setOn(devices[d], r.on);
      Serial.printf("rule %02d:%02d %s %s: %s\n", r.hour, r.minute, devices[d].name.c_str(), r.on ? "on" : "off",
                    ok ? "done" : "device offline");
    }
  }
}
