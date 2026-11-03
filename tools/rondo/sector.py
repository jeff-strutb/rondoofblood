"""CD-ROM MODE1 sector encoding: sync, header, EDC and the P and Q parity.

A MODE1 sector is 2,352 bytes: 12 bytes of sync, a 4-byte header (the
sector's address as BCD minutes, seconds and frames, then the mode), 2,048
bytes of user data, a 4-byte EDC (a CRC over everything before it), 8 zero
bytes, then 172 bytes of P parity and 104 of Q parity (Reed-Solomon codes
over GF(2^8) across the header, data, EDC and zero bytes, ECMA-130 annex A).
"""

RAW = 2352
USER = 2048
SYNC = b"\x00" + b"\xff" * 10 + b"\x00"


def _tables():
    edc = []
    for i in range(256):
        r = i
        for _ in range(8):
            r = (r >> 1) ^ 0xD8018001 if r & 1 else r >> 1
        edc.append(r)
    f = [0] * 256
    b = [0] * 256
    for i in range(256):
        j = ((i << 1) ^ (0x11D if i & 0x80 else 0)) & 0xFF
        f[i] = j
        b[i ^ j] = i
    return edc, f, b


EDC_LUT, ECC_F, ECC_B = _tables()


def edc(data):
    r = 0
    for x in data:
        r = EDC_LUT[(r ^ x) & 0xFF] ^ (r >> 8)
    return r


def _ecc_block(s, base, major_count, minor_count, major_mult, minor_inc, dest):
    size = major_count * minor_count
    f, b = ECC_F, ECC_B
    for major in range(major_count):
        index = (major >> 1) * major_mult + (major & 1)
        a = e = 0
        for _ in range(minor_count):
            t = s[base + index]
            index += minor_inc
            if index >= size:
                index -= size
            a ^= t
            e ^= t
            a = f[a]
        a = b[f[a] ^ e]
        s[dest + major] = a
        s[dest + major + major_count] = a ^ e


def bcd(v):
    return ((v // 10) << 4) | (v % 10)


def msf(lba):
    """Header address of absolute sector LBA (MSF = LBA + 150 frames)."""
    fr = lba + 150
    return bytes((bcd(fr // (75 * 60)), bcd((fr // 75) % 60), bcd(fr % 75)))


def encode_mode1(lba, user):
    """The 2,352-byte MODE1 sector at absolute LBA holding these 2,048 bytes."""
    assert len(user) == USER
    s = bytearray(RAW)
    s[0:12] = SYNC
    s[12:15] = msf(lba)
    s[15] = 1
    s[16:16 + USER] = user
    s[0x810:0x814] = edc(s[0:0x810]).to_bytes(4, "little")
    _ecc_block(s, 0xC, 86, 24, 2, 86, 0x81C)   # P
    _ecc_block(s, 0xC, 52, 43, 86, 88, 0x8C8)  # Q
    return bytes(s)
