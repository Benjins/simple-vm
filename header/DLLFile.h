#ifndef DLLFILE_H
#define DLLFILE_H

#include <string>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#endif

using std::string;

struct DLLFile{
#if defined(_WIN32) || defined(_WIN64)
	HMODULE handle;
#else
	void* handle;
#endif

	DLLFile(){
		handle = nullptr;
	}

	~DLLFile();

	void OpenFile(const string& fileName);
	void CloseFile();
	void* GetFunction(const string& funcName) const;
};

#endif