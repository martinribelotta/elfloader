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

#ifndef LOADER_CONFIG_H_
#define LOADER_CONFIG_H_

#include <stdlib.h>
#include <string.h>

#ifndef DOX

#define LOADER_MAX_SYM_LENGTH 33
#define LOADER_USERDATA_T loader_env_t

#define LOADER_GETUNDEFSYMADDR(userdata, name) getUndefinedSymbol(userdata, name)

#if 0

#include <stdio.h>

typedef struct loader_env {
  FILE *fd;
  const struct ELFEnv * env;
} loader_env_t;

#define LOADER_USERDATA_T loader_env_t

#define LOADER_OPEN_FOR_RD(userdata, path) userdata.fd = fopen(path, "rb")
#define LOADER_FD_VALID(userdata) (userdata.fd != NULL)
#define LOADER_READ(userdata, buffer, size) fread(buffer, 1, size, userdata.fd)
#define LOADER_WRITE(userdata, buffer, size) fwrite(buffer, 1, size, userdata.fd)
#define LOADER_CLOSE(userdata) fclose(userdata.fd)
#define LOADER_SEEK_FROM_START(userdata, off) fseek(userdata.fd, off, SEEK_SET)
#define LOADER_TELL(userdata) ftell(userdata.fd)

#elif 1

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct loader_env {
  int fd;
  const struct ELFEnv * env;
} loader_env_t;

#define LOADER_USERDATA_T loader_env_t

#define LOADER_OPEN_FOR_RD(userdata, path) userdata.fd=open(path, O_RDONLY)
#define LOADER_FD_VALID(userdata) (userdata.fd != -1)
#define LOADER_READ(userdata, buffer, size) read(userdata.fd, buffer, size)
#define LOADER_WRITE(userdata, buffer, size) write(userdata.fd, buffer, size)
#define LOADER_CLOSE(userdata) close(userdata.fd)
#define LOADER_SEEK_FROM_START(userdata, off) (lseek(userdata.fd, off, SEEK_SET) == -1)
#define LOADER_TELL(userdata) lseek(userdata.fd, 0, SEEK_CUR)

#else

#include <ff.h>

typedef struct loader_env {
  FIL fd;
  FRESULT ret;
  const struct ELFEnv * env;
} loader_env_t;

#define LOADER_USERDATA_T loader_env_t

static inline size_t wrap_f_read(LOADER_USERDATA_T *u, void *buf, size_t n)
{
  UINT count = 0;
  u->ret = f_read(u->fd, buf, n, &count);
  if (u->ret == FR_OK)
    return count;
  return -1;
}

static inline size_t wrap_f_write(LOADER_USERDATA_T *u, const void *buf, size_t n)
{
  UINT count = 0;
  u->ret = f_write(u->fd, buf, n, &count);
  if (u->ret == FR_OK)
    return count;
  return -1;
}

#define LOADER_OPEN_FOR_RD(userdata, path) userdata.ret=open(&userdata.fd, path, O_RDONLY)
#define LOADER_FD_VALID(userdata) (userdata.ret != FR_OK)
#define LOADER_READ(userdata, buffer, size) wrap_f_read(&userdata.fd, buffer, size)
#define LOADER_WRITE(userdata, buffer, size) wrap_f_write(&userdata.fd, buffer, size)
#define LOADER_CLOSE(userdata) f_close(&userdata.fd)
#define LOADER_SEEK_FROM_START(userdata, off) (f_lseek(&userdata.fd, off) == FR_OK)
#define LOADER_TELL(userdata) f_tell(&userdata.fd)

#endif

#if 0
#define LOADER_ALIGN_ALLOC(size, align, perm) ((void*) memalign(align, size))
#else

extern void *do_alloc(size_t size, size_t align, ELFSecPerm_t perm);
extern void *do_alloc_sdram(size_t size, size_t align, ELFSecPerm_t perm);

#define LOADER_ALIGN_ALLOC(size, align, perm) do_alloc(size, align, perm)
#define LOADER_ALIGN_ALLOC_SDRAM(size, align, perm) do_alloc_sdram(size, align, perm)

#endif

extern int is_streq(const char *s1, const char *s2);

#define LOADER_FREE(ptr) free(ptr)
#define LOADER_STREQ(s1, s2) (is_streq(s1, s2))

#if 0
#define LOADER_JUMP_TO(entry) entry();
#else

extern void arch_jumpTo(entry_t entry);

#define LOADER_JUMP_TO(entry) arch_jumpTo(entry)

#endif

#define DBG(...) printf("ELF: " __VA_ARGS__)
#define ERR(...) do { printf("ELF: " __VA_ARGS__); __asm__ volatile ("bkpt"); } while(0)
#define MSG(msg) puts("ELF: " msg)

#else

/**
 * @defgroup Configuration Configuration macros
 *
 * This macros defines the access function to various internal function of
 * elf loader
 *
 * You need to define this macros to compile and use the elf loader library
 * in your system
 *
 * @{
 */

/**
 * Undefined-symbol resolver
 *
 * @param userdata
 * @param name symbol name
 * @retval address of symbol, 0xffffffff if not resolved
 */
#define LOADER_GETUNDEFSYMADDR

/**
 * Userdata descriptor type macro
 *
 * This define the userdata declaration type
 */
#define LOADER_USERDATA_T

/**
 * Open for read function macro
 *
 * This macro define the function name and convention call to open file for
 * read. This need to return #LOADER_FD_T type
 *
 * @param userdata User data object
 * @param path Path to file for open
 */
#define LOADER_OPEN_FOR_RD(userdata, path)

/**
 * Macro for check file descriptor validity
 *
 * This macro is used for check the validity of #LOADER_FD_T after open
 * operation
 *
 * @param userdata User data
 * @retval Zero if fd is unusable
 * @retval Non-zero if fd is usable
 */
#define LOADER_FD_VALID(userdata)

/**
 * Macro for read buffer from file
 *
 * This macro is used when need read a block for the #LOADER_FD_T previous
 * opened.
 * @param userdata User data
 * @param buffer Writable buffer to store read data
 * @param size Number of bytes to read
 */
#define LOADER_READ(userdata, buffer, size)

/**
 * Close file macro
 *
 * Close a file descriptor previously opened with LOADER_OPEN_FOR_RD
 *
 * @param userdata User data object
 */
#define LOADER_CLOSE(userdata)

/**
 * Seek read cursor of file
 *
 * Seek read cursor from begin of file
 *
 * @param userdata User data
 * @param off Offset from begin of file
 */
#define LOADER_SEEK_FROM_START(userdata, off)

/**
 * Tell cursor of file
 *
 * Tell current read cursor of file
 *
 * @param userdata User data
 * @return The current read cursor position
 */
#define LOADER_TELL(userdata)

/**
 * Allocate memory service
 *
 * Allocate aligned memory with required right access
 *
 * @param size Size in bytes to allocate
 * @param align Aligned size in bytes
 * @param perm Access rights of allocated block. Mask of #ELFSecPerm_t values
 * @return Void pointer to allocated memory or NULL on fail
 */
#define LOADER_ALIGN_ALLOC(size, align, perm)

/**
 * Free memory
 *
 * Free memory allocated with #LOADER_ALIGN_ALLOC
 *
 * @param ptr Pointer to allocated memory to free
 */
#define LOADER_FREE(ptr)

/**
 * Compare string
 *
 * Compare zero terminated C-strings
 *
 * @param s1 First string
 * @param s2 Second string
 * @retval Zero if non equals
 * @retval Non-zero if s1==s2
 */
#define LOADER_STREQ(s1, s2)

/**
 * Jump to code
 *
 * Execute loaded code from entry point
 *
 * @param entry Pointer to function code to execute
 */
#define LOADER_JUMP_TO(entry)

/**
 * Debug macro
 *
 * This macro is used for trace code execution and not side effect is needed
 *
 * @param ... printf style format and variadic argument list
 */
#define DBG(...)

/**
 * Error macro
 *
 * This macro is used on unrecoverable error message
 *
 * @param msg C string printable text
 */
#define ERR(msg)

/**
 * Information/Warnings message macro
 *
 * This macro is used on information or recoverable warnings mesages
 *
 * @param ... printf style format and variadic argument list
 */
#define MSG(msg)

/** @} */

#endif



/**
 * Exported symbol struct
 */
typedef struct {
  const char *name; /*!< Name of symbol */
  void *ptr; /*!< Pointer of symbol in memory */
} ELFSymbol_t;

/**
 * Environment for execution
 */
typedef struct ELFEnv {
  const ELFSymbol_t *exported; /*!< Pointer to exported symbols array */
  unsigned int exported_size; /*!< Elements on exported symbol array */
} ELFEnv_t;

static uint32_t getUndefinedSymbol(LOADER_USERDATA_T *userdata, const char *sName) {
  const ELFEnv_t *env = userdata->env;
  int i;
  for (i = 0; i < env->exported_size; i++)
    if (LOADER_STREQ(env->exported[i].name, sName))
      return (uint32_t) (env->exported[i].ptr);
  DBG("  Can not find address for symbol %s\n", sName);
  return 0xffffffff;
}


#endif /* LOADER_CONFIG_H_ */
