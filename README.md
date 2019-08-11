# elfloader

## ARMv7M ELF loader

The goal of this project is provide a loader for ELF file format for ARMv7-M
(thumb-2) architecture (Aka Cortex-M, Cortex-R in Thumb2 mode) over bare-metal
or RTOS enviroment.

This loader not required MMU or special OS support (only aligned memory alloc)
and run with minimun memory overhead (only required parts of files is loaded in
memory).

This is developed using gcc arm embedded compiler from [GCC arm embedded
(arm-none-eabi)](https://launchpad.net/gcc-arm-embedded) but is successful
tested with linaro arm-linux-gnueabihf in freestangin mode. 

### ELF creation

For correct handling, The code must be compiled with certain characteristics:

* No common section is allowed. All non-init data is in bss (CC -fno-common)
* Only word relocation is allowed. (CC -mword-relocation) This is not realy
  true, some compilers produce __R\_ARM\_THB\_CALL/JMP__ relocation and ignore
  word-relocation flags, therefore, the loader handling only two relocation:
   * __R\_ARM\_ABS32__ Emmited on every data access and some jmp (weak/extern)
   * __R\_ARM\_THB\_JMP/CALL__ Emmited on some short jumps (CC -mlong-call flag
     not fix it)
* Relocatable ELF is required (LD -r option)
* No start library (LD -nostartfiles)

An example of application is found in the __app__ folder

### Usage

The API is simple, first call to #load_elf to obtain an execution context.

```c
    extern int load_elf(const char *path, const ELFEnv_t *env, ELFExec_t *exec);
```

This function take a path to a file, and ELFEnv_t is a struct containing:

```c
	typedef struct {
	  const ELFSymbol_t *exported;
	  size_t exported_size;
	} ELFEnv_t;
```

 - `exported`: Array of symbols to resolve in executable. `ELFSymbol_t` is a struct contains `const char *name` for C-String symbol name and `void *ptr` pointer to memory related to symbol (entry point of function, variable address, etc)
 - `size`: Size of exported symbol array in elements number

Then, #jumpTo and #get_func calls can be made:

```c
    extern int jumpTo(ELFExec_t *exec);
```
to execute code at the entry point of the ELF.

```c
    extern void * get_func(ELFExec_t *exec, const char *func_name);
```
to obtain a function pointer corresponding to a (mangled) symbol name.

Finally, the execution context can be cleaned up:

```c
    extern int unload_elf(ELFExec_t *exec);
```

### Loader config
##### File handling macros
   - `LOADER_FD_T` File descriptor type
   - `LOADER_OPEN_FOR_RD(path)` Function to open file for read
   - `LOADER_FD_VALID(fd)` Validity evaluation of file descriptor
   - `LOADER_READ(fd, buffer, size)` Function to read buffer from fd
   - `LOADER_CLOSE(fd)` Function to close file descriptor
   - `LOADER_SEEK_FROM_START(fd, off)` Seek function over fd
   - `LOADER_TELL(fd)` Tell position of fd cursor
#####  Memory manager/access
   - `LOADER_ALIGN_ALLOC(size, align, perm)` Aligned malloc function macro
   - `LOADER_FREE(ptr)` Free memory function
   - `LOADER_CLEAR(ptr, size)` Memory clearance (to 0) function
   - `LOADER_STREQ(s1, s2)` String compare function (return !=0 if s1==s2)
#####  Code execution
   - `LOADER_JUMP_TO(entry)` Macro for jump to "entry" pointer (entry_t)
#####  Debug/message
   - `DBG(...)` printf style macro for debug
   - `ERR(msg)` puts style macro used on severe error
   - `MSG(msg)` puts style macro used on info/warning
