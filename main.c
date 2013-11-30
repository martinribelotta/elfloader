#include <stdio.h>
#include <unistd.h>

#include "loader.h"
#include "app/sysent.h"

#define APP_PATH "/home/martin/Proyectos/workspace2/elfloader/app/"
#define APP_NAME "app-striped.elf"

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
2048, /* */
exporteds, /* */
sizeof(exporteds) / sizeof(*exporteds) /* */
};

int main(void) {
  exec_elf(APP_PATH APP_NAME, &env);
  puts("Done");
}
