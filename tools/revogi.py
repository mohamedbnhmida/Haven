"""Test tool for the AwoX SmartPLUG (Revogi SMP-B16-FR) over Bluetooth LE, before writing the ESP32 bridge.

    pip install bleak
    python revogi.py scan
    python revogi.py status <address>
    python revogi.py on <address>
    python revogi.py off <address>

<address> is the BLE address printed by `scan` (a UUID on macOS, a MAC on Linux/Windows).
Protocol recovered from com.awox.smart.control 7.1.9 (RevogiSmartPlugController), see ../docs/awox_smartplug.md.
"""
import asyncio
import sys

from bleak import BleakClient, BleakScanner

SERVICE = '0000fff0-0000-1000-8000-00805f9b34fb'
WRITE = '0000fff3-0000-1000-8000-00805f9b34fb'
NOTIFY = '0000fff4-0000-1000-8000-00805f9b34fb'
VERSION = '0000fff1-0000-1000-8000-00805f9b34fb'


def packet(cmd: int, data: list[int]) -> bytes:
    """0F, len+3, cmd, 00, data..., (cmd + 1 + sum(data)) & FF, FF, FF"""
    return bytes([0x0F, len(data) + 3, cmd, 0x00, *data, (cmd + 1 + sum(data)) & 0xFF, 0xFF, 0xFF])


ON = packet(0x03, [1, 0, 0])      # 0F 06 03 00 01 00 00 05 FF FF
OFF = packet(0x03, [0, 0, 0])     # 0F 06 03 00 00 00 00 04 FF FF
STATUS = packet(0x04, [0, 0])     # 0F 05 04 00 00 00 05 FF FF -> notify cmd 04


def parse(raw: bytes) -> str:
    cmd, data = raw[2], raw[4:-3]
    if cmd == 0x04 and len(data) >= 6:
        on = data[0] != 0
        power = int.from_bytes(data[2:6], 'big')  # unit not confirmed yet (likely mW)
        return f'state={"ON" if on else "OFF"} power_raw={power}'
    return f'cmd={cmd:#04x} data={data.hex(" ")}'


async def scan():
    found = await BleakScanner.discover(timeout=15, return_adv=True)
    for addr, (dev, adv) in found.items():
        name = dev.name or adv.local_name or ''
        tag = 'MATCH ' if any(u.startswith('0000fff0') for u in adv.service_uuids) or 'smp' in name.lower() or 'plug' in name.lower() else '      '
        print(f'{tag}{addr}  {name!r:24} rssi={adv.rssi}  {adv.service_uuids[:3]}')


async def run(action: str, address: str):
    async with BleakClient(address, timeout=20) as client:
        got = asyncio.Event()

        def on_notify(_, data: bytearray):
            print('notify', bytes(data).hex(' '), '->', parse(bytes(data)))
            got.set()

        await client.start_notify(NOTIFY, on_notify)
        try:
            print('firmware', (await client.read_gatt_char(VERSION)).hex(' '))
        except Exception as e:  # not critical
            print('firmware read failed:', e)
        payload = {'on': ON, 'off': OFF, 'status': STATUS}[action]
        props = client.services.get_characteristic(WRITE).properties
        with_response = 'write' in props
        for i in range(0, len(payload), 20):  # the app writes in 20-byte chunks
            await client.write_gatt_char(WRITE, payload[i:i + 20], response=with_response)
        print('sent', payload.hex(' '))
        if action != 'status':
            await client.write_gatt_char(WRITE, STATUS, response=with_response)
        try:
            await asyncio.wait_for(got.wait(), 5)
        except asyncio.TimeoutError:
            print('no notification within 5 s')


if __name__ == '__main__':
    if len(sys.argv) < 2 or sys.argv[1] not in ('scan', 'status', 'on', 'off') or (sys.argv[1] != 'scan' and len(sys.argv) < 3):
        sys.exit(__doc__)
    asyncio.run(scan() if sys.argv[1] == 'scan' else run(sys.argv[1], sys.argv[2]))
