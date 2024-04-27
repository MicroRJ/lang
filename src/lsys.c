/*
** See Copyright Notice In elf.h
** lsys.c
** System Tools
*/


/* em knows where this is at */
#if defined(PLATFORM_WEB)
#include <emscripten.h>
#elif defined(_WIN32)
#pragma comment(lib,"user32")
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define _NO_CRT_STDIO_INLINE
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <Windowsx.h>
#else
/* otherwise user is prob on a calculator */
#endif



lapi elf_bool sys_debugger() {
#if defined(PLATFORM_WIN32)
	DebugBreak();
	return 1;
#elif defined(PLATFORM_WEB)
	emscripten_debugger();
	return 1;
#else
	return 0;
#endif
}



lapi void sys_consolelog(int type, char *message) {
#if defined(PLATFORM_WIN32)
	OutputDebugStringA(message);
#else
	switch (type) {
		case ELF_LOGDBUG: case ELF_LOGINFO: {
			type = EM_LOG_CONSOLE;
		} break;
		case ELF_LOGERROR: case ELF_LOGFATAL: {
			type = EM_LOG_ERROR;
		} break;
		case ELF_LOGWARN: {
		 	type = EM_LOG_WARN;
		} break;
	}
	emscripten_log(type,message);
#endif
}


lapi int sys_getlasterror() {
#if defined(_WIN32)
	return GetLastError();
#else
	return 0;
#endif
}


lapi void sys_geterrormsg(int error, char *buff, int len) {
#if defined(_WIN32)
	if (error == 0) error = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,0x00,error,LANG_USER_DEFAULT,buff,len,NULL);
#endif
}


lapi void *sys_valloc(elf_int length) {
#if defined(_WIN32)
	return VirtualAlloc(NULL,length,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
#else
	return lnil;
#endif
}


/* timing */

lapi void sys_sleep(elf_int ms) {
#if defined(PLATFORM_WIN32)
	Sleep((DWORD) ms);
#elif defined(PLATFORM_WEB)
	emscripten_sleep(ms);
#else
#endif
}


lapi elf_int sys_clockhz() {
#if defined(_WIN32)
	LARGE_INTEGER largeInt;
	QueryPerformanceFrequency(&largeInt);
	return largeInt.QuadPart;
#else
	return 0;
#endif
}


lapi elf_int sys_clocktime() {
#if defined(_WIN32)
	LARGE_INTEGER largeInt;
	QueryPerformanceCounter(&largeInt);
	return largeInt.QuadPart;
#else
	return 0;
#endif
}


lapi int sys_getmyname(int length, char *buffer) {
#if defined(_WIN32)
	return GetModuleFileName(NULL,buffer,length);
#else
	return 0;
#endif
}


lapi int sys_getmypid() {
#if defined(_WIN32)
	return GetCurrentProcessId();
#else
	return 0;
#endif
}


lapi int sys_pwd(int length, char *buffer) {
#if defined(_WIN32)
	return GetCurrentDirectory(length,buffer);
#else
	return 0;
#endif
}


lapi int sys_setpwd(char *buffer) {
#if defined(_WIN32)
	return SetCurrentDirectory(buffer);
#else
	return 0;
#endif
}


lapi elf_Handle sys_loadlib(char const *name) {
#if defined(_WIN32)
	return (elf_Handle) LoadLibraryA(name);
#else
	return 0;
#endif
}


lapi void *sys_libfn(elf_Handle dll, char const *name) {
#if defined(_WIN32)
	return (void *) GetProcAddress(dll,name);
#else
	return 0;
#endif
}


lapi Error sys_loadfilebytes(Alloc *allocfn, void **data, char const *name) {

	Error error = Error_None;

	if (name == lnil) {
		error = Error_FileNameIsInvalid;
		goto leave;
	}
	if (data == lnil) {
		error = Error_InvalidArguments;
		goto leave;
	}

	*data = lnil;
#if defined(_WIN32)
	HANDLE hfile = CreateFileA(name,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0x00,NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
		DWORD hi,lo = GetFileSize(hfile,&hi);
		char *buf = langM_alloc(allocfn,lo+1);
		DWORD bytes;
		if (ReadFile(hfile,buf,lo,&bytes,NULL)) {
			buf[bytes] = 0;
			*data = buf;
			if (bytes != lo) {
				error = Error_CouldNotReadEntireFile;
				goto leave;
			}
		} else {
			elf_delmem(allocfn,buf);
			error = Error_CouldNotReadFile;
			goto leave;
		}
		CloseHandle(hfile);
	} else {
		DWORD lasterror = GetLastError();
		if (lasterror == ERROR_FILE_NOT_FOUND) {
			error = Error_FileNotFound;
		} else {
			error = Error_CouldNotLoadFile;
		}
	}
#else
	FILE *file = fopen(name,"rb");
	if (file == lnil) {
		error = Error_FileNotFound;
		goto leave;
	}
	fseek(file,0,SEEK_END);
	long fileSize = ftell(file);
	fseek(file,0,SEEK_SET);
	char *buf = (char *) langM_alloc(allocfn,fileSize+1);
	fread(buf,1,fileSize,file);
	fclose(file);
	buf[fileSize] = 0;
	*data = buf;
#endif
	leave:
	// if LPASSED(error) {
	// 	elf_loginfo("'%s': file loaded",name);
	// } else {
	// 	elf_loginfo("'%s': failed to load file, %s",name,ERNAME(error));
	// }
	return error;

}


lapi Error sys_savefilebytes(char const *buffer, elf_int length, char const *fileName) {
	FILE *file;
#if defined(_MSC_VER)
	fopen_s(&file,fileName,"wb");
#else
	file = fopen(fileName,"wb");
#endif

	if (file == lnil) {
		return Error_CouldNotOpenFile;
	}

	Error error = Error_None;
	elf_int lengthWritten = fwrite(buffer, 1, length, file);

	if (lengthWritten != length) {
		error = Error_CouldNotWriteEntireFile;
	}

	fclose(file);

	return error;
}
