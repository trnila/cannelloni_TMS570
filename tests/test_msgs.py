#!/usr/bin/env python3
import os
import can
import pytest
from collections import namedtuple

CANMsgID = namedtuple('CANMsgID', ['arbitration_id', 'is_extended_id'])


def create_cans(dir) -> tuple[can.Bus, can.bus]:
    assert dir in ('rx', 'tx')
    a = can.Bus(interface='socketcan', channel=os.getenv("CAN_RX", "can0"))
    b = can.Bus(interface='socketcan', channel=os.getenv("CAN_TX", "can-0-0"))
    return (a, b) if dir == 'rx' else (b, a)



@pytest.mark.parametrize("dlc", range(0, 9))
@pytest.mark.parametrize("msgid", [
    CANMsgID(0, False),
    CANMsgID(1, True),
    CANMsgID(0x42, True),
    CANMsgID(0x42, False),
    CANMsgID(0x7FF, True),
    CANMsgID(0x7FF, False),
    CANMsgID(0x800, True),
    CANMsgID(0x1fffffff, True),
], ids=lambda msg: f"0x{msg.arbitration_id:x}-{'extended' if msg.is_extended_id else 'std'}")
@pytest.mark.parametrize("dir", ['rx', 'tx'])
def test_dlc(dir, msgid, dlc):
    tx, rx = create_cans(dir)
    tx.send(can.Message(arbitration_id=msgid.arbitration_id, is_extended_id=msgid.is_extended_id, data=range(dlc)))
    received = rx.recv(1)

    assert received.arbitration_id == msgid.arbitration_id
    assert received.is_extended_id == msgid.is_extended_id
    assert received.data == bytearray(range(dlc))
