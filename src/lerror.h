/*
** See Copyright Notice In lang.h
** lerror.h
** Error Codes
*/


#if !defined(ERROR_XITEM)

#define LFAILED(err) ((err) != Error_None)
#define LPASSED(err) ((err) == Error_None)

#define ERNAME(xx) (lErrorNames[xx])

typedef enum Error {
	Error_None = 0,

#define ERROR_XITEM(NAME,DESC) Error_##NAME,
	#include __FILE__
#undef ERROR_XITEM

} Error;


lglobal char const *lErrorNames[] = {
	"No Error",

#define ERROR_XITEM(NAME,DESC) DESC,
	#include __FILE__
#undef ERROR_XITEM

};

#else
ERROR_XITEM(AsssertionTriggered,            "Assertion Triggered")
ERROR_XITEM(InternalError,                  "Internal Error")
ERROR_XITEM(OutOfMemory,                    "Out of Memory")
ERROR_XITEM(InvalidArguments,               "Invalid Arguments")
ERROR_XITEM(FileNameIsInvalid,              "File Name is Invalid")
ERROR_XITEM(FileNotFound,                   "File Was Not Found")
ERROR_XITEM(CouldNotLoadLibrary,            "Could Not Load Library")
ERROR_XITEM(CouldNotOpenFile,               "Could Not Open File")
ERROR_XITEM(CouldNotReadEntireFile,         "Could Not Read Entire File")
ERROR_XITEM(CouldNotWriteEntireFile,        "Could Not Write Entire File")
ERROR_XITEM(CouldNotReadFile,               "Could Not Read Entire File")
ERROR_XITEM(CouldNotLoadFile,               "Could Not Load Entire File")
ERROR_XITEM(Halted,                         "Halted")
ERROR_XITEM(Breaked,                        "Breaked")
#endif

