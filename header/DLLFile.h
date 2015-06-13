#ifndef DLLFILE_H
#define DLLFILE_H

#include <string>
using std::string;

struct DLLFile{
	void* handle;

	DLLFile(){
		handle = nullptr;
	}

	~DLLFile();

	void OpenFile(const string& fileName);
	void CloseFile();
	void* GetFunction(const string& funcName) const;
};

#endif