/*
** See Copyright Notice In elf.h
** elf-mod.h
** Module
*/


typedef struct elf_File {
	char *name;
	lbyteid bytes;
	lbyteid nbytes;
	int **protos;
	/* todo: eventually remove these */
	char *pathondisk;
	llineid lines;
	int nlines;
} elf_File;


/*
** Symbols
** 	lBytecode and globals can be added dynamically and
** safely, in fact, multiple files will reference the
** same global by name, no matter the order in which
** they were loaded, or the means, runtime/compiletime.
** This is because we use a symbol table that maps a
** name at compile time to an index in the global values.
** Even if a file is loaded at runtime, the compilation
** process finds the global symbol and maps it to the
** target index. Lookups are effectively done at compile
** time.
**
*/
typedef struct elf_Module {
	union { elf_Table *g, *globals; };

	elf_Proto *p;
	elf_num *kn;
	elf_int *ki;
	int *track;
	lBytecode *bytes;
	lbyteid nbytes;
	char **lines;
	elf_File *files;
} elf_Module;


lglobalid elf_setsym(elf_Module *md, elf_String *name);
lglobalid lang_addglobal(elf_Module *md, elf_String *name, elf_val v);
lglobalid lang_addproto(elf_Module *md, elf_Proto p);

/*
	elf_Module\r: runtime is stored here
for garbage collection.
	elf_Module\gc: all objects to be automatically
managed, or garbage collected, are listed here.
By default all objects are added here, you
can however remove them from this array.

elf_Module\gf: buffer for functions definitions,
essentially a type table, anonymous functions
are also added here.

elf_Module\g: global symbol table which
maps names to values, indexed
at runtime by index.

elf_Module\bytes: buffer for bytes, all the bytes
are stored here, functions index into this
buffer.

*/