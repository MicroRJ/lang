/*
** See Copyright Notice In lang.h
** lmodule.h
** Object/Bytecode Module
*/

typedef int Symbol;


#if 0
typedef struct CodeFile {
	char *name;
	char *diskname;
	int  bytes;
	int nbytes;
	int nfuncs;
} CodeFile;
#endif


/* Symbols
** 	Bytecode and globals can be added dynamically
** safely, in fact, multiple files will reference the
** same global by name, no matter the order in which
** they were loaded, or the means, runtime/compiletime.
** This is because we use a symbol table that maps a
** name at compile time to an index in the global values.
** Even if a file is loaded at runtime, the compilation
** process finds the global symbol and maps it to the
** target index. Lookups are effectively done at compile
** time.
*/
typedef struct Module {
	Table *g;
	Proto *p;
	Bytecode *bytes;
	Integer nbytes;
	char **lineinfo;
} Module;


Symbol lang_addsymbol(Module *md, String *name);
Symbol lang_addglobal(Module *md, String *name, Value v);
Symbol lang_addproto(Module *md, Proto p);

/*
	Module\r: runtime is stored here
for garbage collection.
	Module\gc: all objects to be automatically
managed, or garbage collected, are listed here.
By default all objects are added here, you
can however remove them from this array.

Module\gf: buffer for functions definitions,
essentially a type table, anonymous functions
are also added here.

Module\g: global symbol table which
maps names to values, indexed
at runtime by index.

Module\bytes: buffer for bytes, all the bytes
are stored here, functions index into this
buffer.

*/