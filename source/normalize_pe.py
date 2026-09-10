"""Remove the link timestamp and clear the optional PE checksum for repeatable builds."""
import pathlib
import struct
import sys

for name in sys.argv[1:]:
    path = pathlib.Path(name)
    data = bytearray(path.read_bytes())
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    assert data[:2] == b'MZ' and data[pe:pe + 4] == b'PE\0\0'
    assert struct.unpack_from('<H', data, pe + 24)[0] == 0x20b
    struct.pack_into('<I', data, pe + 8, 0)
    struct.pack_into('<I', data, pe + 24 + 64, 0)
    path.write_bytes(data)
