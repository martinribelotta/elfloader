#ifndef LOADER_H_
#define LOADER_H_

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus__
extern "C" {
#endif

typedef struct {
  void *data;
  size_t size;
  int index;
  off_t fOff;
  off_t rOff;
} ELFSection_t;

typedef struct {
  const char *name;
  void *ptr;
} ELFSymbol_t;

extern int exec_elf(const char *path, ELFSymbol_t *exported);

#ifdef __cplusplus__
}
#endif

#endif /* LOADER_H_ */
