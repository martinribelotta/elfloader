#include <stdio.h>
#include <unistd.h>

#include "loader.h"
#include "app/sysent.h"

#define APP_PATH
#define APP_NAME "app-striped.elf"
#define STACK_SIZE 2048

extern int open(const char *path, int mode, ...);

static const sysent_t sysentries = { /* */
open, /* */
close, /* */
write, /* */
read, /* */
printf, /* */
scanf /* */
};

static const ELFSymbol_t exporteds[] = { {"syscalls", (void*) &sysentries}};

static const ELFEnv_t env = { /* */
exporteds, /* */
sizeof(exporteds) / sizeof(*exporteds) /* */
};

int main(void) {
  exec_elf(APP_PATH APP_NAME, &env);
  puts("Done");
}

void arch_jumpTo(entry_t entry) {
	void *stack = malloc(STACK_SIZE);
	if (stack) {
		register uint32_t saved;
		void *tos = stack + STACK_SIZE;
		__asm__ volatile("MOV %0, sp\n\t" : : "r" (saved)); /* s->saved */
		__asm__ volatile("MOV sp, %0\n\t" : : "r" (tos)); /* tos->MSP */
		__asm__ volatile("PUSH {%0}\n\t" : : "r" (saved)); /* push saved */
		entry();
		__asm__ volatile("POP {%0}\n\t" : : "r" (saved)); /* pop saved */
		__asm__ volatile("MOV sp, %0\n\t" : : "r" (saved)); /* saved->sp */
		free(stack);
	} else
		perror("Stack alloc");
}
