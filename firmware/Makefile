###############################################################################
#    Project Configuration    #  

## The name of the project.
# TARGET := $(lastword $(subst /, ,$(CURDIR)))
TARGET = picstickfw

## Build directory.
BUILD_DIR = build

## Include directories.
INC_DIR = .

## The device to compile for.
MCU = attiny44

## Frequency of clock in Hz.
F_CPU = 8000000UL

## Fuse settings.
LFUSE = 0xE2
HFUSE = 0xDF
EFUSE = 0xFF


################################################################################
#    Programmer options    #

## Type of programmer using.
PROGRAMMER_TYPE = avrisp

## Serial port of programmer.
PROGRAMMER_PORT = /dev/ttyACM0

## Extra arguments to avrdude: baud rate, chip type, -F flag, etc.
PROGRAMMER_ARGS =



################################################################################
#    Compiler Setup    #

CC := avr-gcc
OBJCOPY := avr-objcopy
AVRDUDE := avrdude

## Firmware build options
FWFLAGS := -mmcu=${MCU} -DF_CPU=${F_CPU}

## Compiler options.
## ***NORE: 2022-17-6	gcc 12.1.0
## ***NOTE: array-bounds=0 is needed to prevent warnings about accessing bits in a byte?
CFLAGS := -g -Os -Wall -Warray-bounds=0 $(addprefix -I,$(INC_DIR))

## Linker options.
LFLAGS :=



################################################################################
#    Match n' Making    #

# Recursively find all *.c files in the directory.
SOURCES := $(shell find . -name '*.c' -printf '%P\n')

# Generate list of object files from source files
OBJECTS := $(SOURCES:%.c=$(BUILD_DIR)/%.o)


# HEADERS := $(SOURCES:.c=.h)

## Generate list of folders in build directory
# BUILD_FILE_DIRS = $(shell find $(BUILD_DIR) -type d)

## This is our main target: a .hex file
$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	@echo "Building hexfile..."
	$(OBJCOPY) -j .text -j .data -O ihex $< $@


$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	@echo "Linking objects..."
	$(CC) $(CFLAGS) $(LFLAGS) $(FWFLAGS) $^ -o $@


# Generate object files from source files
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) $(FWFLAGS) -c -o $@ $<




################################################################################
#    Make Commands    #

## Commands to use
.PHONY: fw clean fclean flash fuses


########################
#    Build Commands    #

## Build firmware with settings defined in Makefile. Equal to a plain 'make'
fw: $(BUILD_DIR)/$(TARGET).hex


########################
#    Clean Commands    #

## Clean-up build files.
clean:
	@echo "Removing firmware build files..."
	${eval BUILD_FILE_DIRS = $(shell find $(BUILD_DIR) -type d)}
	@rm -f $(BUILD_FILE_DIRS:%=%/*.*)
	@echo ""

## Clean-up build directories
fclean:
	@echo "Removing firmware build directories..."
	@rm -rf $(BUILD_DIR)
	@echo ""


###########################
#    Flashing Commands    #

PROGRAMMER := $(AVRDUDE)								\
				-p $(MCU)								\
				-c $(PROGRAMMER_TYPE)					\
				-P $(PROGRAMMER_PORT)					\
				$(PROGRAMMER_ARGS)

## Flashes the built hexfile onto a device using avrdude
flash: fw
	@echo "Flashing..."
	@$(PROGRAMMER) -U flash:w:$(BUILD_DIR)/$(TARGET).hex

## Sets fuses
fuses:
	$(PROGRAMMER)\
		-U lfuse:w:$(LFUSE):m \
		-U hfuse:w:$(HFUSE):m \
		-U efuse:w:$(EFUSE):m 



#######################
#    Help Commands    #

info:
	@echo "sources:  " $(SOURCES)
	@echo "assembly: " $(ASS_SOURCES)
	@echo "objects:  " $(OBJECTS)

.DEFAULT:
	@echo "Error: Unknown command"
	@echo " "
	@echo "'make [fw]'   - Build firmware with settings defined in Makefile."
	@echo " "
	@echo "'make flash'  - Flash firmware using avrdude, building only if neccessary."
	@echo "'make fuses'  - Burn fuses as defined in the Makefile using avrdude."
	@echo " "
	@echo "'make clean'  - Remove built firmware files."
	@echo "'make fclean' - Remove all files and folders created."
	@echo " "
