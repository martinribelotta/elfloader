CROSS?=arm-none-eabi-
CC=$(CROSS)gcc
LD=$(CROSS)gcc
STRIP=$(CROSS)strip

SRC=main.c start.c
OBJ=$(SRC:.c=.o)

OPT?=0
CFLAGS=-mcpu=cortex-m3 -mthumb -O$(OPT) -ggdb3 \
	-mword-relocations -mlong-calls -fno-common
LDFLAGS=-r -Bsymbolic -nostartfiles \
	-mcpu=cortex-m3 -mthumb -mlong-calls -fno-common

all: app.elf

app.elf: $(OBJ)
	$(LD) -o app.elf $(OBJ) $(LDFLAGS)
	$(STRIP) -g -o app-striped.elf app.elf

.PHONY: clean all list

clean:
	@rm -fR *.o *.elf

list:
	arm-none-eabi-objdump -hpxdSsrt app.elf > app.elf.list
	arm-none-eabi-readelf -atSln app.elf > app.elf.read
