/*
** See Copyright Notice Below.
** elf.h
** The λ elf language.
*/

#ifndef _elf_
#define _elf_


#if !defined(PLATFORM_DESKTOP) && !defined(PLATFORM_WEB)
#error elf-lang: No Platform Defined
#endif


#if defined(_MSC_VER)
# if !defined(ELF_KEEPWARNINGS)
#  pragma warning(push)
# endif
# pragma warning(disable:4100)
# pragma warning(disable:4245)
# pragma warning(disable:4057)
# pragma warning(disable:4189)
# pragma warning(disable:4201)
# pragma warning(disable:4244)
# pragma warning(disable:4267)
# pragma warning(disable:4389)
# pragma warning(disable:4996)
#endif

#if defined(__clang__)
# if !defined(ELF_KEEPWARNINGS)
#  pragma clang diagnostic push
# endif
# pragma clang diagnostic ignored "-Wparentheses-equality"
# pragma clang diagnostic ignored "-Wnon-literal-null-conversion"
# pragma clang diagnostic ignored "-Wmissing-braces"
# pragma clang diagnostic ignored "-Wunused-variable"
# pragma clang diagnostic ignored "-Wmissing-braces"
# pragma clang diagnostic ignored "-Wunused-function"
# pragma clang diagnostic ignored "-Wmissing-field-initializers"
# pragma clang diagnostic ignored "-Wsign-compare"
# pragma clang diagnostic ignored "-Wpointer-sign"
# pragma clang diagnostic ignored "-Wunused-function"
#endif


/* todo: remove deps */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>


#if defined(_MSC_VER)
#include <crtdbg.h>
#endif


/* em knows where this is at */
#if defined(PLATFORM_WEB)
#include <emscripten.h>
#include <unistd.h>
#endif


#ifndef STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_IMPLEMENTATION
	#include "stb/stb_sprintf.h"
#endif


/* ... */
#define LITC(xx) (xx)


#define XSTRINGIFY_(xx) #xx
#define XSTRINGIFY(xx) XSTRINGIFY_(xx)


#define XFUSE_(xx,yy) xx##yy
#define XFUSE(xx,yy) XFUSE_(xx,yy)


#if !defined(MAX)
	#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif
#if !defined(MIN)
	#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

#define elf_globaldecl static

#if defined(__EMSCRIPTEN__)
	#define elf_api EMSCRIPTEN_KEEPALIVE
	#define elf_libfundecl EMSCRIPTEN_KEEPALIVE
	#define elf_threaddecl static
#else
	#define elf_api static
	#define elf_libfundecl __declspec(dllexport)
	#define elf_threaddecl static __declspec(thread)
#endif


#define MEGABYTES(x) ((x)*1024llu*1024llu)
#define GIGABYTES(x) ((x)*1024llu*1024llu*1024llu)

/* todo: */
#if !defined(_WIN32)
	#define MAX_PATH 0xff
#endif


#define lfalse ("false",(elf_bool)(0))
#define ltrue ("true",(elf_bool)(1))
#define lnil ("nil",(Ptr)(0))


typedef struct elf_ThreadState elf_ThreadState;
typedef struct elf_Module elf_Module;
typedef struct elf_FileState elf_FileState;
typedef struct elf_Table elf_Table;
typedef struct elf_String elf_String;
typedef struct elf_Object elf_Object;
typedef struct elf_Closure elf_Closure;


#include "src/elf-tys.h"
#include "src/lerror.h"
#include "src/ldebug.h"
#include "src/lmem.h"
#include "src/elf-sys.h"
#include "src/llog.h"
#include "src/elf-obj.h"
#include "src/elf-aux.h"
#include "src/elf-byte.h"
#include "src/elf-mod.h"
#include "src/elf-api.h"
#include "src/ltoken.h"
#include "src/elf-run.h"
#include "src/elf-str.h"
#include "src/elf-var.h"
#include "src/elf-tab.h"
#include "src/lnode.h"
#include "src/lcode.h"
#include "src/lfile.h"


#include "src/elf-sys.c"
#include "src/lmem.c"
#include "src/ldebug.c"
#include "src/llog.c"
#include "src/elf-obj.c"
#include "src/lgc.c"
#include "src/elf-mod.c"
#include "src/elf-aux.c"
#include "src/elf-str.c"
#include "src/elf-var.c"
#include "src/elf-tab.c"
#include "src/lfunc.c"
#include "src/llexer.c"
#include "src/lnode.c"
#include "src/lcode.c"
#include "src/lfile.c"
#include "src/elf-api.c"


#if !defined(ELF_NOLIBS)
# include "src/ltest.c"
# include "src/elf-lib.c"
# include "src/elf-web.c"
# include "src/lcrtlib.c"
# include "src/lnetlib.c"
#endif


#include "src/elf-run.c"


#if !defined(ELF_KEEPWARNINGS)
# if defined(_MSC_VER)
#  pragma warning(pop)
# if defined(__clang__)
#  pragma clang diagnostic pop
# endif
#endif

#endif

#endif
/*
** Copyright (C) 2023-2024 Dayan Rodriguez
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
*/