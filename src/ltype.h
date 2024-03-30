/*
** See Copyright Notice In lang.h
** ltype.h
** Type Definitions
*/



typedef long long int llong;
typedef signed int lbool;
typedef double lnumber;

typedef unsigned int lhash;
typedef int llocalid;

typedef char lchar;
typedef void *Ptr;
typedef void *Handle;


typedef int (* lBinding)(Runtime *);


/*
** Function Prototype
*/
typedef struct Proto {
	short ncaches;
	short nlocals;
	short arity;
	short nbytes;
	int bytes;
} Proto;
