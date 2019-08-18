PREFIX = arm-none-eabi-
CPP = g++
CC = gcc
LD = gcc
OBJDUMP = objdump
OBJCOPY = objcopy

OBJS         = $(patsubst src/%.c,build/%.o,$(wildcard src/*.c))
#OBJS        += $(patsubst src/periph/%.c,build/periph/%.o,$(wildcard src/periph/*.c))
DEPS         = $(OBJS:%.o=%.d)

DEVICE = stm32f103c8
LIBNAME = opencm3_stm32f1

OPENCM3_DIR     = libopencm3

DEFS		+= -DSTM32F1
DEFS		+= -I$(OPENCM3_DIR)/include
DEFS		+= -I./include

FP_FLAGS	?= -msoft-float
ARCH_FLAGS  = -mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd

CFLAGS      += -ggdb3 -std=c99 -Wall -Wpedantic
CXXFLAGS    += -ggdb3
CPPFLAGS	+= -MD $(DEFS) $(ARCH_FLAGS)

LDSCRIPT 	 = bluepill.ld

LDFLAGS     += -static -nostartfiles
LDFLAGS		+= -L$(OPENCM3_DIR)/lib
LDFLAGS		+= $(ARCH_FLAGS)

LDLIBS	 	+= -Wl,-lc -Wl,-lgcc -Wl,-lnosys -Wl,-l$(LIBNAME)

MAKEFLAGS += --no-print-directory

.SILENT:
.SECONDARY:
.PHONY: clean all libopencm3 flash

all: build/firmware.elf build/firmware.bin
st:  build/firmware-st.elf build/firmware-st.bin

%.bin: %.elf
	printf "  OBJCOPY $(*).bin\n"
	$(PREFIX)$(OBJCOPY) -Obinary $(*).elf $(*).bin

%.list: %.elf
	printf "  OBJDUMP $(*).list\n"
	$(PREFIX)$(OBJDUMP) -S $(*).elf > $(*).list

build/%.elf: $(OBJS) %.ld
	printf "  LD      $(*).elf\n"
	$(PREFIX)$(LD) $(LDFLAGS) -T$(*).ld $(OBJS) $(LDLIBS) -o $@

build/%.o: src/%.c | libopencm3
	@mkdir -p $(@D)
	printf "  CC      $(*).c\n"
	$(PREFIX)$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

build/%.o: src/%.cpp | libopencm3
	@mkdir -p $(@D)
	printf "  CXX     $(*).cpp\n"
	$(PREFIX)$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

flash: build/firmware.bin
	echo "Uploading to board..."
	echo "Push reset."
	sleep 2
	dfu-util -d 1EAF:0003 -a 2 -D $< -R

flash-st: build/firmware-st.bin
	echo "Flashing firmware in 3s..."
	sleep 3
	openocd -c "log_output openocd.log" -f debug/bluepill-flash.cfg

clean:
	printf "  CLEAN\n"
	$(RM) -r build/

libopencm3: $(OPENCM3_DIR)/lib/libopencm3_stm32f1.a

$(OPENCM3_DIR)/lib/libopencm3_stm32f1.a:
	$(MAKE) -C $(OPENCM3_DIR) --no-print-directory TARGETS=stm32/f1

-include $(DEPS)
