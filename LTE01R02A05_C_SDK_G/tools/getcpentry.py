#!/usr/bin/env python3
# Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
# All rights reserved.
#
# This software is supplied "AS IS" without any warranties.
# RDA assumes no responsibility or liability for the use of the software,
# conveys no license or title under any patent, copyright, or mask work
# right to the product. RDA reserves the right to make changes in the
# software without notification.  RDA also make no representation or
# warranty that such application will be suitable for the specified use
# without further testing or modification.

import os
import sys
import struct

def get_cp_entry(mem_list_file):
    with open(mem_list_file, 'rb') as fh:
        indata = fh.read()

    pos = 0
    insize = len(indata)
    while pos < insize:
        name, address, size, flags = struct.unpack(
            '20sIII', indata[pos:pos+32])
        pos += 32

        rname = name.rstrip(b'\x00').decode('utf-8')
        if 'cp_jump' == rname:
            print('%#x' % address)
            break

    if pos >= insize:
        print('')

    return


if __name__ == "__main__":
    sys.exit(get_cp_entry(sys.argv[1]))