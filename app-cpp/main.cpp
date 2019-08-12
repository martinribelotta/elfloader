#include "sysent.h"

static char test_data[] __attribute__ ((section (".sdram_data"))) =
    {10, 11, 12, 13, 14, 15, 16, 17 };
static const char test_rodata[] __attribute__ ((section (".sdram_rodata"))) =
    {20, 21, 22, 23, 24, 25, 26, 27 };
static char test_bss[16] __attribute__ ((section (".sdram_bss")));

class testclass {
public:
  // Default Constructor
  int i;
  testclass() {
    i = 1;
    syscalls.printf("Construct world\n");
  }
  ~testclass() {
    syscalls.printf("Destruct world\n");
  }
  void banner(void) {
    syscalls.printf("Hello banner %d (%d %d %d)\n", i, test_data[i],
        test_rodata[i], test_bss[i]);
  }
  void doit(void) {
    i++;
    syscalls.printf("Doing it %d (%d %d %d)\n", i, test_data[i], test_rodata[i],
        test_bss[i]);
  }
};

static testclass test_instance;

extern "C" {

void _start(void) {
  syscalls.printf("Hello main\n");
  test_instance.banner();
  syscalls.printf("Bye main\n");
}

void doit(void) {
  test_instance.doit();
}

}

