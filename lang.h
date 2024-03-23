/*
** See Copyright Notice In File Footer
** lang.h
** The programming language with no name.
*/
#ifndef _lang_
#define _lang_

#pragma warning(push)
#pragma warning(disable:4100)
#pragma warning(disable:4189)
#pragma warning(disable:4201)
#pragma warning(disable:4244)
#pragma warning(disable:4267)
#pragma warning(disable:4389)
#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <stdarg.h>
#include <math.h> /* math functions */

#ifndef STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_IMPLEMENTATION
	#include <stb/stb_sprintf.h>
#endif

#define XSTRINGIFY_(xx) #xx
#define XSTRINGIFY(xx) XSTRINGIFY_(xx)

#define XFUSE_(xx,yy) xx##yy
#define XFUSE(xx,yy) XFUSE_(xx,yy)

#if !defined(LAPI)
	#define LAPI static
#endif

#define lglobal static



#include <src/ltype.h>
#include <src/lerror.h>
#include <src/ldebug.h>
#include <src/llog.h>
#include <src/lmem.h>
#include <src/lsys.h>

typedef struct Runtime Runtime;
typedef struct Table Table;
typedef struct FileState FileState;

#include <src/llexer.h>
#include <src/lbyte.h>
#include <src/lobject.h>
#include <src/lmodule.h>
#include <src/lruntime.h>
#include <src/lstring.h>
#include <src/larray.h>
#include <src/ltable.h>
#include <src/lnode.h>
#include <src/lcode.h>
#include <src/lfile.h>
#include <src/ldoaux.h>


#if defined(_WIN32)
	#pragma comment(lib,"user32")
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#define _NO_CRT_STDIO_INLINE
	#define _CRT_SECURE_NO_WARNINGS
	#include <windows.h>
	#include <Windowsx.h>
#endif


#include <src/lsys.c>
#include <src/lmem.c>
#include <src/ldebug.c>
#include <src/llog.c>
#include <src/lgc.c>
#include <src/lmodule.c>
#include <src/lstring.c>
#include <src/larray.c>
#include <src/ltable.c>
#include <src/lfunc.c>
#include <src/llexer.c>
#include <src/lnode.c>
#include <src/lcode.c>
#include <src/lfile.c>
#include <src/ltest.c>
#include <src/lsyslib.c>
#include <src/ldoaux.c>
#include <src/lruntime.c>


Integer lang_clocktime() {
	return sys_clocktime();
}


Number lang_timediffs(Integer begin) {
	/* todo: clockhz can be cached */
	return (sys_clocktime() - begin) / (Number) sys_clockhz();
}

#pragma warning(pop)
#endif

/******************************************************************************
* Copyright (C) 2023-2024 Dayan Rodriguez
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/