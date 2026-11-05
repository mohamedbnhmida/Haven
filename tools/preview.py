"""Serve the Haven web app from firmware/home_bridge/web_app.h with a fake API, so the UI
can be designed and screenshotted without the ESP32.

    python3 tools/preview.py            → http://localhost:8080
    python3 tools/preview.py --login    → sign-in screen, on :8081
    http://localhost:8080/#splash       → splash screen held; #nosplash skips it (screenshots)
"""
import json, math, re, sys, time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlparse

SRC = (Path(__file__).parent.parent / 'firmware/home_bridge/web_app.h').read_text()
def block(name, tag):
    return re.search(rf'{name}\[\] PROGMEM = R"{tag}\((.*?)\){tag}";', SRC, re.S).group(1)
ASSETS = {'/': ('text/html; charset=utf-8', block('kApp', 'html')),
          '/icon.svg': ('image/svg+xml', block('kIcon', 'svg')),
          '/og.svg': ('image/svg+xml', block('kOgImage', 'svg')),
          '/manifest.webmanifest': ('application/manifest+json', block('kManifest', 'json'))}

T0 = time.time()
authed = '--login' not in sys.argv
devices = [
    {'id': 0, 'name': 'Living room', 'address': '98:7b:f3:34:9b:83', 'kind': 'plug', 'linked': True, 'on': True,
     'reconnects': 1, 'sinric': '000000000000000000000000', 'linkedFor': 15840, 'offAt': 0,
     'powerMw': 64250, 'todayWh': 412.6, 'yesterdayWh': 980.2},
    {'id': 1, 'name': 'Bedroom lamp', 'address': 'fc:58:fa:01:00:06', 'kind': 'lamp', 'linked': True, 'on': True,
     'reconnects': 0, 'sinric': '', 'linkedFor': 7300, 'offAt': 0, 'white': False, 'color': '7c5cff', 'brightness': 70},
    {'id': 2, 'name': 'Coffee machine', 'address': '98:7b:f3:12:44:a0', 'kind': 'plug', 'linked': False, 'on': False,
     'reconnects': 3, 'sinric': '', 'linkedFor': 0, 'offAt': 0, 'powerMw': -1, 'todayWh': 85.0, 'yesterdayWh': 120.4},
]
rules = [{'address': '98:7b:f3:34:9b:83', 'enabled': True, 'on': True, 'days': 31, 'hour': 7, 'minute': 0},
         {'address': '98:7b:f3:34:9b:83', 'enabled': True, 'on': False, 'days': 127, 'hour': 23, 'minute': 30},
         {'address': '98:7b:f3:34:9b:83', 'enabled': False, 'on': True, 'days': 96, 'hour': 9, 'minute': 15}]

def state():
    return {'mode': 'ok', 'devices': devices, 'rules': rules, 'system': {
        'ssid': 'Maison-5G', 'rssi': -58, 'ip': '192.168.1.42', 'host': 'haven.local',
        'uptime': int(time.time() - T0) + 273600, 'reconnects': 2, 'cloud': True, 'cloudConfigured': True,
        'time': int(time.time()), 'maxDevices': 3}}

def history():  # a believable day: night standby, morning bump, evening peak
    w = []
    for k in range(96):
        h = k / 4
        v = 4 + 55 * math.exp(-((h - 7.5) ** 2) / 1.2) + 85 * math.exp(-((h - 20) ** 2) / 4) + 6 * math.sin(k)
        w.append(round(max(2, v), 1))
    return {'bucketSec': 900, 'end': int(time.time()), 'w': w}

class H(BaseHTTPRequestHandler):
    def log_message(self, *a): pass
    def send(self, code, body, ctype='application/json'):
        b = body if isinstance(body, bytes) else (body if isinstance(body, str) else json.dumps(body)).encode()
        self.send_response(code); self.send_header('Content-Type', ctype); self.end_headers(); self.wfile.write(b)
    def do_GET(self):
        global authed
        u = urlparse(self.path)
        if u.path in ASSETS: return self.send(200, ASSETS[u.path][1], ASSETS[u.path][0])
        if not authed: return self.send(401, {'auth': 'login'})
        if u.path == '/api/state': return self.send(200, state())
        if u.path == '/api/history': return self.send(200, history())
        if u.path == '/api/networks': return self.send(200, [
            {'ssid': 'Maison-5G', 'rssi': -58, 'secure': True}, {'ssid': 'Maison', 'rssi': -63, 'secure': True},
            {'ssid': 'TOPNET_A1F2', 'rssi': -77, 'secure': True}, {'ssid': 'Cafe Guest', 'rssi': -84, 'secure': False}])
        if u.path == '/api/scan':
            time.sleep(1.5)
            return self.send(200, [{'address': 'a4:c1:38:0b:22:9e', 'type': 0, 'kind': 'plug', 'name': 'SMP-B16-FR', 'rssi': -62},
                                   {'address': 'fc:58:fa:01:02:11', 'type': 0, 'kind': 'lamp', 'name': 'LED', 'rssi': -71}])
        self.send(404, {'error': 'not found'})
    def do_POST(self):
        global authed, rules
        u = urlparse(self.path)
        q = {k: v[0] for k, v in parse_qs(self.rfile.read(int(self.headers.get('Content-Length') or 0)).decode()).items()}
        if u.path == '/api/login': authed = True; return self.send(200, {})
        if u.path == '/api/logout': authed = False; return self.send(200, {})
        if not authed: return self.send(401, {'auth': 'login'})
        if u.path == '/api/device':
            d = devices[int(q['id'])]
            if 'on' in q: d['on'] = q['on'] == '1'
            if 'color' in q: d.update(color=q['color'], white=False, on=True)
            if 'white' in q: d.update(white=True, on=True)
            if 'brightness' in q: d['brightness'] = int(q['brightness'])
            if d['kind'] == 'plug': d['powerMw'] = 64250 if d['on'] else 0
        elif u.path == '/api/timer':
            m = int(q.get('min', 0)); devices[int(q['id'])]['offAt'] = int(time.time()) + m * 60 if m else 0
        elif u.path == '/api/rules': rules = json.loads(q['json'])
        elif u.path == '/api/rename': devices[int(q['id'])].update(name=q['name'], sinric=q.get('sinric', ''))
        elif u.path == '/api/wifi': return self.send(200, {})
        return self.send(200, state())

if __name__ == '__main__':
    port = 8081 if '--login' in sys.argv else 8080
    print(f'Haven preview on http://localhost:{port}')
    ThreadingHTTPServer(('', port), H).serve_forever()
