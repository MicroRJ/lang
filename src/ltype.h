/*
** See Copyright Notice In lang.h
** ltype.h
** Type Definitions
*/



typedef long long int llong;
typedef signed int lbool;
typedef double lnumber;



typedef unsigned int lhashid;
typedef int llocalid;
typedef int lbyteid;
typedef int lglobalid;
/* todo: eventually convert this to an offset */
typedef char *llineid;


typedef char lchar;
typedef void *Ptr;
typedef void *Handle;


typedef int (* lBinding)(Runtime *);


/*
** Function Prototype
*/
typedef struct Proto {
	short x,y;
	short ncaches;
	short nlocals;
	short nbytes;
	short bytes;
} Proto;



