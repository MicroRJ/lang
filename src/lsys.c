/*
** See Copyright Notice In lang.h
** lsys.c
** System Tools
*/


LAPI int sys_getlasterror() {
#if defined(_WIN32)
	return GetLastError();
#endif
}


LAPI void sys_geterrormsg(int error, char *buff, int len) {
#if defined(_WIN32)
	if (error == 0) error = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,0x00,error,LANG_USER_DEFAULT,buff,len,NULL);
#endif
}


LAPI void *sys_valloc(Integer length) {
#if defined(_WIN32)
	return VirtualAlloc(NULL,length,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
#endif
}


LAPI void sys_sleep(Integer ms) {
#if defined(_WIN32)
	Sleep((DWORD) ms);
#endif
}


LAPI Integer sys_clockhz() {
#if defined(_WIN32)
	LARGE_INTEGER largeInt;
	QueryPerformanceFrequency(&largeInt);
	return largeInt.QuadPart;
#endif
}


LAPI Integer sys_clocktime() {
#if defined(_WIN32)
	LARGE_INTEGER largeInt;
	QueryPerformanceCounter(&largeInt);
	return largeInt.QuadPart;
#endif
}


LAPI int sys_getmyname(int length, char *buffer) {
#if defined(_WIN32)
	return GetModuleFileName(NULL,buffer,length);
#endif
}


LAPI int sys_pwd(int length, char *buffer) {
#if defined(_WIN32)
	return GetCurrentDirectory(length,buffer);
#endif
}


LAPI Handle sys_loaddll(char const *name) {
#if defined(_WIN32)
	return (Handle) LoadLibraryA(name);
#endif
}


LAPI void *sys_finddllfn(Handle dll, char const *name) {
#if defined(_WIN32)
	return (void *) GetProcAddress(dll,name);
#endif
}


/* todo: convert this to windows version */
LAPI Handle sys_fopen(char *name, char *flags) {
	FILE *file = Null;
	fopen_s(&file,name,flags);
	return (Handle) file;
}


LAPI Error sys_loadfilebytes(Alloc *allocator, void **data, char const *name) {

	Error error = Error_None;

	if (name == Null) {
		error = Error_FileNameIsInvalid;
		goto leave;
	}
	if (data == Null) {
		error = Error_InvalidArguments;
		goto leave;
	}

	*data = Null;
#if defined(_WIN32)
	HANDLE hfile = CreateFileA(name,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0x00,NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
		DWORD hi,lo = GetFileSize(hfile,&hi);
		char *buf = langM_alloc(allocator,lo+1);
		DWORD bytes;
		if (ReadFile(hfile,buf,lo,&bytes,NULL)) {
			buf[bytes] = 0;
			*data = buf;
			if (bytes != lo) {
				error = Error_CouldNotReadEntireFile;
				goto leave;
			}
		} else {
			langM_dealloc(allocator,buf);
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
	if (file == Null) {
		error = Error_FileNotFound;
		goto leave;
	}
	fseek(file,0,SEEK_END);
	long fileSize = ftell(file);
	fseek(file,0,SEEK_SET);
	char *buf = (char *) langM_alloc(money,fileSize+1);
	fread(buf,1,fileSize,file);
	fclose(file);
	buf[fileSize] = 0;
	*data = buf;
#endif
	leave:
	// if LPASSED(error) {
	// 	lang_loginfo("'%s': file loaded",name);
	// } else {
	// 	lang_loginfo("'%s': failed to load file, %s",name,ERNAME(error));
	// }
	return error;

}


LAPI Error sys_savefilebytes(char const *buffer, Integer length, char const *fileName) {

	FILE *file;
	fopen_s(&file,fileName,"wb");

	if (file == Null) {
		return Error_CouldNotOpenFile;
	}

	Error error = Error_None;
	Integer lengthWritten = fwrite(buffer, 1, length, file);

	if (lengthWritten != length) {
		error = Error_CouldNotWriteEntireFile;
	}

	fclose(file);

	return error;
}
