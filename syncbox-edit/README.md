# syncbox-edit

Read, write and update syncbox configuration.


## Usage

	$ syncbox-edit -h
	usage: syncbox-edit [-h] [-l] [-c | -s | -r | -u] [-p PORT | -t UART] [-i INC]
	                    [-e SET]
	                    [file]
	...

Option -i specifies a comma-separated list of sections that
apply to the operating mode. Available options are:

   - general: General configuration
   - ck: ck output
   - rs: rs output
   - fl: fl output
   - g1: g1 output
   - g2: g2 output
   - g3: g3 output
   - src: select all sections present in input file (default)
   - all: select all configuration sections

Option -e enables override of a specific setting using
the form: section.key=value and may be specified multiple
times. Section may be omitted for general
configuration settings. Recognised settings are:

   - general.tempo: Set internal tempo in bpm
   - general.inertia: Set run/stop inertia in ms
   - general.channel: Set MIDI basic channel, 1-16
   - general.mode: Set MIDI mode "omni on" or "omni off"
   - general.fusb: USB cable event filter
   - general.fmidi: MIDI cable event filter
   - general.triglen: Trigger length in ms

   - OUT.flags: Output flags
   - OUT.divisor: Output clock divisor (96ppq units)
   - OUT.offset: Output clock offset (96ppq units)
   - OUT.note: Output MIDI note/control number

Where OUT is one of ck, rs, fl, g1, g2, g3.

Operating modes: list, create, send, receive and update
are described below.


### -l, --list : List Available MIDI Ports

Example:

	$ syncbox-edit -l
	Available MIDI Ports:
	 	'UM-ONE:UM-ONE MIDI 1 20:0'
		'Syncbox:Syncbox Sync 24:0'


### -c, --create : Create Configuration File

Create a configuration containing the selected sections
and write it to the nominated filename or stdout if file
is omitted.

Example: Write a complete default configuration to file "config.json"

	$ syncbox-edit -c config.json
	Wrote default configuration to: config.json

Example: Create an output g3 configuration, overriding divisor and flags

	$ syncbox-edit -c -i g3 -e g3.flags=clock -e g3.divisor=beat
	{
	 "output": {
	  "g3": {
	   "flags": "clock",
	   "divisor": "1 beat",
	   "offset": 0,
	   "note": 0
	  }
	 }
	}


### -s, --send : Send configuration to device

Send the sections specified to the device over SysEx. If an input
file is provided, all sections in the file will be sent along
with any sections explicitly included on the command line. If no
file is provided, only those sections listed will be sent.

Example: Send all sections in "config.json"

	$ syncbox-edit -s config.json

Example: Send a complete default configuration

	$ syncbox-edit -s -i all

Example: Send output g1 from "config.json", along
with an amendment to the offset

	$ syncbox-edit -s -i g1 -e g1.offset=3 config.json


### -r, --receive : Receive Configuration

Request the nominated sections from an attached syncbox
and output them to a configuration file. 

Example: Receive output g2 configuration from syncbox:

	$ syncbox-edit -r -i g2
	{
	 "output": {
	  "g2": {
	   "flags": "note",
	   "divisor": 0,
	   "offset": 0,
	   "note": 64
	  }
	 }
	}


### -u, --update : Update Configuration

Update mode reads the requested sections as per receive mode
then sets any required values and sends the resulting configuration
as per send mode.

Example: Alter output g3 divisor

	$ ./syncbox.py -u -i g3 -e g3.divisor=6


### -p, -t : Select MIDI or UART port

By default, the first MIDI device will be used. To specify
a different device, use the name provided by a list output.

Example: Receive configuration from a specific MIDI port

	$ syncbox-edit -r -p 'Syncbox:Syncbox Sync 24:0'

Note: receive and update modes only work over the syncbox
USB MIDI interface. When using the MIDI interface, SysEx
requests will time out.

To send updates over a serial port, use the -t option
with mode --send to specify the serial port device.

Example: Send a default configuration via USB serial adapter

	$ syncbox-edit -s -i all -t /dev/ttyUSB0


## Configuration File Format

Device configuration is saved as a JSON encoded object with two
sections: 'general' and 'output'. General device configuration
is a map containing the following keys and values:

   - tempo (string): Internal clock tempo in bpm 10.0-500.0, default:
     "120.000 bpm"
   - inertia (string): Run/stop inertia in ms 0.0-12.7, default: "5.0 ms"
   - channel (int): Basic MIDI Channel 1-16, default: 1
   - mode (string): MIDI Mode "omni on"(1) or "omni off"(3), default:
     "omni on"
   - fusb (string): 21 bit USB cable filter bitmask, default:
     "system common | note off | note on | controller | realtime"
   - musb (string): 21 bit MIDI cable filter bitmask, default:
     "system common | note off | note on | controller | realtime"
   - triglen (string): Trigger length in ms 0-127, default: "20 ms"

The output section contains an output configuration object for each
of the outputs:

   - ck: DIN pin 3 "clock"
   - rs: DIN pin 1 "run/stop"
   - fl: DIN pin 5 "continue" or "fill"
   - g1: Gate output 1
   - g2: Gate output 2
   - g3: Gate output 3

Each output configuration object has the following keys and values:

   - flags (string): 14 bit output flags or integer value
   - divisor (string): 96ppq sync clock divisor 0-32766 *
   - offset (string): 96ppq clock offset 0-16383
   - note (int): MIDI note or controller

Note: Divisor is halved before sending to syncbox, so odd values
will be effectively truncated to even.


### MIDI Cable Filter Flags

   - "default": 0x8b2c (common, note, controller, realtime)
   - "system common": 0x2c (includes 1, 2 and 3 byte common)
   - "misc function": 0x1
   - "cable events": 0x2
   - "2 byte system common": 0x4
   - "3 byte system common": 0x8
   - "sysex start": 0x10
   - "1 byte system common": 0x20
   - "sysex ends with 2 bytes": 0x40
   - "sysex ends with 3 bytes": 0x80
   - "note off": 0x100
   - "note on": 0x200
   - "poly pressure": 0x400
   - "controller": 0x800
   - "program change": 0x1000
   - "channel pressure": 0x2000
   - "bender": 0x4000
   - "realtime": 0x8000

Note: Misc function, cable events and SysEx messages are not filtered
by the syncbox, these flags and undefined bits are ignored.

### Output Flags

The behaviour of output pins is determined by a combination
of the following flags. Where states overlap, the output
behaviour is determined by the order of reception of
MIDI messages.

   - "clock": Output is switched on and off by the internal 96ppq
     reference according to the divisor and offset configuration
     (subject to inertia setting)
   - "runstop": Output is set on reception of a MIDI start message
     and cleared on reception of a MIDI stop (subject to inertia
     setting)
   - "continue": Output is set on reception of a MIDI continue message
   - "note": Output is set and cleared on reception of Note on/off
     messages matching the configured note value
   - "trig": Output is cleared triglen milliseconds after being set
   - "divisor": The output's divisor value is adjusted on reception of a
     controller matching the configured note value
   - "offset": The output's offset value is adjusted on reception of a
     controller matching the configured note value
   - "controller": The output is set when a matching controller is
     received with a value of 64 or greater, and cleared if the value
     is less than 64.
   - "run mask": The output is held clear while stopped


### Divisor/Offset Labels

Divisors and offsets are specified as multiples of the
internal 96ppq reference clock. The following flags are
available to set standard tempo durations:

   - "48ppq": 2
   - "korg": 2 (alias of "48ppq")
   - "24ppq": 4
   - "roland": 4 (alias of "24ppq")
   - "32nd": 12
   - "24th": 16
   - "16th": 24
   - "triplet": 32
   - "8th": 48
   - "6th": 64
   - "beat": 96
   - "quarter": 96 (alias of "beat")
   - "half": 192
   - "bar": 384

Multiples are specified by prefixing a label with an integer
multiplier and a space, eg: "3 16th" is equivalent to 72.


## Installation

MIDI device support is provided via the mido library. Install a
[mido compatible backend](https://mido.readthedocs.io/en/latest/backends/index.html)
and then install syncbox-edit via pip in a venv.
For example, a Debian system with rtmidi backend:

	# apt-get install python3-rtmidi python3-mido
	$ python3 -m venv --system-site-packages DIR
	$ DIR/bin/pip install syncbox-edit
	$ DIR/bin/syncbox-edit -h

