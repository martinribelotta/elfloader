CROSS?=arm-none-eabi-
CC=$(CROSS)gcc
CXX=$(CROSS)g++
LD=$(CROSS)gcc
STRIP=$(CROSS)strip
SIZE=$(CROSS)size

SRC= main.cpp

OPT?=0

CFLAGS=-mcpu=cortex-m3 -mthumb -O$(OPT) -ggdb3 \
	-mword-relocations -mlong-calls -fno-common
#	-ffreestanding
#	-ffunction-sections -fdata-sections

CXXFLAGS=$(CFLAGS) -fno-rtti -fno-exceptions -fno-use-cxa-atexit

LDFLAGS=-r -Bsymbolic -nostartfiles \
	-mcpu=cortex-m3 -mthumb -mlong-calls -fno-common \
	-T elf.ld
#	--specs=nano.specs \

OBJS=$(SRC:.cpp=.o)
DEPS=$(SRC:.cpp=.d)

all: app-cpp.elf

-include $(DEPS)

#%.o: %.c
#	@echo " CC $<"
#	@$(CC) -MMD $(CFLAGS) -o $@ -c $<

%.o: %.cpp
	@echo " CXX $<"
	@$(CXX) -MMD $(CXXFLAGS) -o $@ -c $<

app-cpp.elf: $(OBJS)
	@echo " LINK $@"
	@$(LD) $(LDFLAGS) -o $@ $^
	@$(STRIP) -g -o app-cpp-striped.elf $@
	@$(SIZE) --common $@

.PHONY: clean all list

clean:
	@echo " CLEAN"
	@rm -fR $(OBJS) $(DEPS) *.elf

list:
	@echo " Creating list..."
	@$(CROSS)objdump -hpxdSsrt app-cpp.elf > app-cpp.elf.list
	@$(CROSS)readelf -atSln app-cpp.elf > app-cpp.elf.read
