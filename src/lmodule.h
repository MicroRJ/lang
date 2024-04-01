/*
** See Copyright Notice In lang.h
** lmodule.h
** lObject/Bytecode lModule
*/


typedef struct lFile {
	char *name;
	lbyteid bytes;
	lbyteid nbytes;
	int **protos;
	/* todo: eventually remove these */
	char *pathondisk;
	char *contents;
} lFile;


/*
** Symbols
** 	Bytecode and globals can be added dynamically and
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
typedef struct lModule {
	Table *g;
	Proto *p;
	Bytecode *bytes;
	lbyteid nbytes;
	char **lines;
	lFile *files;
} lModule;


lglobalid lang_addsymbol(lModule *md, lString *name);
lglobalid lang_addglobal(lModule *md, lString *name, lValue v);
lglobalid lang_addproto(lModule *md, Proto p);

/*
	lModule\r: runtime is stored here
for garbage collection.
	lModule\gc: all objects to be automatically
managed, or garbage collected, are listed here.
By default all objects are added here, you
can however remove them from this array.

lModule\gf: buffer for functions definitions,
essentially a type table, anonymous functions
are also added here.

lModule\g: global symbol table which
maps names to values, indexed
at runtime by index.

lModule\bytes: buffer for bytes, all the bytes
are stored here, functions index into this
buffer.

*/