CPU=attiny45

GCC=avr-gcc

CFLAGS= -Os -mmcu=$(CPU) -Wall -fpack-struct -fshort-enums -funsigned-bitfields -Wl,--relax -fno-move-loop-invariants -funsigned-char -fno-inline-small-functions -fdata-sections -fno-tree-loop-optimize -lprintf_min 

INCLUDES=
LIBS=

OBJCPY=avr-objcopy
OBJECTS=main.o

PROJECT_NAME=monitor_switch
HEX_FILE=$(PROJECT_NAME).hex
HEX_FILE_DUMP=$(PROJECT_NAME)_dump.hex

PROG=avrdude

PROG_FLAGS=-V -c usbasp -p $(CPU)
#PROG_FLAGS=-V -c buspirate -P /dev/buspirate -p $(CPU) 

# remember to set fuses:
# -p attiny45 -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m
# -p attiny45 -U efuse:w:0xff:m


all:	cls $(PROJECT_NAME) obj size #upload

$(PROJECT_NAME):	$(OBJECTS)
		$(GCC) -o $(PROJECT_NAME) $(OBJECTS) $(CFLAGS) $(LIBS) $(INCLUDES)

main.o:	main.c main.h
		$(GCC) $(CFLAGS) $(INCLUDES) -c main.c

obj:	$(OBJECTS)
		$(OBJCPY) -O ihex $(PROJECT_NAME) $(HEX_FILE)

clean:
		rm -f $(PROJECT_NAME) $(OBJECTS) $(HEX_FILE)

cls:
		clear;true;

size:
		du -b $(HEX_FILE) 

upload: all $(HEX_FILE)
		$(PROG) $(PROG_FLAGS) -e -U flash:w:$(HEX_FILE)

download:
		$(PROG) $(PROG_FLAGS) -U flash:r:$(HEX_FILE_DUMP)

reset:
		$(PROG) $(PROG_FLAGS)
