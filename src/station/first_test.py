#!/usr/bin/env python3
#
#

import json
import time
from datetime import datetime

DATE_FMT = "%d/%m/%Y %H:%M:%S"
import requests
import telebot

from private import TOKEN, CHAT_ID

# Bot related
bot = telebot.TeleBot(TOKEN, parse_mode=None)

# Station related
STATION = "http://localhost:8123"
DeviceMap = {}
NUM_BOARDS_MAX = 13
DEVS_GOOD = list(range(1, NUM_BOARDS_MAX + 1))
# Boards than can be written to
DEVS_WRITE_EN = list(range(1, (NUM_BOARDS_MAX // 2) + 1))

CMD_WAIT = 1 * 60  # Time in min to wait between commands

# 1. Setup up the station
# 2. Register connected boards
# 3. Read memory contents off all boards
#   3.1. First read serves as reference samples
#   3.2. Consecutive reads serves as data samples
# 3.
# 4. Power off for 5 min


def timestamp():
    """
    Timestamp to keep track of the events
    """
    return datetime.now().strftime(DATE_FMT)


def power_on():
    """
    Power on all boards in the chain and wait
    """
    res = requests.get(f"{STATION}/devices/poweron")
    res = json.loads(res.text)
    msg = f'{res["message"].upper()}\n'
    msg = f"{msg}\n[{timestamp()}]"
    bot.send_message(CHAT_ID, msg)


def power_off():
    """
    Power off all boards in the chain and wait
    """
    res = requests.get(f"{STATION}/devices/poweroff")
    res = json.loads(res.text)
    msg = f'{res["message"].upper()}\n'
    msg = f"{msg}\n[{timestamp()}]"
    bot.send_message(CHAT_ID, msg)


def register_ports():
    """
    Register ports connected to the station
    """
    res = requests.get(f"{STATION}/ports/register")
    res = json.loads(res.text)
    msg = f'{res["message"].upper()}\n'
    time.sleep(1)

    res = requests.get(f"{STATION}/ports/available")
    res = json.loads(res.text)
    for port in res["ports"]:
        msg = f"{msg}{port}\n"
    msg = f"{msg}\n[{timestamp()}]"
    bot.send_message(CHAT_ID, msg)


def register_boards():
    """
    Register and discover all boards connected to the station
    """
    register_ports()
    time.sleep(5)

    res = requests.get(f"{STATION}/devices/register")
    res = json.loads(res.text)
    msg = f'{res["message"].upper()}\n'
    msg = f"{msg}\n[{timestamp()}]"
    bot.send_message(CHAT_ID, msg)


def available_boards():
    """
    Shows the available boards connected to thes station
    """
    global DeviceMap

    res = requests.get(f"{STATION}/devices/available")
    DeviceMap = json.loads(res.text)
    msg = ""

    for port_name in DeviceMap["ports"]:
        msg = f"{port_name.upper()}\n"
        device_list = DeviceMap["ports"][port_name]
        for dev in device_list:
            msg = f'{msg}{dev["TTL"]:0>2}: {dev["board_id"]}\n'
        msg = f"{msg}\n"

    msg = f"{msg}[{timestamp()}]"
    bot.send_message(CHAT_ID, msg)


def read_memory():
    """
    Read the contents of memory from all boards
    """
    bot.send_message(CHAT_ID, "READ MEMORY VALUES")

    for port_name in DeviceMap["ports"]:
        device_list = DeviceMap["ports"][port_name]
        for dev in device_list:
            # Memory map for 80kB: 0x20000000 - 0x20014000
            # We need to use the offset from the start of the memory
            for address in range(0x0000, 0x14000, 512):
                msg = f'READ [0x{address:08x}] BOARD [{dev["board_id"]}]'
                msg = f"{msg}\n[{timestamp()}]"
                bot.send_message(CHAT_ID, msg)
                res = requests.post(
                    f"{STATION}/commands/read",
                    data=json.dumps(
                        {
                            "board_id": dev["board_id"],
                            "mem_address": address,
                            "port_name": port_name,
                        }
                    ),
                    headers={"content-type": "application/json"},
                )
                time.sleep(CMD_WAIT)


def write_memory():
    """
    Write opposite values in memory to half the boards
    """

    bot.send_message(CHAT_ID, "WRITE MEMORY VALUES")

    for port_name in DeviceMap["ports"]:
        device_list = DeviceMap["ports"][port_name]
        for dev in device_list:
            # Only write to allowed boards
            if not dev["TTL"] in DEVS_WRITE_EN:
                continue

            # Memory map for 80kB: 0x20000000 - 0x20014000
            # For writing to memory we are leaving 3K at each end
            # This will prevent overwriting important values
            # So the new memory map is: 0x20001000 - 0x20013400
            for address in range(0x1000, 0x13400, 512):
                msg = f'WRITE [0x{address:08x}] BOARD [{dev["board_id"]}]'
                msg = f"{msg}\n[{timestamp()}]"
                bot.send_message(CHAT_ID, msg)

                res = requests.post(
                    f"{STATION}/commands/write_invert",
                    data=json.dumps(
                        {
                            "board_id": dev["board_id"],
                            "mem_address": address,
                            "port_name": port_name,
                        }
                    ),
                    headers={"content-type": "application/json"},
                )
                time.sleep(CMD_WAIT)


if __name__ == "__main__":
    msg = f"STARTING STATION\n[{timestamp()}]"
    bot.send_message(CHAT_ID, msg)

    ttl_devs = []
    bids = set()

    STATE = "read"

    while True:

        # Wait untill all boars are correctly mapped
        while sorted(ttl_devs) != DEVS_GOOD and len(bids) != NUM_BOARDS_MAX:

            power_off()
            time.sleep(5 * 60)

            power_on()
            time.sleep(10)

            ttl_devs = []
            bids = set()

            register_boards()
            time.sleep(30)

            available_boards()

            for port_name in DeviceMap["ports"]:
                device_list = DeviceMap["ports"][port_name]
                for dev in device_list:
                    ttl_devs.append(int(dev["TTL"]))
                    bids.add(dev["board_id"])

            time.sleep(1)

        msg = f"STARTING OPERATIONS\n[{timestamp()}]"

        bot.send_message(CHAT_ID, msg)

        if STATE == "read":
            read_memory()
            STATE = "write"
        else:
            write_memory()
            STATE = "read"
