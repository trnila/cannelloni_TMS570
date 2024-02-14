#!/usr/bin/env python3
import re
import io
from dataclasses import dataclass

MAX_PORTS = 5


class Symbol:
    def __init__(self, end_bit, start_bit=None):
        self.end_bit = end_bit
        self.start_bit = start_bit if start_bit is not None else end_bit

    @property
    def bits(self):
        return self.end_bit - self.start_bit + 1


class Mem:
    @classmethod
    def parse(cls, *bytes):
        assert len(bytes) == 4 * cls.WORDS
        num = 0
        for b in range(len(bytes) // 4):
            dword = int.from_bytes(bytearray(bytes[4 * b : 4 * (b + 1)]), "big")
            num |= dword << (b * 32)

        symbols = {}
        for name, info in cls.__dataclass_fields__.items():
            symbol: Symbol = info.type
            symbols[name] = (num >> symbol.start_bit) & ((1 << symbol.bits) - 1)

        return cls(**symbols)

    def to_bytes(self):
        register_val = 0
        for name, info in self.__dataclass_fields__.items():
            symbol: Symbol = info.type
            val = getattr(self, name)
            if val == -1:
                val = (1 << symbol.bits) - 1

            if (val & ((1 << symbol.bits) - 1)) != val:
                raise Exception(
                    f"Symbol {name} with value 0x{val:x} overflows {symbol.bits} bits"
                )

            register_val |= (val & ((1 << symbol.bits) - 1)) << symbol.start_bit

        dwords = []
        while len(dwords) != self.WORDS:
            dwords.append(register_val & 0xFFFFFFFF)
            register_val >>= 32
        return dwords

    def __str__(self):
        fields = []
        for name, _ in self.__dataclass_fields__.items():
            val = getattr(self, name)
            if val:
                fields.append(f"{name}={val}")

        return f'{self.__class__.__name__}({", ".join(fields)})'


class Writer:
    def __init__(self):
        self.padding = 4 * " "
        self.new_code = io.StringIO()
        self.written = []
        self.device_id = 0x9E00030E

    def bit_reverse(self, val, width=32):
        new_val = 0
        for i in range(width):
            bit = (val & (1 << i)) != 0
            new_val |= bit << (width - i - 1)
        return new_val

    def bit_not(self, n, numbits):
        return (1 << numbits) - 1 - n

    def crc32_add(self, crc, b):
        b = self.bit_reverse(b, 32)
        for i in range(8):
            if (crc ^ b) & (1 << 31):
                crc <<= 1
                crc ^= 0x04C11DB7
            else:
                crc <<= 1
            b <<= 1
        return crc

    def crc32(self, data):
        crc = 0xFFFFFFFF
        for w in data:
            crc = self.crc32_add(crc, w & 0xFF)
            crc = self.crc32_add(crc, (w >> 8) & 0xFF)
            crc = self.crc32_add(crc, (w >> 16) & 0xFF)
            crc = self.crc32_add(crc, (w >> 24) & 0xFF)
        return self.bit_reverse(self.bit_not(crc, 32), 32)

    def add_line(self, line):
        if line:
            self.new_code.write(self.padding)
            self.new_code.write(line.strip())
        self.new_code.write("\n")

    def add_word(self, w):
        self.written.append(w)
        self.add_line(
            ", ".join(
                [
                    f"0x{b:02x}"
                    for b in [
                        (w >> 24) & 0xFF,
                        (w >> 16) & 0xFF,
                        (w >> 8) & 0xFF,
                        w & 0xFF,
                    ]
                ]
            )
            + ","
        )

    def add_words(self, words):
        for w in words:
            self.add_word(w)

    def write(self, code_path, blocks: dict[int, list[Mem]]):
        self.add_word(self.device_id)
        for block_id, records in blocks.items():
            size = 0
            for item in records:
                if isinstance(item, Mem):
                    size += len(item.to_bytes())
                else:
                    size += 1

            hdr = [block_id << 24, size]
            hdr.append(self.crc32(hdr))
            self.add_line(f"// BLOCK 0x{block_id:x} HEADER")
            self.add_words(hdr)
            self.add_line(f"// BLOCK 0x{block_id:x} PAYLOAD")
            payload_words = []
            for item in records:
                if isinstance(item, Mem):
                    self.add_line(f"// {item}")
                    for word in item.to_bytes():
                        self.add_word(word)
                        payload_words.append(word)
                else:
                    self.add_word(item)
                    payload_words.append(item)
            self.add_line(f"// BLOCK 0x{block_id:x} CRC")
            self.add_word(self.crc32(payload_words))
            self.add_line("")
        self.add_line("// TOTAL CRC")
        self.add_words([0, 0])
        self.add_word(self.crc32(self.written))

        with open(code_path, "r+") as f:
            code = f.read()

            delimiter = "SWITCH CONFIG"
            begin = f"// BEGIN {delimiter}"
            end = f"// END {delimiter}"
            code = re.sub(
                f"{begin}(.*?){end}",
                f"{begin}\n{self.new_code.getvalue()}{self.padding}{end}",
                code,
                flags=re.DOTALL,
            )

            f.seek(0)
            f.write(code)
            f.truncate()


@dataclass
class L2PolicyRecord(Mem):
    WORDS = 2

    SHARINDX: Symbol(63, 58) = 0
    SMAX: Symbol(57, 42) = 0
    RATE: Symbol(41, 26) = 0
    MAXLEN: Symbol(25, 15) = 0
    PARTITION: Symbol(14, 12) = 0


@dataclass
class L2ForwardingRecord(Mem):
    WORDS = 2

    BC_DOMAIN: Symbol(63, 59) = 0
    REACH_PORT: Symbol(58, 54) = 0
    FL_DOMAIN: Symbol(53, 49) = 0
    VLAN_PMAP7: Symbol(48, 46) = 0
    VLAN_PMAP0: Symbol(27, 25) = 0


@dataclass
class XMIIModeParams(Mem):
    WORDS = 1

    xMII_MODE0: Symbol(18, 17) = 0
    xMII_MAC0: Symbol(19) = 0

    xMII_MODE1: Symbol(21, 20) = 0
    xMII_MAC1: Symbol(22) = 0

    xMII_MODE2: Symbol(24, 23) = 0
    xMII_MAC2: Symbol(25) = 0

    xMII_MODE3: Symbol(27, 26) = 0
    xMII_MAC3: Symbol(28) = 0

    xMII_MODE4: Symbol(30, 29) = 0
    xMII_MAC4: Symbol(31) = 0


@dataclass
class GeneralParameters(Mem):
    WORDS = 10

    VLLUPFORMAT: Symbol(351) = 0
    MIRR_PTACU: Symbol(350) = 0
    SWITCHID: Symbol(349, 347) = 0
    HOSTPRIO: Symbol(346, 344) = 0
    MAC_FLTRES1: Symbol(343, 296) = 0
    MAC_FLTRES0: Symbol(295, 248) = 0
    MAC_FLT1: Symbol(247, 200) = 0
    MAC_FLT0: Symbol(199, 152) = 0
    INCL_SRCPT1: Symbol(151) = 0
    INCL_SRCPT0: Symbol(150) = 0
    SEND_META1: Symbol(149) = 0
    SEND_META0: Symbol(148) = 0
    CASC_PORT: Symbol(147, 145) = 0
    HOST_PORT: Symbol(144, 142) = 0
    MIRR_PORT: Symbol(141, 139) = 0
    VIMARKER: Symbol(138, 107) = 0
    VIMASK: Symbol(106, 75) = 0
    TPID: Symbol(74, 59) = 0
    IGNORE2STF: Symbol(58) = 0
    TPID2: Symbol(57, 42) = 0
    QUEUE_TS: Symbol(41) = 0
    EGRMIRRVID: Symbol(40, 29) = 0
    EGRMIRRPCP: Symbol(28, 26) = 0
    EGRMIRRDEI: Symbol(25) = 0
    REPLAY_PORT: Symbol(24, 22) = 0


@dataclass
class L2ForwardingParam(Mem):
    WORDS = 3

    PART_SPC0: Symbol(22, 13) = 0


@dataclass
class MACConfigTable(Mem):
    WORDS = 7

    TOP0: Symbol(255, 247) = 0
    BASE0: Symbol(246, 238) = 0
    ENABLED0: Symbol(237) = 0
    TOP0: Symbol(122, 114) = 0
    BASE0: Symbol(113, 105) = 0
    ENABLED0: Symbol(104) = 0
    IFG: Symbol(103, 99) = 0
    SPEED: Symbol(98, 97) = 0
    TP_DELIN: Symbol(96, 81) = 0
    TP_DELOUT: Symbol(80, 65) = 0
    MAXAGE: Symbol(64, 57) = 0
    VLANPRIO: Symbol(56, 54) = 0
    VLANID: Symbol(53, 42) = 0
    ING_MIRR: Symbol(41) = 0
    EGR_MIRR: Symbol(40) = 0
    DRPNONA664: Symbol(39) = 0
    DRPDTAG: Symbol(38) = 0
    DRPSOTAG: Symbol(37) = 0
    DRPSITAG: Symbol(36) = 0
    DRPUNTAG: Symbol(35) = 0
    RETAG: Symbol(34) = 0
    DYN_LEARN: Symbol(33) = 0
    EGRESS: Symbol(32) = 0
    INGRESS: Symbol(31) = 0
    MIRRCIE: Symbol(30) = 0
    MIRRCETAG: Symbol(29) = 0
    INGMIRRVID: Symbol(28, 17) = 0
    INGMIRRPCP: Symbol(16, 14) = 0
    INGMIRRDEI: Symbol(13) = 0
    X: Symbol(4, 0) = 0xE


def ports_bits(excluding):
    return 0b11111 & ~(1 << excluding)


blocks = {}
blocks[0x6] = [
    L2PolicyRecord(SHARINDX=0, SMAX=-1, RATE=-1, MAXLEN=1518, PARTITION=0)
] * 45

blocks[0x8] = []
for i in range(MAX_PORTS):
    blocks[0x8].append(
        L2ForwardingRecord(
            BC_DOMAIN=ports_bits(i),
            REACH_PORT=ports_bits(i),
            FL_DOMAIN=ports_bits(i),
        )
    )
for i in range(8):
    blocks[0x8].append(L2ForwardingRecord())

blocks[0x9] = [
    MACConfigTable(TP_DELIN=1022, TP_DELOUT=130),
    MACConfigTable(TP_DELIN=1022, TP_DELOUT=130),
    MACConfigTable(TP_DELIN=1022, TP_DELOUT=130),
    MACConfigTable(TP_DELIN=1022, TP_DELOUT=130),
    MACConfigTable(TP_DELIN=1022, TP_DELOUT=129),
]

blocks[0xE] = [L2ForwardingParam(PART_SPC0=929)]

blocks[0x11] = [
    GeneralParameters(
        MAC_FLT1=-1,
        MAC_FLT0=-1,
        CASC_PORT=-1,
        HOST_PORT=-1,
        MIRR_PORT=-1,
        TPID=0x88A8,
        TPID2=0x8100,
    )
]

blocks[0x4E] = [
    XMIIModeParams(
        xMII_MAC0=1, xMII_MAC1=1, xMII_MAC2=1, xMII_MODE3=0b01, xMII_MODE4=0b10
    )
]

Writer().write("src/SJA1105.c", blocks)
