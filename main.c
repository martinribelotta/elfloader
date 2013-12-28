#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>

#include "loader.h"
#include "app/sysent.h"

#define APP_PATH
#define APP_NAME "app-striped.elf"
#define APP_STACK_SIZE 2048

extern int open(const char *path, int mode, ...);

static const sysent_t sysentries = { /* */
open, /* */
close, /* */
write, /* */
read, /* */
printf, /* */
scanf /* */
};

static const ELFSymbol_t exports[] = { { "syscalls", (void*) &sysentries } };
static const ELFEnv_t env = { exports, sizeof(exports) / sizeof(*exports) };

int main(void) {
  exec_elf(APP_PATH APP_NAME, &env);
  puts("Done");
}

void *do_alloc(size_t size, size_t align, ELFSecPerm_t perm) {
  (void) perm;
  /* TODO Change perms with MPU */
  return memalign(align, size);
}

void arch_jumpTo(entry_t entry) {
  void *stack = do_alloc(APP_STACK_SIZE, 8, ELF_SEC_READ | ELF_SEC_WRITE);
  if (stack) {
    register uint32_t saved;
    void *tos = stack + APP_STACK_SIZE;

    /* s->saved */
    __asm__ volatile("MOV %0, sp\n\t" : : "r" (saved));
    /* tos->MSP */
    __asm__ volatile("MOV sp, %0\n\t" : : "r" (tos));
    /* push saved */
    __asm__ volatile("PUSH {%0}\n\t" : : "r" (saved));

    entry();

    /* pop saved */
    __asm__ volatile("POP {%0}\n\t" : : "r" (saved));
    /* saved->sp */
    __asm__ volatile("MOV sp, %0\n\t" : : "r" (saved));

    free(stack);
  } else
    perror("Stack alloc");
}
