PREFIX = arm-none-eabi-
CPP = g++
CC = gcc
LD = gcc
OBJDUMP = objdump
OBJCOPY = objcopy

OBJS        += build/miniblink.o
DEPS         = $(OBJS:%.o=%.d)

DEVICE = stm32f103c8
LIBNAME = opencm3_stm32f1

OPENCM3_DIR     = libopencm3

DEFS		+= -DSTM32F1
DEFS		+= -I$(OPENCM3_DIR)/include
DEFS		+= -I./include

FP_FLAGS	?= -msoft-float
ARCH_FLAGS  = -mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd

CFLAGS      += -Os -ggdb3 -std=c99 $(DEFS) $(ARCH_FLAGS)
CPPFLAGS	+= -MD $(DEFS) $(ARCH_FLAGS)

LDSCRIPT 	 = bluepill.ld

LDFLAGS     += -static -nostartfiles
LDFLAGS		+= -L$(OPENCM3_DIR)/lib -T$(LDSCRIPT)
LDFLAGS		+= $(ARCH_FLAGS)

LDLIBS	 	+= -Wl,-lc -Wl,-lgcc -Wl,-lnosys -Wl,-l$(LIBNAME)

MAKEFLAGS += --no-print-directory

.SILENT:
.SECONDARY:
.PHONY: clean all libopencm3 flash

all: build/binary.elf build/binary.bin

%.bin: %.elf
	printf "  OBJCOPY $(*).bin\n"
	$(PREFIX)$(OBJCOPY) -Obinary $(*).elf $(*).bin

%.list: %.elf
	printf "  OBJDUMP $(*).list\n"
	$(PREFIX)$(OBJDUMP) -S $(*).elf > $(*).list

%.elf: $(OBJS) $(LDSCRIPT)
	printf "  LD      $(*).elf\n"
	$(PREFIX)$(LD) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(*).elf

build/%.o: src/%.c | libopencm3 build/.keep
	printf "  CC      $(*).c\n"
	$(PREFIX)$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

build/%.o: src/%.cpp | libopencm3 build/.keep
	printf "  CXX     $(*).cpp\n"
	$(PREFIX)$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

build/.keep:
	mkdir -p build
	touch build/.keep

flash: build/binary.bin
	echo "Uploading to board..."
	echo "Push reset."
	sleep 2
	dfu-util -d 1EAF:0003 -a 2 -D $< -R

clean:
	printf "  CLEAN\n"
	$(RM) -r build/

libopencm3: $(OPENCM3_DIR)/lib/libopencm3_stm32f1.a

$(OPENCM3_DIR)/lib/libopencm3_stm32f1.a:
	$(MAKE) -C $(OPENCM3_DIR) --no-print-directory TARGETS=stm32/f1

-include $(DEPS)
