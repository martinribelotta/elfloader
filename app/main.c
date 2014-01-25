#include "sysent.h"

const char *build_date = __DATE__;
const char *build_time = __TIME__;

volatile int i;

void banner(void) {
  for (i = 0; i < 10; i++)
    syscalls.printf("Hello world on %s at %s (%d)\n", build_date, build_time, i);
}

void (*const trampoline)(void) = banner;

int main(void) {
  trampoline();
  return 0;
}
