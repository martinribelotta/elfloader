#ifndef LOADER_H_
#define LOADER_H_

#ifdef __cplusplus__
extern "C" {
#endif

typedef enum {
  ELF_SEC_WRITE = 0x1, ELF_SEC_READ = 0x2, ELF_SEC_EXEC = 0x4,
} ELFSecPerm_t;

typedef struct {
  const char *name;
  void *ptr;
} ELFSymbol_t;

typedef struct {
  const ELFSymbol_t *exported;
  unsigned int exported_size;
} ELFEnv_t;

extern int exec_elf(const char *path, const ELFEnv_t *env);

#ifdef __cplusplus__
}
#endif

#endif /* LOADER_H_ */
