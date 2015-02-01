/****************************************************************************
 * ARMv7M ELF loader
 * Copyright (c) 2013-2015 Martin Ribelotta
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
