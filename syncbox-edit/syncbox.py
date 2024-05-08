#!/usr/bin/python3
# SPDX-License-Identifier: MIT
"""syncbox-edit

Crude SysEx settings editor for syncbox

"""

from struct import pack_into
from math import gcd
from time import sleep
import sys
import argparse
import serial
import json
import os
import re
from mido import get_output_names, open_output, open_input, Message
from mido.ports import BaseOutput

# Manufacturer ID (0x7d) and Device ID (0x11)
SYSID = 0x1100007d

# Message Constants
COMMAND_GENERAL = 0x4
COMMAND_GENERALREQ = 0x14
COMMAND_OUTPUT = 0x5
COMMAND_OUTPUTREQ = 0x15

# Config Constants
FLAG_CLOCK = 1 << 0
FLAG_RUNSTOP = 1 << 1
FLAG_CONTINUE = 1 << 2
FLAG_NOTE = 1 << 3
FLAG_TRIG = 1 << 4
FLAG_CTRLDIV = 1 << 5
FLAG_CTRLOFT = 1 << 6
FLAG_CTRL = 1 << 7
FLAG_RUNMASK = 1 << 8
MODE_OMNION = 1
MODE_OMNIOFF = 3
DIVISOR_48PPQ = 1 << 1
DIVISOR_24PPQ = 2 << 1
DIVISOR_16TH = 12 << 1
DIVISOR_8TH = 24 << 1
DIVISOR_BEAT = 48 << 1
DIVISOR_BAR = 192 << 1

# Default filter: Common, Note On/Off, Controller and Realtime messages
FILTER_STD = 0x8b2c

# Output Numbers
OUTPUTNO = {
    'ck': 0,
    'rs': 1,
    'fl': 2,
    'g1': 3,
    'g2': 4,
    'g3': 5,
}

# Default Configuration - Equivalent to Preset 0
CONFIG = {
    'general': {
        'tempo': 120.0,
        'inertia': 5.0,
        'channel': 1,
        'mode': MODE_OMNION,
        'fusb': FILTER_STD,
        'fmidi': FILTER_STD,
        'triglen': 20,
    },
    'output': {
        'ck': {
            'flags': FLAG_CLOCK,
            'divisor': DIVISOR_24PPQ,
            'offset': 0,
            'note': 0,
        },
        'rs': {
            'flags': FLAG_RUNSTOP,
            'divisor': 0,
            'offset': 0,
            'note': 0,
        },
        'fl': {
            'flags': FLAG_CONTINUE,
            'divisor': 0,
            'offset': 0,
            'note': 0,
        },
        'g1': {
            'flags': FLAG_CLOCK | FLAG_TRIG | FLAG_RUNMASK,
            'divisor': DIVISOR_16TH,
            'offset': 0,
            'note': 0,
        },
        'g2': {
            'flags': FLAG_CLOCK | FLAG_TRIG | FLAG_RUNMASK,
            'divisor': 3 * DIVISOR_16TH,
            'offset': 0,
            'note': 0,
        },
        'g3': {
            'flags': FLAG_CLOCK | FLAG_TRIG | FLAG_RUNMASK,
            'divisor': DIVISOR_BEAT,
            'offset': 0,
            'note': 0,
        },
    }
}

# Speed of syncbox mpu (MHz)
FMPU = 24000000

# CRC7 lookup table
_CRC7MMC = (0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f, 0x48, 0x41, 0x5a,
            0x53, 0x6c, 0x65, 0x7e, 0x77, 0x19, 0x10, 0x0b, 0x02, 0x3d, 0x34,
            0x2f, 0x26, 0x51, 0x58, 0x43, 0x4a, 0x75, 0x7c, 0x67, 0x6e, 0x32,
            0x3b, 0x20, 0x29, 0x16, 0x1f, 0x04, 0x0d, 0x7a, 0x73, 0x68, 0x61,
            0x5e, 0x57, 0x4c, 0x45, 0x2b, 0x22, 0x39, 0x30, 0x0f, 0x06, 0x1d,
            0x14, 0x63, 0x6a, 0x71, 0x78, 0x47, 0x4e, 0x55, 0x5c, 0x64, 0x6d,
            0x76, 0x7f, 0x40, 0x49, 0x52, 0x5b, 0x2c, 0x25, 0x3e, 0x37, 0x08,
            0x01, 0x1a, 0x13, 0x7d, 0x74, 0x6f, 0x66, 0x59, 0x50, 0x4b, 0x42,
            0x35, 0x3c, 0x27, 0x2e, 0x11, 0x18, 0x03, 0x0a, 0x56, 0x5f, 0x44,
            0x4d, 0x72, 0x7b, 0x60, 0x69, 0x1e, 0x17, 0x0c, 0x05, 0x3a, 0x33,
            0x28, 0x21, 0x4f, 0x46, 0x5d, 0x54, 0x6b, 0x62, 0x79, 0x70, 0x07,
            0x0e, 0x15, 0x1c, 0x23, 0x2a, 0x31, 0x38, 0x41, 0x48, 0x53, 0x5a,
            0x65, 0x6c, 0x77, 0x7e, 0x09, 0x00, 0x1b, 0x12, 0x2d, 0x24, 0x3f,
            0x36, 0x58, 0x51, 0x4a, 0x43, 0x7c, 0x75, 0x6e, 0x67, 0x10, 0x19,
            0x02, 0x0b, 0x34, 0x3d, 0x26, 0x2f, 0x73, 0x7a, 0x61, 0x68, 0x57,
            0x5e, 0x45, 0x4c, 0x3b, 0x32, 0x29, 0x20, 0x1f, 0x16, 0x0d, 0x04,
            0x6a, 0x63, 0x78, 0x71, 0x4e, 0x47, 0x5c, 0x55, 0x22, 0x2b, 0x30,
            0x39, 0x06, 0x0f, 0x14, 0x1d, 0x25, 0x2c, 0x37, 0x3e, 0x01, 0x08,
            0x13, 0x1a, 0x6d, 0x64, 0x7f, 0x76, 0x49, 0x40, 0x5b, 0x52, 0x3c,
            0x35, 0x2e, 0x27, 0x18, 0x11, 0x0a, 0x03, 0x74, 0x7d, 0x66, 0x6f,
            0x50, 0x59, 0x42, 0x4b, 0x17, 0x1e, 0x05, 0x0c, 0x33, 0x3a, 0x21,
            0x28, 0x5f, 0x56, 0x4d, 0x44, 0x7b, 0x72, 0x69, 0x60, 0x0e, 0x07,
            0x1c, 0x15, 0x2a, 0x23, 0x38, 0x31, 0x46, 0x4f, 0x54, 0x5d, 0x62,
            0x6b, 0x70, 0x79)

# Symbol table symboltype: {symbol: value, ...},
SYMBOLS = {
    'mode': {
        'omni on': 1,
        'omni off': 3,
        'on': 1,
        'off': 3,
    },
    'duration': {
        '48ppq': DIVISOR_48PPQ,
        'korg': DIVISOR_48PPQ,
        '24ppq': DIVISOR_24PPQ,
        'roland': DIVISOR_24PPQ,
        '32nd': DIVISOR_16TH >> 1,
        '24th': 16,
        '16th': DIVISOR_16TH,
        'triplet': 32,
        '8th': DIVISOR_8TH,
        '6th': 64,
        'beat': DIVISOR_BEAT,
        'quarter': DIVISOR_BEAT,
        'half': DIVISOR_BEAT << 1,
        'bar': DIVISOR_BAR,
    },
    'flags': {
        'clock': FLAG_CLOCK,
        'runstop': FLAG_RUNSTOP,
        'continue': FLAG_CONTINUE,
        'note': FLAG_NOTE,
        'trig': FLAG_TRIG,
        'divisor': FLAG_CTRLDIV,
        'offset': FLAG_CTRLOFT,
        'controller': FLAG_CTRL,
        'run mask': FLAG_RUNMASK,
    },
    'filter': {
        'default': FILTER_STD,
        'system common': 0x2c,
        'misc function': 1 << 0x0,
        'cable events': 1 << 0x1,
        '2 byte system common': 1 << 0x2,
        '3 byte system common': 1 << 0x3,
        'sysex start': 1 << 0x4,
        '1 byte system common': 1 << 0x5,
        'sysex ends with 2 bytes': 1 << 0x6,
        'sysex ends with 3 bytes': 1 << 0x7,
        'note off': 1 << 0x8,
        'note on': 1 << 0x9,
        'poly pressure': 1 << 0xa,
        'controller': 1 << 0xb,
        'program change': 1 << 0xc,
        'channel pressure': 1 << 0xd,
        'bender': 1 << 0xe,
        'realtime': 1 << 0xf,
    },
}

# Option types, limits and symbol lookup
TYPEMAP = {
    'tempo': {
        'type': 'delay',
        'min': 1.0,
        'max': 500.0,
        'units': 'bpm',
        'places': 5,
    },
    'inertia': {
        'type': 'uptime',
        'min': 0,
        'max': 127 / 8,
        'units': 'ms',
    },
    'channel': {
        'type': 'int',
        'min': 1,
        'max': 16,
    },
    'mode': {
        'type': 'choice',
        'sym': 'mode',
        'options': (1, 3),
    },
    'fusb': {
        'type': 'bits',
        'sym': 'filter',
        'mask': 0x1fffff,
    },
    'fmidi': {
        'type': 'bits',
        'sym': 'filter',
        'mask': 0x1fffff,
    },
    'triglen': {
        'type': 'int',
        'min': 0,
        'max': 127,
        'sym': None,
        'units': 'ms',
    },
    'flags': {
        'type': 'bits',
        'sym': 'flags',
        'mask': 0x3fff,
    },
    'divisor': {
        'type': 'multiple',
        'min': 0,
        'max': 32766,
        'sym': 'duration',
    },
    'offset': {
        'type': 'multiple',
        'min': 0,
        'max': 16383,
        'sym': 'duration',
        'minmult': DIVISOR_16TH >> 1,
    },
    'note': {
        'type': 'int',
        'min': 0,
        'max': 127,
    },
}


class UartMidi(BaseOutput):
    """mido port wrapper for plain serial port"""

    def _open(self, **kwargs):
        self._uart = serial.Serial(self.name, 31250, timeout=1.0)

    def _send(self, message):
        self._uart.write(message.bytes())


def minmax(value, cfg):
    """Saturate value according to config"""
    if 'min' in cfg:
        value = max(value, cfg['min'])
    if 'max' in cfg:
        value = min(value, cfg['max'])
    return value


def intval(value, cfg):
    """Return saturated integer value"""
    if isinstance(value, str):
        if value.lower().startswith('0x'):
            value = int(value, 16)
        elif value.lower().startswith('0b'):
            value = int(value, 2)
        else:
            value = int(round(float(value)))
    else:
        value = int(round(value))
    return minmax(value, cfg)


def tokenmatch(token, symbols):
    """Match token to a known symbol"""
    token = re.sub('[\W_]+', '', token.lower())
    for t in symbols:
        if token == re.sub(r'[\W_]+', '', t):
            return t
    return None


def maskval(value, cfg):
    """Return value masked according to config"""
    if 'mask' in cfg:
        value = value & cfg['mask']
    return value


def bitval(value, cfg):
    """Return a masked bitfield value according to config"""
    if isinstance(value, str):
        sym = {}
        if 'sym' in cfg:
            sym = SYMBOLS[cfg['sym']]
        nv = 0
        for token in value.lower().split('|'):
            tk = tokenmatch(token, sym)
            tv = 0
            if tk in sym:
                tv = sym[tk]
            else:
                tv = intval(token, cfg)
            nv |= tv
        value = nv
    return maskval(value, cfg)


def valbit(value, cfg):
    """Return an exploded bitfield string according to cfg"""
    rv = []
    sym = {}
    if 'sym' in cfg:
        sym = SYMBOLS[cfg['sym']]
    for token in sym:
        if value & sym[token] == sym[token]:
            rv.append(token)
            value &= ~sym[token]
    if value:
        rv.append(hex(value))
    if rv:
        return ' | '.join(rv)
    else:
        return 0


def intopt(value, cfg):
    """Return a value from a set of options"""
    iv = intval(value, cfg)
    if 'options' in cfg:
        if iv in cfg['options']:
            return iv
        else:
            return cfg['options'][0]
    else:
        return iv


def choiceval(value, cfg):
    """Select a value from a set of options"""
    if isinstance(value, str):
        sym = {}
        if 'sym' in cfg:
            sym = SYMBOLS[cfg['sym']]
        tv = value
        token = tokenmatch(value, sym)
        if token:
            tv = sym[token]
        value = tv
    return intopt(value, cfg)


def valchoice(value, cfg):
    """Return a symbolic representation of the value"""
    minmult = 0
    sym = {}
    if 'sym' in cfg:
        sym = SYMBOLS[cfg['sym']]
    if 'minmult' in cfg:
        minmult = cfg['minmult']
    if value >= minmult:
        for token in sym:
            if value == sym[token]:
                return token
    return value


def multipleval(value, cfg):
    """Return a product from the string value: n unit"""
    if isinstance(value, str):
        sv = value.split(maxsplit=1)
        if len(sv) == 2:
            mult = float(sv[0])
            unit = choiceval(sv[1], cfg)
            return intval(mult * unit, cfg)
        else:
            return choiceval(value, cfg)
    else:
        return intval(value, cfg)


def valmultiple(value, cfg):
    """Return a product string for delay/offset value: n unit"""
    if value == 0:
        return 0
    elif value == 2 or value == 4:
        return valchoice(value, cfg)
    else:
        unit = gcd(DIVISOR_BAR, value)
        if unit > 5:
            token = valchoice(unit, cfg)
            if unit == token:
                # gcd unit was not matched
                return value
            else:
                count = value // unit
                if count > 1:
                    return '%d %s' % (value // unit, token)
                else:
                    return token
        else:
            return value


def unitsval(value, cfg):
    """Strip units label from value"""
    if 'units' in cfg:
        if isinstance(value, str):
            value = value.removesuffix(cfg['units']).strip()
    return value


def valunits(value, cfg):
    """Append units string to value"""
    if 'units' in cfg:
        value = str(value) + ' ' + cfg['units']
    return value


def valfloat(value, cfg):
    """Convert float value to string rounded to desired places"""
    if isinstance(value, float):
        if 'places' in cfg:
            value = round(value, cfg['places'])
        value = '{0:g}'.format(value)
    return value


def parseval(key, value):
    """Read key:value according to TYPEMAP"""
    if key not in TYPEMAP or 'type' not in TYPEMAP[key]:
        return value
    cfg = TYPEMAP[key]
    itype = cfg['type']
    if itype == 'float':
        return minmax(float(unitsval(value, cfg)), cfg)
    elif itype == 'uptime':
        return uptimems(msuptime(minmax(float(unitsval(value, cfg)), cfg)))
    elif itype == 'delay':
        return delaytempo(tempodelay(minmax(float(unitsval(value, cfg)), cfg)))
    elif itype == 'int':
        return intval(unitsval(value, cfg), cfg)
    elif itype == 'bits':
        return bitval(value, cfg)
    elif itype == 'choice':
        return choiceval(value, cfg)
    elif itype == 'multiple':
        return multipleval(value, cfg)
    else:
        return value


def unparseval(key, value):
    """Return value for export according to TYPEMAP"""
    if key not in TYPEMAP or 'type' not in TYPEMAP[key]:
        return value
    cfg = TYPEMAP[key]
    itype = cfg['type']
    if itype == 'bits':
        return valbit(value, cfg)
    elif itype == 'choice':
        return valchoice(value, cfg)
    elif itype == 'multiple':
        return valmultiple(value, cfg)
    elif itype in ['float', 'delay', 'uptime']:
        return valunits(valfloat(value, cfg), cfg)
    else:
        return valunits(value, cfg)


def crc7mmc(msgstr=b''):
    """Return CRC7/MMC over elements of msgstr"""
    r = 0x0
    for b in msgstr:
        i = (r << 1) ^ b
        r = _CRC7MMC[i] & 0x7f
    return r


def mk_generalreq():
    """Return a SysEx general config request"""
    msg = bytearray(8)
    msg[0] = 0xf0
    pack_into('<L', msg, 1, SYSID)
    msg[5] = COMMAND_GENERALREQ
    msg[6] = crc7mmc(msg[1:6])
    msg[7] = 0xf7
    return msg


def tempodelay(bpm):
    """Return a FMPU delay for the provided tempo"""
    return int(round(FMPU * 60 / (96 * bpm)))


def delaytempo(delay):
    """Return a tempo for the provided FMPU delay"""
    return 60 * FMPU / (96 * delay)


def msuptime(ms):
    """Return uptime count for the given milliseconds"""
    return int(round(8 * ms))


def uptimems(uptime):
    """Return millisecond value for given uptime"""
    return uptime / 8


def mk_general(cfg):
    """Return a SysEx general config for the provided config object"""
    msg = bytearray(23)
    msg[0] = 0xf0
    pack_into('<L', msg, 1, SYSID)
    msg[5] = COMMAND_GENERAL

    delay = tempodelay(cfg['tempo'])
    msg[6] = delay & 0x7f
    msg[7] = (delay >> 7) & 0x7f
    msg[8] = (delay >> 14) & 0x7f
    msg[9] = (delay >> 21) & 0x7f
    msg[10] = msuptime(cfg['inertia']) & 0x7f
    msg[11] = (cfg['channel'] - 1) & 0x0f
    msg[12] = cfg['mode'] & 0x0f
    msg[14] = cfg['fusb'] & 0x7f
    msg[15] = (cfg['fusb'] >> 7) & 0x7f
    msg[16] = (cfg['fusb'] >> 14) & 0x7f
    msg[17] = cfg['fmidi'] & 0x7f
    msg[18] = (cfg['fmidi'] >> 7) & 0x7f
    msg[19] = (cfg['fmidi'] >> 14) & 0x7f
    msg[20] = cfg['triglen'] & 0x7f
    msg[21] = crc7mmc(msg[1:21])
    msg[22] = 0xf7
    return msg


def unmk_general(cfg):
    """Read a SysEx general config and return a config object"""
    cr = None
    if len(cfg) == 21:
        sysid = cfg[0] | cfg[1] << 8 | cfg[2] << 16 | cfg[3] << 24
        cmd = cfg[4]
        crc = crc7mmc(cfg[0:20])
        if sysid == SYSID and crc == cfg[20] and cmd == 0x4:
            cr = {}
            delay = cfg[5] | cfg[6] << 7 | cfg[7] << 14 | cfg[8] << 21
            cr['tempo'] = delaytempo(delay)
            cr['inertia'] = uptimems(cfg[9])
            cr['channel'] = (cfg[10] & 0xf) + 1
            cr['mode'] = cfg[11] & 0xf
            cr['fusb'] = cfg[13] | cfg[14] << 7 | cfg[15] << 14
            cr['fmidi'] = cfg[16] | cfg[17] << 7 | cfg[18] << 14
            cr['triglen'] = cfg[19]
        else:
            print('Warning: Received invalid general configuration',
                  file=sys.stderr)
    return cr


def mk_outputreq(onum):
    """Return a SysEx output config request"""
    cfg = bytearray(9)
    cfg[0] = 0xf0
    pack_into('<L', cfg, 1, SYSID)
    cfg[5] = COMMAND_OUTPUTREQ
    cfg[6] = onum & 0x07
    cfg[7] = crc7mmc(cfg[1:7])
    cfg[8] = 0xf7
    return cfg


def mk_output(oc, onum):
    """Return a SysEx output config for the config object and output no"""
    cfg = bytearray(16)
    cfg[0] = 0xf0
    pack_into('<L', cfg, 1, SYSID)
    cfg[5] = COMMAND_OUTPUT
    cfg[6] = onum & 0x07
    cfg[7] = oc['flags'] & 0x7f
    cfg[8] = (oc['flags'] >> 7) & 0x7f
    div = oc['divisor'] >> 1
    cfg[9] = div & 0x7f
    cfg[10] = (div >> 7) & 0x7f
    cfg[11] = oc['offset'] & 0x7f
    cfg[12] = (oc['offset'] >> 7) & 0x7f
    cfg[13] = oc['note'] & 0x7f
    cfg[14] = crc7mmc(cfg[1:14])
    cfg[15] = 0xf7
    return cfg


def unmk_output(cfg, onum):
    """Read a SysEx output config and return a config object"""
    cr = None
    if len(cfg) == 14:
        sysid = cfg[0] | cfg[1] << 8 | cfg[2] << 16 | cfg[3] << 24
        cmd = cfg[4]
        crc = crc7mmc(cfg[0:13])
        if sysid == SYSID and crc == cfg[13] and cmd == 0x5 and onum == cfg[5]:
            cr = {}
            cr['flags'] = cfg[6] | cfg[7] << 7
            cr['divisor'] = (cfg[8] | cfg[9] << 7) << 1
            cr['offset'] = cfg[10] | cfg[11] << 7
            cr['note'] = cfg[12]
        else:
            print('Warning: Received invalid output configuration',
                  file=sys.stderr)
    return cr


def load_config(filename):
    """Return file config merged with defaults and separate file config"""
    cr = {'general': {}, 'output': {}}
    incfg = {}
    if os.path.exists(filename):
        with open(filename) as f:
            incfg = json.load(f)

    for k in CONFIG['general']:
        nv = None
        if 'general' in incfg and k in incfg['general']:
            nv = incfg['general'][k]
        else:
            nv = CONFIG['general'][k]
        cr['general'][k] = parseval(k, nv)

    for output in CONFIG['output']:
        cr['output'][output] = {}
        dst = cr['output'][output]
        src = CONFIG['output'][output]
        lsrc = None
        if 'output' in incfg and output in incfg['output']:
            lsrc = incfg['output'][output]
        for k in src:
            nv = None
            if lsrc and k in lsrc:
                nv = lsrc[k]
            else:
                nv = src[k]
            dst[k] = parseval(k, nv)
    return cr, incfg


def save_config(config, optref, file, indent=1):
    """Write values from config with defined optref keys to file"""
    cr = {}
    if 'general' in optref and 'general' in config:
        cr['general'] = {}
        for k in optref['general']:
            cr['general'][k] = unparseval(k, config['general'][k])
    if 'output' in optref and 'output' in config:
        oc = {}
        for output in optref['output']:
            if output in config['output']:
                oc[output] = {}
                src = config['output'][output]
                for k in src:
                    oc[output][k] = unparseval(k, src[k])
        if oc:
            cr['output'] = oc
    json.dump(cr, file, indent=indent)


def mkoptref(include, cfg):
    """Construct an optref object from command line include options"""
    sections = include.lower().split(',')
    ocfg = {}
    seckeys = ('general', 'ck', 'rs', 'fl', 'g1', 'g2', 'g3')
    req = set()
    for sec in sections:
        sec = re.sub('[\W_]+', '', sec)
        if sec == 'all':
            return CONFIG
        elif sec == 'src':
            return cfg
        else:
            if sec in seckeys:
                req.add(sec)
    if 'general' in req:
        ocfg['general'] = CONFIG['general']
        req.remove('general')
        if 'general' not in cfg:
            pass  # not important
    if req:
        ocfg['output'] = {}
        for out in CONFIG['output']:
            if out in req:
                ocfg['output'][out] = CONFIG['output'][out]
                if 'output' not in cfg or out not in cfg['output']:
                    pass  # not important

    return ocfg


def send_request(oport, iport, optref):
    """Ask device to send all sections in optref"""
    while iport.poll():
        pass
    cfg = {}
    if 'general' in optref:
        msg = Message.from_bytes(mk_generalreq())
        print('Requesting general configuration', file=sys.stderr)
        oport.send(msg)
        count = 0
        while count < 10:
            msg = iport.poll()
            if msg and msg.type == 'sysex' and len(msg) == 23:
                gc = unmk_general(msg.data)
                if gc is not None:
                    cfg['general'] = gc
                    break
            count += 1
            sleep(0.01)
        if 'general' not in cfg:
            raise RuntimeError('Timeout waiting for SysEx reply')

    if 'output' in optref:
        cfg['output'] = {}
        for output in optref['output']:
            ocno = OUTPUTNO[output]
            msg = Message.from_bytes(mk_outputreq(ocno))
            print('Requesting %s output configuration' % (output, ),
                  file=sys.stderr)
            oport.send(msg)
            count = 0
            while count < 10:
                msg = iport.poll()
                if msg:
                    if msg and msg.type == 'sysex' and len(msg) == 16:
                        oc = unmk_output(msg.data, ocno)
                        if oc is not None:
                            cfg['output'][output] = oc
                            break
                count += 1
                sleep(0.01)
            if output not in cfg['output']:
                raise RuntimeError('Timeout waiting for SysEx reply')
    return cfg


def send_config(port, cfg, optref):
    """Send values from cfg with defined keys in optref to device"""
    if 'general' in optref:
        msg = Message.from_bytes(mk_general(cfg['general']))
        print('Sending general configuration', file=sys.stderr)
        port.send(msg)
    if 'output' in optref:
        for output in optref['output']:
            oc = cfg['output'][output]
            ocno = OUTPUTNO[output]
            msg = Message.from_bytes(mk_output(oc, ocno))
            print('Sending %s output configuration' % (output, ),
                  file=sys.stderr)
            port.send(msg)


def chomptok(toklist):
    """Pop the first element of toklist, or return None if empty"""
    if toklist:
        return toklist.pop(0).strip()
    else:
        return None


def parseset(setting):
    """Decode -e sec.key=value from command line"""
    s = None
    o = None
    k = None
    v = None
    kv = setting.split('=')
    if len(kv) == 2:
        val = kv[1].strip()
        kv = kv[0].lower().split('.')
        tok = chomptok(kv)
        if tok in ['output', 'general']:
            tok = chomptok(kv)
        if tok in CONFIG['general']:
            s = 'general'
            k = tok
            v = parseval(k, val)
        elif tok in CONFIG['output']:
            o = tok
            tok = chomptok(kv)
            if tok in CONFIG['output']['ck']:
                s = 'output'
                k = tok
                v = parseval(k, val)
    return s, o, k, v


def setoptions(settings, cfg, optref):
    """Update cfg and optref with all -e command line settings"""
    if isinstance(settings, list):
        for setting in settings:
            s, o, k, v = parseset(setting)
            if s is not None:
                # ensure section exists
                if s not in cfg:
                    cfg[s] = {}
                if s not in optref:
                    optref[s] = {}
                if s == 'general':
                    # overwrite value
                    cfg[s][k] = v
                    optref[s][k] = v
                elif s == 'output':
                    if o not in cfg[s]:
                        cfg[s][o] = {}
                    if o not in optref[s]:
                        optref[s][o] = {}
                    # overwrite value
                    cfg[s][o][k] = v
                    optref[s][o][k] = v
            else:
                raise RuntimeError('Invalid setting %r' % (setting, ))


def mergeconf(src, dst, optref):
    """Overwrite dst values with values from src that appear in optref"""
    # assumes src and dst hold sections and keys already
    if 'general' in optref:
        for k in src['general']:
            if k in optref['general']:
                dst['general'][k] = src['general'][k]
    if 'output' in optref:
        for output in src['output']:
            if output in optref['output']:
                for k in optref['output'][output]:
                    dst['output'][output][k] = src['output'][output][k]


def main():
    parser = argparse.ArgumentParser(prog='syncbox-edit')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('-c',
                       '--create',
                       action='store_true',
                       help='create a default configuration')
    group.add_argument('-s',
                       '--send',
                       action='store_true',
                       help='send configuration to device')
    group.add_argument('-r',
                       '--receive',
                       action='store_true',
                       help='receive configuration from device')
    group.add_argument('-u',
                       '--update',
                       action='store_true',
                       help='update configuration on device')
    parser.add_argument('-l',
                        '--list',
                        action='store_true',
                        help='list available MIDI ports')
    group = parser.add_mutually_exclusive_group()
    group.add_argument(
        '-p',
        dest='port',
        help='MIDI port name',
    )
    group.add_argument('-t', dest='uart', help='UART port name')
    parser.add_argument('-i',
                        dest='include',
                        default='src',
                        metavar='INC',
                        help='specify sections to include eg: -i general,ck')
    parser.add_argument('-e',
                        dest='set',
                        help='specify setting eg: -e general.tempo=120bpm',
                        action='append')
    parser.add_argument('file',
                        nargs='?',
                        help='JSON settings file',
                        default='')
    args = parser.parse_args()
    cfg = CONFIG
    optref = {}

    if args.list:
        try:
            portlist = []
            for p in get_output_names():
                p = "\t'" + p.strip() + "'"
                if p not in portlist:
                    portlist.append(p)
            print('Available MIDI Ports:\n',
                  '\n'.join(portlist),
                  file=sys.stderr)
        except Exception as e:
            print('Error listing MIDI ports:', e, file=sys.stderr)
            return -1
        return 0
    if args.receive:
        try:
            if args.uart:
                print('Error: UART port not supported for receive mode',
                      file=sys.stderr)
                return -1
            cfg = {}
            optref = mkoptref(args.include, CONFIG)
            with open_input(args.port) as ip:
                with open_output(args.port) as op:
                    cfg = send_request(op, ip, optref)
            if args.file == '-':
                args.file = ''
            ofile = sys.stdout
            if args.file:
                ofile = open(args.file, 'w')
            save_config(cfg, optref, ofile)
            if ofile is not sys.stdout:
                print('Wrote default configuration to:',
                      args.file,
                      file=sys.stderr)
            if ofile is sys.stdout and sys.stdout.isatty():
                print()
            ofile.close()
        except Exception as e:
            print('Error reading configuration:', e, file=sys.stderr)
            return -1
    elif args.send:
        try:
            if args.file:
                if not os.path.exists(args.file):
                    print('Error: Configuration file not found',
                          file=sys.stderr)
                    return -1
                cfg, optref = load_config(args.file)
            else:
                pass  # using defaults
            optref = mkoptref(args.include, optref)
            setoptions(args.set, cfg, optref)
            mp = None
            if args.uart:
                # use mido wrapped serial port
                mp = UartMidi(args.uart)
            else:
                mp = open_output(args.port)
            send_config(mp, cfg, optref)
        except Exception as e:
            print('Error sending configuration:', e, file=sys.stderr)
            return -1
    elif args.update:
        try:
            if args.uart:
                print('Error: UART port not supported for update mode',
                      file=sys.stderr)
                return -1
            fcfg = {}
            if args.file:
                if not os.path.exists(args.file):
                    print('Error: Configuration file not found',
                          file=sys.stderr)
                    return -1
                fcfg, optref = load_config(args.file)
            else:
                optref = {}
            optref = mkoptref(args.include, optref)

            # flag cmd line updates in optref
            setoptions(args.set, cfg, optref)

            with open_input(args.port) as ip:
                with open_output(args.port) as op:
                    cfg = send_request(op, ip, optref)
                    if 'general' in cfg or ('output' in cfg and cfg['output']):
                        if fcfg:
                            mergeconf(fcfg, cfg, optref)
                        # overwrite file config with cmd line values
                        setoptions(args.set, cfg, optref)
                        send_config(op, cfg, optref)
        except Exception as e:
            print('Error updating configuration:', e, file=sys.stderr)
            return -1
    elif args.create:
        try:
            if args.file == '-':
                args.file = ''
            ofile = sys.stdout
            if args.file:
                ofile = open(args.file, 'w')
            optref = mkoptref(args.include, CONFIG)
            setoptions(args.set, cfg, optref)
            save_config(cfg, optref, ofile)
            if ofile is not sys.stdout:
                print('Wrote configuration to:', args.file, file=sys.stderr)
            if ofile is sys.stdout and sys.stdout.isatty():
                print()
            ofile.close()
        except Exception as e:
            print('Error creating configuration:', e, file=sys.stderr)
            return -1
    else:
        parser.print_usage()
        return -1
    return 0


if __name__ == '__main__':
    sys.exit(main())
