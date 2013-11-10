#include "sysent.h"

volatile int i;

void banner(void) {
	for (i = 0; i < 10; i++)
		syscalls.printf("Hello world on %s at %s %d times\n", __DATE__,
				__TIME__, i);
}

int main(void) {
	banner();
	return 0;
}
