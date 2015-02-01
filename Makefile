SRC=main.c loader.c arm/fault.c
ASRC=arm/startup_ARMCM4.S

TARGET=elfloader

CROSS=arm-none-eabi-
CC=$(CROSS)gcc
AS=$(CROSS)gcc
LD=$(CROSS)gcc

CFLAGS=-O0 -ggdb3 -mcpu=cortex-m3 -mthumb \
	-flto -ffunction-sections -fdata-sections \
	-Iarm/CMSIS/include -I.

LDFLAGS=--specs=rdimon.specs \
  -Wl,--start-group -lgcc -lc -lc -lm -lrdimon -Wl,--end-group \
  -Wl,--gc-sections \
  -L ldscripts -T gcc.ld \
  -mcpu=cortex-m4 -mthumb

OBJS=$(SRC:.c=.o) $(ASRC:.S=.o)
DEPS=$(SRC:.c=.d)

all: $(TARGET)

-include $(DEPS)

%.o: %.c
	@echo " CC $<"
	@$(CC) -MMD $(CFLAGS) -o $@ -c $<

%.o: %.S
	@echo " AS $<"
	@$(AS) $(CFLAGS) -o $@ -c $<

.PHONY: clean all debug app

$(TARGET): $(OBJS)
	@echo " LINK $@"
	@$(LD) -o $@ $(LDFLAGS) $^

app:
	@$(MAKE) -C app clean all list

clean:
	@echo " CLEAN"
	@rm -fR $(OBJS) $(DEPS) $(TARGET)

debug: $(TARGET) app
	@echo " Debuggin..."
	@$(CROSS)gdb $(TARGET) \
		-ex 'target remote :3333' \
		-ex 'monitor reset halt' \
		-ex 'load' \
		-ex 'monitor arm semihosting enable'

run: $(TARGET) app
	@echo " Debuggin..."
	@$(CROSS)gdb $(TARGET) \
		-ex 'target remote :3333' \
		-ex 'monitor reset halt' \
		-ex 'load' \
		-ex 'monitor arm semihosting enable' \
		-ex 'continue'

oocd: app
	@echo " Launch OpenOCD for stm32f4discovery"
	@cd app && openocd -f board/stm32f4discovery.cfg -l oocd.log
