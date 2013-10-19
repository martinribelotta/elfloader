#include "sysent.h"

extern int main(void);

sysent_t *syscalls;

void _start(sysent_t *sys) {
	syscalls = sys;
	main();
}
