#include "../header/DLLFile.h"

#include <iostream>

using std::cout;

#if defined(_WIN32) || defined(_WIN64)
#else
#    include <dlfcn.h>
#endif

void DLLFile::OpenFile(const string& fileName){
#if defined(_WIN32) || defined(_WIN64)
#else	
	handle = dlopen(fileName.c_str(), RTLD_LAZY);
	if (!handle) {
        fprintf (stderr, "%s\n", dlerror());
        exit(1);
    }
#endif
}

void DLLFile::CloseFile(){
#if defined(_WIN32) || defined(_WIN64)
#else
	if(handle != nullptr){
		dlclose(handle);
		handle = nullptr;
	}
#endif
}

void* DLLFile::GetFunction(const string& funcName) const{
#if defined(_WIN32) || defined(_WIN64)
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