SRC=main.c arm/fault.c
ASRC=arm/startup_ARMCM4.S
OBJ=$(SRC:.c=.o) $(ASRC:.S=.o)

TARGET=elfloader

CROSS=arm-none-eabi-
CC=$(CROSS)gcc
AS=$(CROSS)gcc
LD=$(CROSS)gcc

CFLAGS=-O0 -ggdb3 -mcpu=cortex-m4 -mthumb \
	-flto -ffunction-sections -fdata-sections \
	-Iarm/CMSIS/include

%.o: %.S
	$(AS) -c $(CFLAGS) -o $@ $^

LDFLAGS=--specs=rdimon.specs \
  -Wl,--start-group -lgcc -lc -lc -lm -lrdimon -Wl,--end-group \
  -Wl,--gc-sections \
  -L ldscripts -T gcc.ld \
  -mcpu=cortex-m4 -mthumb

.PHONY: clean all debug

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LD) -o $(TARGET) $(LDFLAGS) $(OBJ)

clean:
	rm -fR *.o $(TARGET)

debug: $(TARGET)
	arm-none-eabi-gdb $(TARGET) \
		-ex 'target remote :3333' \
		-ex 'monitor reset halt' \
		-ex 'load' \
		-ex 'monitor arm semihosting enable'
