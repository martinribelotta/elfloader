#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "elf32.h"
#include "arm/elf.h"
#include "app/sysent.h"

typedef struct {
	void *data;
	size_t size;
	int index;
	off_t fOff;
	off_t rOff;
} ELFSection_t;

static sysent_t sysentries;

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
} ELFExec_t;

#define eq(s1, s2) (strcmp(s1, s2) == 0)

int readSectionName(ELFExec_t *e, off_t off, char *buf, size_t max) {
	int ret = -1;
	off_t offset = e->sectionTableStrings + off;
	off_t old = ftell(e->f);
	if (fseek(e->f, offset, SEEK_SET) == 0)
		if (fread(buf, 1, max, e->f) == 0)
			ret = 0;
	fseek(e->f, old, SEEK_SET);
	return ret;
}

int readSymbolName(ELFExec_t *e, off_t off, char *buf, size_t max) {
	int ret = -1;
	off_t offset = e->symbolTableStrings + off;
	off_t old = ftell(e->f);
	if (fseek(e->f, offset, SEEK_SET) == 0)
		if (fread(buf, 1, max, e->f) == 0)
			ret = 0;
	fseek(e->f, old, SEEK_SET);
	return ret;
}

void *elf_getMemory(size_t nbytes, size_t align) {
	return (void*) memalign(align, nbytes);
}

void freeSection(ELFSection_t *s) {
	if (s->data)
		free(s->data);
}

static uint32_t swabo(uint32_t hl) {
	return ((((hl) >> 24)) | //
			(((hl) >> 8) & 0x0000ff00) | //
			(((hl) << 8) & 0x00ff0000) | //
			(((hl) << 24)));
}

void dumpSection(ELFSection_t *s) {
	size_t i;
	uint8_t *data = (uint8_t*) s->data;
	i = 0;
	while (i < s->size) {
		if ((i & 0xf) == 0)
			printf("\n  %04X: ", i);
		printf("%08x ", swabo(*((uint32_t*) (data + i))));
		i += sizeof(uint32_t);
	}
	printf("\n");
}

int loadSecData(ELFExec_t *e, Elf32_Shdr *h, ELFSection_t *s, const char *n) {
	s->size = h->sh_size;
	s->data = elf_getMemory(s->size, h->sh_addralign);
	if (s->data) {
		if (fseek(e->f, h->sh_offset, SEEK_SET) == 0) {
			if (fread(s->data, 1, s->size, e->f) == s->size) {
				printf("Contents of section %s:", n);
				dumpSection(s);
				return 0;
			} else
				perror("     read data fail");
		} else
			perror("    seek fail");
		freeSection(s);
	} else
		perror("    elf_getMemory fail");
	return -1;
}

int readSymbol(ELFExec_t *e, int n, Elf32_Sym *sym, char *name, size_t nlen) {
	int ret = -1;
	off_t old = ftell(e->f);
	if (fseek(e->f, e->symbolTable + n * sizeof(Elf32_Sym), SEEK_SET) == 0)
		if (fread(sym, 1, sizeof(Elf32_Sym), e->f) == sizeof(Elf32_Sym)) {
			if (sym->st_name)
				ret = readSymbolName(e, sym->st_name, name, nlen);
			else {
				Elf32_Shdr shdr;
				ret = readSection(e, sym->st_shndx, &shdr, name, nlen);
			}
		}
	fseek(e->f, old, SEEK_SET);
	return ret;
}

const char *typeStr(int symt) {
#define STRCASE(name) case name: return #name
	switch (symt) {
		STRCASE(R_ARM_NONE);
		STRCASE(R_ARM_ABS32);
		STRCASE(R_ARM_THM_CALL);
		STRCASE(R_ARM_THM_JUMP24);
	default:
		return "R_<unknow>";
	}
#undef STRCASE
}

void relocateSymbol(Elf32_Addr relAddr, int type, Elf32_Addr symAddr) {
	switch (type) {
	case R_ARM_ABS32:
		*((uint32_t*) relAddr) += symAddr;
		puts("R_ARM_ABS32 relocated");
		break;
	case R_ARM_THM_CALL:
	case R_ARM_THM_JUMP24:
		do {
			uint16_t upper_insn = ((uint16_t *) relAddr)[0];
			uint16_t lower_insn = ((uint16_t *) relAddr)[1];
			uint32_t S = (upper_insn >> 10) & 1;
			uint32_t J1 = (lower_insn >> 13) & 1;
			uint32_t J2 = (lower_insn >> 11) & 1;

			int32_t offset = (S << 24) | /* S     -> offset[24] */
			((~(J1 ^ S) & 1) << 23) | /* J1    -> offset[23] */
			((~(J2 ^ S) & 1) << 22) | /* J2    -> offset[22] */
			((upper_insn & 0x03ff) << 12) | /* imm10 -> offset[12:21] */
			((lower_insn & 0x07ff) << 1); /* imm11 -> offset[1:11] */
			if (offset & 0x01000000)
				offset -= 0x02000000;

			offset += symAddr - relAddr;

			S = (offset >> 24) & 1;
			J1 = S ^ (~(offset >> 23) & 1);
			J2 = S ^ (~(offset >> 22) & 1);

			upper_insn = ((upper_insn & 0xf800) | (S << 10)
					| ((offset >> 12) & 0x03ff));
			((uint16_t*) relAddr)[0] = upper_insn;

			lower_insn = ((lower_insn & 0xd000) | (J1 << 13) | (J2 << 11)
					| ((offset >> 1) & 0x07ff));
			((uint16_t*) relAddr)[1] = lower_insn;

			puts("R_ARM_THM_CALL/JMP relocated");

		} while (0);
		break;
	default:
		printf("Undefined relocation %d\n", type);
	}
}

ELFSection_t *sectionOf(ELFExec_t *e, int index) {
#define IFSECTION(sec, i) do { \
		if ((sec).index == i) \
			return &(sec); \
	} while(0)
	IFSECTION(e->text, index);
	IFSECTION(e->rodata, index);
	IFSECTION(e->data, index);
	IFSECTION(e->bss, index);
#undef IFSECTION
	return NULL;
}

Elf32_Addr symbolAddress(ELFSection_t *sec, Elf32_Sym *sym) {
	return (Elf32_Addr) (sec->data + sym->st_value);
}

int relocate(ELFExec_t *e, Elf32_Shdr *h, ELFSection_t *s, const char *name) {
	printf("Relocating section %s\n", name);
	if (s->data) {
		Elf32_Rel rel;
		size_t relEntries = h->sh_size / sizeof(rel);
		size_t relCount;
		fseek(e->f, h->sh_offset, SEEK_SET);
		printf(" Offset   Info     Type             Name\n");
		for (relCount = 0; relCount < relEntries; relCount++) {
			if (fread(&rel, 1, sizeof(rel), e->f) == sizeof(rel)) {
				Elf32_Sym sym;
				ELFSection_t *symSec;

				char name[33] = "<unnamed>";
				int symEntry = ELF32_R_SYM(rel.r_info);
				int relType = ELF32_R_TYPE(rel.r_info);

				readSymbol(e, symEntry, &sym, name, sizeof(name));
				printf(" %08X %08X %-16s %s\n", rel.r_offset, rel.r_info,
						typeStr(relType), name);

				symSec = sectionOf(e, sym.st_shndx);
				if (symSec) {
					Elf32_Addr symAddr = ((Elf32_Addr) symSec->data)
							+ sym.st_value;
					Elf32_Addr relAddr = ((Elf32_Addr) s->data) + rel.r_offset;
					printf(" symAddr=%08X relAddr=%08X\n", symAddr, relAddr);
					if (symSec->index == e->text.index) {
						printf("Fixing thumb addr %p to %p\n", (void*) relAddr,
								(void*) (relAddr | 1));
						symAddr |= 1; /* thumb fix */
					}
					relocateSymbol(relAddr, relType, symAddr);
				} else
					printf("No symbol address of %s\n", name);
			}
		}
		return 0;
	} else
		puts("Section not loaded");
	return -1;
}

int readSecHeader(ELFExec_t *e, off_t offset, Elf32_Shdr *h) {
	if (fseek(e->f, offset, SEEK_SET) != 0)
		return -1;
	if (fread(h, 1, sizeof(Elf32_Shdr), e->f) != sizeof(Elf32_Shdr))
		return -1;
	else
		return 0;
}

int readSection(ELFExec_t *e, int n, Elf32_Shdr *h, char *name, size_t nlen) {
	if (readSecHeader(e, e->sectionTable + n * sizeof(Elf32_Shdr), h) != 0)
		return -1;
	if (h->sh_name)
		return readSectionName(e, h->sh_name, name, nlen);
	return 0;
}

int findSymbol(ELFExec_t *e, const char *sName, Elf32_Sym *sym) {
	int i;
	char name[33];
	fseek(e->f, e->symbolTable, SEEK_SET);
	for (i = 0; i < e->symbolCount; i++) {
		if (fread(sym, 1, sizeof(Elf32_Sym), e->f) == sizeof(Elf32_Sym)) {
			if (sym->st_name) {
				readSymbolName(e, sym->st_name, name, sizeof(name));
				if (eq(name, sName))
					return 0;
			}
		} else
			return -1;
	}
	return -1;
}

void dumpSymbols(ELFExec_t *e) {
	int i;
	fseek(e->f, e->symbolTable, SEEK_SET);
	puts("  Num:   Value  Size shNdx Name");
	for (i = 0; i < e->symbolCount; i++) {
		Elf32_Sym sym;
		char name[33] = "<unnamed>";

		readSymbol(e, i, &sym, name, sizeof(name));
		printf(" % 4d: %08X % 4d %4d  %s\n", i, sym.st_value, sym.st_size,
				sym.st_shndx, name);
	}
}

#define IS_FLAGS_SET(v, m) ((v&m) == m)

typedef enum {
	FoundSymTab = (1 << 0),
	FoundStrTab = (1 << 2),
	FoundText = (1 << 3),
	FoundRodata = (1 << 4),
	FoundData = (1 << 5),
	FoundBss = (1 << 6),
	FoundRelText = (1 << 7),
	FoundRelRodata = (1 << 8),
	FoundRelData = (1 << 9),
	FoundRelBss = (1 << 10),
	FoundValid = FoundSymTab | FoundStrTab,
	FoundExec =	FoundValid | FoundText,
	FoundAll =
			FoundSymTab |
			FoundStrTab |
			FoundText |
			FoundRodata |
			FoundData |
			FoundBss |
			FoundRelText |
			FoundRelRodata |
			FoundRelData |
			FoundRelBss
} FindFlags_t;

int loadSymbols(ELFExec_t *e) {
	int n;
	int founded = 0;
	puts("Scan ELF indexs...");
	for (n = 0; n < e->sections; n++) {
		Elf32_Shdr sectHdr;
		char name[33] = "<unamed>";
		off_t offset = e->sectionTable + n * sizeof(Elf32_Shdr);

		readSecHeader(e, offset, &sectHdr);
		if (sectHdr.sh_name)
			readSectionName(e, sectHdr.sh_name, name, sizeof(name));
		printf("Examining section %d %s\n", n, name);
		if (eq(name, ".symtab")) {
			e->symbolTable = sectHdr.sh_offset;
			e->symbolCount = sectHdr.sh_size / sizeof(Elf32_Sym);
			founded |= FoundSymTab;
		} else if (eq(name, ".strtab")) {
			e->symbolTableStrings = sectHdr.sh_offset;
			founded |= FoundStrTab;
		} else if (eq(name, ".text")) {
			e->text.fOff = offset;
			e->text.index = n;
			founded |= FoundText;
		} else if (eq(name, ".rodata")) {
			e->rodata.fOff = offset;
			e->rodata.index = n;
			founded |= FoundRodata;
		} else if (eq(name, ".data")) {
			e->data.fOff = offset;
			e->data.index = n;
			founded |= FoundData;
		} else if (eq(name, ".bss")) {
			e->bss.fOff = offset;
			e->bss.index = n;
			founded |= FoundBss;
		} else if (eq(name, ".rel.text")) {
			e->text.rOff = offset;
			founded |= FoundRelText;
		} else if (eq(name, ".rel.rodata")) {
			e->rodata.rOff = offset;
			founded |= FoundRelText;
		} else if (eq(name, ".rel.data")) {
			e->data.rOff = offset;
			founded |= FoundRelText;
		} else if (eq(name, ".rel.bss")) {
			e->bss.rOff = offset;
			founded |= FoundRelText;
		}
		if (IS_FLAGS_SET(founded, FoundAll))
			return FoundAll;
	}
	puts("Done");
	return founded;
}

int initElf(ELFExec_t *e, FILE *f) {
	Elf32_Ehdr h;
	Elf32_Shdr sH;

	memset(e, 0, sizeof(ELFExec_t));

	if (fread(&h, 1, sizeof(h), f) != sizeof(h))
		return -1;

	e->f = f;

	if (readSecHeader(e, h.e_shoff + h.e_shstrndx * sizeof(sH), &sH) == -1)
		return -1;

	e->entry = h.e_entry;
	e->sections = h.e_shnum;
	e->sectionTable = h.e_shoff;
	e->sectionTableStrings = sH.sh_offset;

	return 0;
}

void freeElf(ELFExec_t *e) {
	freeSection(&e->text);
	freeSection(&e->rodata);
	freeSection(&e->data);
	freeSection(&e->bss);
	fclose(e->f);
}

int loadSection(ELFExec_t *e, ELFSection_t *s, const char *name) {
	printf("Loading section %s\n", name);
	if (s->index && s->fOff) {
		Elf32_Shdr h;
		if (readSecHeader(e, s->fOff, &h) == 0) {
			return loadSecData(e, &h, s, name);
		} else
			puts("Error loading data");
	} else
		puts("No offset or index for section");
	return -1;
}

int loadSections(ELFExec_t *e) {
	loadSection(e, &e->text, ".text");
	loadSection(e, &e->rodata, ".rodata");
	loadSection(e, &e->data, ".data");
	loadSection(e, &e->bss, ".bss");
	return 0;
}

int relocateSection(ELFExec_t *e, ELFSection_t *s, const char *name) {
	printf("Relocating section %s\n", name);
	if (s->rOff) {
		Elf32_Shdr sectHdr;
		readSecHeader(e, s->rOff, &sectHdr);
		relocate(e, &sectHdr, &e->text, name);
	} else
		puts("No relocation index");
	return 0;
}

int relocateSections(ELFExec_t *e) {
	relocateSection(e, &e->text, ".text");
	relocateSection(e, &e->rodata, ".rodata");
	relocateSection(e, &e->data, ".data");
	relocateSection(e, &e->bss, ".bss");
	return 0;
}

void jumpTo(ELFExec_t *e) {
	if (e->entry) {
		entry_t *entry = (entry_t*) (e->text.data + e->entry);
		entry(&sysentries);
	} else {
		Elf32_Sym sym;
		puts("No entry defined. Find _start");
		if (findSymbol(e, "_start", &sym) == 0) {
			Elf32_Addr eAddr = symbolAddress(&e->text, &sym);
			entry_t *entry = (entry_t*) (eAddr | 1); /* FIX THUMB */
			entry(&sysentries);
		} else
			puts("_start not found");
	}
}

void initSysent(void) {
	int open(const char *path, int mode, ...);

	sysentries.open = open;
	sysentries.close = close;
	sysentries.write = write;
	sysentries.read = read;
	sysentries.printf = printf;
	sysentries.scanf = scanf;
}

#define APP_PATH "/home/martin/Proyectos/workspace2/elfloader/app/"
#define APP_NAME "app-striped.elf"

int main(void) {
	ELFExec_t exec;
	unsigned int idx;
	initSysent();

	initElf(&exec, fopen(APP_PATH APP_NAME, "rb"));
	if (IS_FLAGS_SET(loadSymbols(&exec), FoundValid)) {
		dumpSymbols(&exec);
		loadSections(&exec);
		relocateSections(&exec);
		jumpTo(&exec);
		freeElf(&exec);
	} else
		puts("Invalid EXEC");

	puts("Done");
}
