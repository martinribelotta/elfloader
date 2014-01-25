#include "sysent.h"

const char *datetime[] = {
  __DATE__,
  __TIME__
};

volatile int i;

void banner(void) {
  for (i = 0; i < 10; i++)
    syscalls.printf("Hello world on %s at %s (%d)\n", datetime[0], datetime[1], i);
}

void (*const trampoline)(void) = banner;

int main(void) {
  trampoline();
  return 0;
}
