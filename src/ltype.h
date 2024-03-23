/*
** See Copyright Notice In lang.h
** ltype.h
** Type Definitions
*/


typedef long long int Integer;
typedef signed int Bool;
typedef double Number;
typedef char Char;
typedef void *Ptr;
typedef void *Handle;


/* pseudo types? */
#define Null ("null",(Ptr)(0))
#define False ("false",(Bool)(0))
#define True ("true",(Bool)(1))


/* function's prototype */
typedef struct Proto {
	short ncaches;
	short nlocals;
	int arity;
	int bytes;
	int nbytes;
} Proto;
