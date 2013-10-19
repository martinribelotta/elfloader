#include "sysent.h"

int main(void) {
	syscalls->printf("Hola mundo el %s a las %s\n", __DATE__, __TIME__);
	return 0;
}
