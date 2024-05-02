#!/usr/bin/python3
# SPDX-License-Identifier: MIT
#
# Trace stm32f303re ITM stim ports via openocd tcl socket
#
# On target write uint32_t to stim port eg:
#
#	ITM->PORT[12].u32 = 0x12345678;
#
# Trace output:
#
#     2.4845  +2.4845	0c: 78 56 34 12
#       elap     +oft	id: -- -- -- --
#
# Refer: https://github.com/robertlong13/SWO-Parser

import socket
import time
import sys

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect(('', 6666))

    # Enable trace target->SWO->openocd->tcl
    s.sendall(b'init\n\x1a')
    s.sendall(b'stm32f3x.tpiu configure -protocol uart\n\x1a')
    s.sendall(b'stm32f3x.tpiu configure -output -\n\x1a')
    s.sendall(b'stm32f3x.tpiu configure -traceclk 24000000\n\x1a')
    s.sendall(b'stm32f3x.tpiu enable\n\x1a')
    s.sendall(b'itm ports on\n\x1a')
    s.sendall(b'tcl_trace on\n\x1a')

    tty = sys.stdout.isatty()
    st = time.time()
    lt = st
    hdr = '      elap     +oft\tid: -- -- -- --'
    buf = ''
    try:
        if not tty:
            print(hdr)
        while True:
            data = s.recv(1024).decode('ascii')
            nt = time.time()
            if not data:
                break
            data = data.replace('\x1a', '').strip()
            if data.startswith('type target_trace data '):
                buf += data.replace('type target_trace data ', '')
                while len(buf) > 9:
                    itmport = int(buf[0:2], 16) >> 3
                    print('% 10.4f% +9.4f\t%02x: %s %s %s %s' %
                          (nt - st, nt - lt, itmport, buf[2:4], buf[4:6],
                           buf[6:8], buf[8:10]))
                    lt = nt
                    buf = buf[10:]
                if tty:
                    print(hdr, end='\r', flush=True)
            elif data:
                print('\t\t' + data)
    finally:
        # Disable target trace before closing the port
        s.sendall(b'stm32f3x.tpiu disable\n\x1a')
        s.sendall(b'itm ports off\n\x1a')
        s.sendall(b'tcl_trace off\n\x1a')
        s.shutdown(socket.SHUT_RDWR)
        s.close()
