#ifndef SYSENT_H_
#define SYSENT_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int (*open)(const char *path, int mode, ...);
  int (*close)(int fd);
  int (*write)(int fd, const void *data, size_t size);
  int (*read)(int fd, void *buf, size_t size);
  int (*printf)(const char *fmt, ...);
  int (*scanf)(const char *fmt, ...);
} sysent_t;

typedef void (entry_t)(void);

extern sysent_t syscalls;

#ifdef __cplusplus
}
#endif

#endif /* SYSENT_H_ */
