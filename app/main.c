#include "sysent.h"

void banner(void) {
	syscalls->printf("Hola mundo el %s a las %s\n", __DATE__, __TIME__);
}

int main(void) {
	banner();
	return 0;
}
