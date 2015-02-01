/****************************************************************************
 *  ARMv7M ELF loader
 *  Copyright (C) 2013  Martin Ribelotta
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>

#include "loader.h"
#include "app/sysent.h"

#define APP_PATH
#define APP_NAME "app-striped.elf"
#define APP_STACK_SIZE 1048

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
#if 1
  entry();
#else
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
#endif
}

int is_streq(const char *s1, const char *s2) {
	while(*s1 && *s2) {
		if (*s1 != *s2)
			return 0;
		s1++;
		s2++;
	}
	return *s1 == *s2;
}
