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

The API is simple, call to exec_elf(path, exported symbol table) function and
enjoying.

    int exec_elf(const char *path, ELFSymbol_t *exported);

This function take a path to a file, and ELFSymbol_t array (zero terminated)
for find, open, relocate and finally jump to executable code. 

The caller of this function is called "Host" and the code called over this
function is called "Guest".

The path is fopen valid path. In this example, semihosting library is used to
provide filesystem mechanism but can be replaced for your system calls.

The parameter "exported" is a pointer to an array of zero-terminated
ELFSymbol_t containing the symbols exported by the host over the guest.
These symbols are used to solve the undefined links in the guest
