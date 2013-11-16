#ifndef LOADER_H_
#define LOADER_H_

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

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

typedef struct {
  FILE *f;

  size_t sections;
  off_t sectionTable;
  off_t sectionTableStrings;

  size_t symbolCount;
  off_t symbolTable;
  off_t symbolTableStrings;
  off_t entry;

  ELFSection_t text;
  ELFSection_t rodata;
  ELFSection_t data;
  ELFSection_t bss;

  ELFSymbol_t *exported;
} ELFExec_t;

extern int exec_elf(const char *path, ELFSymbol_t *exported);

#ifdef __cplusplus__
}
#endif

#endif /* LOADER_H_ */
