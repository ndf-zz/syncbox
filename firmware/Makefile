# SPDX-License-Identifier: MIT
#
# STM32F303 USB/MIDI to DIN/Gate converter
#

# Project  Basename
PROJECT = syncbox

# Firmware Version
VERSION = 20240401

# SYSEX Manufacturer and device ID
SYSID = 0x1100007d

# Conditional compilation options

# Define chip type for STM headers
CPPFLAGS = -DSTM32F303xE
# Blink note LED on reception of matching Note-Off message
#CPPFLAGS += -DNOTE_OFF_BLINK
# Enable sleep on exit from handler
CPPFLAGS += -DENABLE_SLEEP
# Lock code memory pages in system_init() - Requires power on reset to clear
#CPPFLAGS += -DLOCK_CODE_MEMORY
# Lock GPIO ports in system_init() - Requires power on reset to clear
#CPPFLAGS += -DLOCK_GPIO
# Enable IWDG in user interface handler [prod]
#CPPFLAGS += -DUSE_IWDG
# Include __BKPT instruction in build [debug]
CPPFLAGS += -DUSE_BKPT
# Include ITM Trace calls in build [debug]
CPPFLAGS += -DUSE_TRACE

# Build Objects

# system_init(), reset and fault handers [required]
OBJECTS += src/system.o

# Cortex-M4 vector table [required]
OBJECTS += src/vtable.o

# Extra build objects
OBJECTS += src/main.o
OBJECTS += src/flash.o
OBJECTS += src/midi_event.o
OBJECTS += src/midi_uart.o
OBJECTS += src/midi_usb.o
OBJECTS += src/display.o
OBJECTS += src/settings.o
OBJECTS += src/timer.o

# Link script
LINKSCRIPT = include/stm32f303xe_ram.ld

# Use STLINK programmer attached to target on Nucleo board
#OCDCONFIG = include/openocd-nucleo.cfg

# Stand alone board with SWD/STLINKv2
OCDCONFIG = include/openocd-stlinkv2.cfg

# Target binary
TARGET = $(PROJECT).elf

# Flash memory loader
LOADELF = loader.elf
LOADSRC = src/loader.s
LOADSCRIPT = include/loader.ld

# ROM Options
OPTIONS = options.elf

# Listing files
TARGETLIST = $(TARGET:.elf=.lst)

# Compiler
CC = arm-none-eabi-gcc

# Include debugging symbols
DEBUG = -g

# Set language standard and bare metal environment (note: ARM CMSIS is C99)
DIALECT = -std=c99 -pedantic -ffreestanding

# General code optimisation level
OPTIMISE = -Os
# Mark functions and data so linker can omit dead sections
OPTIMISE += -ffunction-sections -fdata-sections
# Perform link-time optimisation
OPTIMISE += -flto

# Treat all warnings as errors
WARN = -Werror
# Warn on questionable constructs
WARN += -Wall
# Extra warning flags not enabled by -Wall
WARN += -Wextra
# Warn whenever a local variable or type declaration shadows another ...
WARN += -Wshadow
# Warn if an undefined identifier is evaluated in an #if directive
WARN += -Wundef
# Warn for implicit conversions that may alter a value [annoying]
WARN += -Wconversion

# Arm cpu type (implies endian and arch)
ARMOPTS = -mcpu=cortex-m4
# Generate thumb code
ARMOPTS += -mthumb

# Add include path for headers
CPPFLAGS += -Iinclude
# Sysex manufacturer and device ID
CPPFLAGS += -DSYSEX_ID=$(SYSID)
# Firmware version number
CPPFLAGS += -DSYSTEMVERSION=$(VERSION)

# Padding value for unused program space - default value fills program
# space with invalid Thumb instruction: 0xdede
PROGPAD = 0xde

# Extra libraries
LDLIBS = 

# Use provided memory script when linking [required]
LDFLAGS = -T$(LINKSCRIPT)
# Don't include stdlib when linking
LDFLAGS += -nostdlib
# Omit dead sections
LDFLAGS += -Wl,--gc-sections

# Combined Compiler flags
CFLAGS = $(DIALECT) $(DEBUG) $(OPTIMISE) $(WARN) $(ARMOPTS)
LCFLAGS = CFLAGS

# Debugger
DB = gdb-multiarch
DBARCH = armv7e-m
DBSCRIPT = -ex 'target extended-remote :3333'
DBSCRIPT += -ex 'monitor reset halt'
DBSCRIPT += -ex 'set $$msp = 0x20010000'
DBSCRIPT += -ex 'load'

# On-chip debugger
OCD = openocd
OCDFLAGS = -d2 -f $(OCDCONFIG)

# Flash re-programming script
OCDWRITE = -c init
OCDWRITE += -c "reset halt" 
OCDWRITE += -c "flash write_image erase $(FIRMWARE) 0x08000000"
OCDWRITE += -c "verify_image $(FIRMWARE) 0x08000000"
OCDWRITE += -c shutdown

# Flash erase script
OCDERASE = -c init
OCDERASE += -c "reset halt" 
OCDERASE += -c "stm32f1x mass_erase 0"
OCDERASE += -c shutdown

# binutils
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size
SIZEFLAGS = --format=GNU
NM = arm-none-eabi-nm
NMFLAGS = -n -r
DISFLAGS = -d -S
OBJDUMP = arm-none-eabi-objdump
GPG = gpg
GPGFLAGS = -b --local-user $(GPGKEY)
DD = dd

# Output files
FIRMWARE = $(PROJECT)_$(VERSION).bin
BINSIGNATURE = $(FIRMWARE).sign

# Default target is $(TARGET)
.PHONY: elf
elf: $(TARGET)

$(OBJECTS): Makefile

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $(TARGET) $(OBJECTS)

.PHONY: options
options: $(OPTIONS)

$(OPTIONS): Makefile src/options.o include/settings.h
	$(CC) $(CFLAGS) -nostdlib -o $(OPTIONS) src/options.o

$(LOADELF): $(LOADSRC) $(LOADSCRIPT)
	$(CC) $(CFLAGS) -T$(LOADSCRIPT) -nostdlib -o $(LOADELF) $(LOADSRC)

$(FIRMWARE): $(LOADELF) $(TARGET) $(OPTIONS)
	$(OBJCOPY) --pad-to=0x08008000 --gap-fill=$(PROGPAD) -O binary $(LOADELF) $(FIRMWARE)
	$(OBJCOPY) -O binary $(TARGET) .target.bin
	$(DD) if=.target.bin seek=1 bs=2K count=8 conv=notrunc of=$(FIRMWARE)
	$(OBJCOPY) -O binary $(OPTIONS) .presets.bin
	$(DD) if=.presets.bin seek=9 bs=2K count=7 conv=notrunc of=$(FIRMWARE)
	-rm .target.bin .presets.bin

$(BINSIGNATURE): $(FIRMWARE)
	$(GPG) $(GPGFLAGS) --output $(BINSIGNATURE) $(FIRMWARE)

# Override compilation recipe for assembly files
%.o: %.s
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

# Add recipe for listing files
%.lst: %.o
	$(OBJDUMP) $(DISFLAGS) $< > $@
%.lst: %.elf
	$(OBJDUMP) $(DISFLAGS) $< > $@

.PHONY: size
size: $(TARGET)
	$(SIZE) $(SIZEFLAGS) $(TARGET)

.PHONY: nm
nm: $(TARGET)
	$(NM) $(NMFLAGS) $(TARGET)

.PHONY: list
list: $(TARGETLIST)

.PHONY: erase
erase:
	$(OCD) $(OCDFLAGS) $(OCDERASE)

.PHONY: upload
upload: $(FIRMWARE)
	$(OCD) $(OCDFLAGS) $(OCDWRITE)

.PHONY: ocd
ocd:
	$(OCD) $(OCDFLAGS)
	
.PHONY: debug
debug: $(TARGET)
	$(DB) $(DBSCRIPT) $(TARGET)

.PHONY: bin
bin: $(FIRMWARE)

.PHONY: sign
sign: $(BINSIGNATURE)

.PHONY: clean
clean:
	-rm -f $(TARGET) $(LOADELF) $(LOADOBJ) $(OPTIONS) src/options.o $(FIRMWARE) $(BINSIGNATURE) $(OBJECTS) $(LISTFILES) $(TARGETLIST)

.PHONY: requires
requires:
	sudo apt-get install build-essential gcc-arm-none-eabi binutils-arm-none-eabi gdb-multiarch openocd

.PHONY: help
help:
	@echo
	@echo Targets:
	@echo " elf [default]	build all objects, link and write $(TARGET)"
	@echo " options 	create ROM options binary $(OPTIONS)"
	@echo " bin		create flash loader binary $(FIRMWARE)"
	@echo " sign		sign $(FIRMWARE) using gpg"
	@echo " size		list $(TARGET) section sizes"
	@echo " nm		list all defined symbols in $(TARGET)"
	@echo " list		create text listing for $(TARGET)"
	@echo " ocd		launch openocd on target in foreground"
	@echo " debug		debug $(TARGET) on target"
	@echo " erase		bulk erase flash on target"
	@echo " upload		write $(FIRMWARE) to flash and verify"
	@echo " clean		remove all intermediate files and logs"
	@echo " requires	install development dependencies"
	@echo
