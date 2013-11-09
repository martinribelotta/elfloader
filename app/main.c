#include "sysent.h"

volatile int i;

void banner(void) {
	for(i=0; i<10; i++)
		syscalls->printf("Hola mundo el %s a las %s %d veces\n", __DATE__, __TIME__, i);
}

int main(void) {
	banner();
	return 0;
}
