#pragma once
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <system_error>

using nameBuf = char[NAME_MAX+1];
using dirent = struct dirent;


struct path{
	std::string pathString;

	path(const char * strPtr):pathString(strPtr){
	}

	path(const path& other){
		pathString = other.pathString;
	}

	void push(const char* buf){
		std::string nStr(buf);
		pathString+='/';
		pathString+=nStr;
	}

	void pop(){
		auto i = pathString.rfind('/');
		pathString.erase(pathString.begin()+i,pathString.end());
	}

	const char* c_str(){
		return pathString.c_str();
	}
};

struct entry_iterator {
	DIR *dirPtr;
	dirent *ptr;

	entry_iterator():dirPtr(nullptr),ptr(nullptr){

	}

	entry_iterator(const char *path) {
		dirPtr = opendir(path);
		if (dirPtr == nullptr) {
			throw std::system_error(errno, std::generic_category(), path);
		}
		ptr = readdir(dirPtr);

	}

	~entry_iterator(){
		closedir(dirPtr);
	}

	friend bool operator==(const entry_iterator &a, const entry_iterator &b) {
		return a.ptr == b.ptr;
	}

	friend bool operator!=(const entry_iterator &a, const entry_iterator &b) {
		return !(a == b);
	}

	dirent* operator->(){
		return ptr;
	}

	dirent* operator*(){
		return ptr;
	}

	dirent* operator++(int) {
		auto *ret = ptr;
		ptr = readdir(dirPtr);
		return ret;
	}
	dirent* operator++() {
		ptr = readdir(dirPtr);
		return ptr;
	}
};


struct entryRange{
	const char* p;
	entryRange(const char* path):p(path){}

	entry_iterator begin(){
		return entry_iterator{p};
	}

	entry_iterator end(){
		return entry_iterator{};
	}
};
