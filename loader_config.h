#ifndef LOADER_CONFIG_H_
#define LOADER_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if 0
#define LOADER_FD_T FILE *
#define LOADER_OPEN_FOR_RD(path) fopen(path, "rb")
#define LOADER_FD_VALID(fd) (fd != NULL)
#define LOADER_READ(fd, buffer, size) fread(buffer, 1, size, fd)
#define LOADER_WRITE(fd, buffer, size) fwrite(buffer, 1, size, fd)
#define LOADER_CLOSE(fd) fclose(fd)
#define LOADER_SEEK_FROM_START(fd, off) fseek(fd, off, SEEK_SET)
#define LOADER_TELL(fd) ftell(fd)
#else
#define LOADER_FD_T int
#define LOADER_OPEN_FOR_RD(path) open(path, O_RDONLY)
#define LOADER_FD_VALID(fd) (fd != -1)
#define LOADER_READ(fd, buffer, size) read(fd, buffer, size)
#define LOADER_WRITE(fd, buffer, size) write(fd, buffer, size)
#define LOADER_CLOSE(fd) close(fd)
#define LOADER_SEEK_FROM_START(fd, off) (lseek(fd, off, SEEK_SET) == -1)
#define LOADER_TELL(fd) lseek(fd, 0, SEEK_CUR)
#endif

#define LOADER_ALIGN_ALLOC(size, align, perm) ((void*) memalign(align, size))

#define LOADER_FREE(ptr) free(ptr)
#define LOADER_CLEAR(ptr, size) memset(ptr, 0, size)
#define LOADER_STREQ(s1, s2) (strcmp(s1, s2) == 0)

#define DBG(...) printf(__VA_ARGS__)
#define ERR(msg) do { perror(msg); __asm__ volatile ("bkpt"); } while(0)
#define MSG(msg) puts(msg)

#endif /* LOADER_CONFIG_H_ */
