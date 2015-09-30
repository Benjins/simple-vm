#include "../header/DLLFile.h"

#include <iostream>
#include <stdio.h>

using std::cout;

#if defined(_WIN32) || defined(_WIN64)
#else
#    include <dlfcn.h>
#endif

void DLLFile::OpenFile(const string& fileName){
#if defined(_WIN32) || defined(_WIN64)
	handle = LoadLibraryA(fileName.c_str());
#else	
	handle = dlopen(fileName.c_str(), RTLD_LAZY);
	if (!handle) {
        fprintf (stderr, "%s\n", dlerror());
    }
#endif
}

void DLLFile::CloseFile(){
#if defined(_WIN32) || defined(_WIN64)
	if(handle != 0){
		FreeLibrary(handle);
		handle = 0;
	}
#else
	if(handle != nullptr){
		dlclose(handle);
		handle = nullptr;
	}
#endif
}

void* DLLFile::GetFunction(const string& funcName) const{
#if defined(_WIN32) || defined(_WIN64)
	void* func = GetProcAddress(handle, funcName.c_str());
	return func;
#else
	void* func = dlsym(handle, funcName.c_str());
	char *error;
	if ((error = dlerror()) != NULL)  {
        cout << error << "\n";
    }

    return func;
#endif
}

DLLFile::~DLLFile(){
#if defined(_WIN32) || defined(_WIN64)
#else
	if(handle != nullptr){
		dlclose(handle);
	}
#endif
}