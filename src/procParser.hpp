#pragma once
#include "posixDirIterator.hpp"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

bool isProcessName(const nameBuf &name) {
	int i = 0;
	bool ret = true;
	while (name[i] != '\0') {
		if (!(name[i] >= '0' && name[i] <= '9') && name[i] != '\0') {
			ret = false;
			break;
		}
		i++;
	}
	return ret;
}



template<typename F>
struct auto_guard {
	F func;
	auto_guard(F f) :
			func(f) {
	}
	~auto_guard() {
		func();
	}
};

void skipToNext(const char* str, size_t& pos){
	while(str[pos++]!= ' ');
}

int getInt(const char* str, size_t& pos){
	int ret = atoi(str+pos);
	skipToNext(str,pos);
	return ret;
}

long int getLong(const char* str, size_t& pos){
	long int ret = atol(str+pos);
	skipToNext(str,pos);
	return ret;
}

long long int getLongLong(const char* str, size_t& pos){
	long long int ret = atoll(str+pos);
	skipToNext(str,pos);
	return ret;
}

char getChar(const char* str, size_t& pos){
	return str[pos++];
}

std::string getName(const char* str, size_t& pos){
	std::string ret;
	pos++;
	while(str[pos] != ')'){
		ret+=str[pos++];
	}
	pos+=2;
	return ret;

}
struct procInfo {
	int pid;			//#1
	std::string name;
	char state;
	int ppid;
	int pgrp;
	int session;
	int tty_nr;
	int tpgid;
	unsigned int flags;
	long unsigned int minflt;	//#10
	long unsigned int cminflt;
	long unsigned int majflt;
	long unsigned int cmajflt;
	long unsigned int utime;
	long unsigned int stime;
	long int cutime;
	long int cstime;
	long int priority;
	long int nice;
	long int num_threads; //#20
	long int itrealvalue;
	long long unsigned int starttime;
	long unsigned int vsize;
	long int rss;
	long unsigned int rsslim;
	long unsigned int startcode;
	long unsigned int endcode;
	long unsigned int startstack;
	long unsigned int kstkesp;
	long unsigned int kstkeip;		//#30
	long unsigned int signal;
	long unsigned int blocked;
	long unsigned int sigignore;
	long unsigned int sigcatch;
	long unsigned int wchan;
	long unsigned int nswap;
	long unsigned int cnswap;
	int exit_signal;
	int processor;
	unsigned int rt_priority;		//#40
	unsigned int policy;
	long long unsigned int delayacct_blkio_ticks;
	long unsigned int guest_time;
	long int cguest_time;
	long unsigned int start_data;
	long unsigned int end_data;
	long unsigned int start_brk;
	long unsigned int arg_start;
	long unsigned int arg_end;
	long unsigned int env_start;	//#50
	long unsigned int env_end;
	long unsigned int exit_code;

};

procInfo parseProcPidStat(const char *path) {
	FILE *stream = fopen(path, "r");
	if (stream == nullptr) {
		throw std::system_error(errno, std::generic_category(), path);
	}
	char* str = nullptr;
	size_t size=0;
	auto_guard guard([&stream,&str](){
		free(str);
		fclose(stream);
	});
	getline(&str,&size,stream);
	procInfo ret;
	size_t pos =0;
	ret.pid = getInt(str,pos); //#1
	ret.name = getName(str,pos);
	ret.state = getChar(str,pos);
	ret.ppid = getInt(str,pos);
	ret.pgrp = getInt(str,pos);
	ret.session = getInt(str,pos);
	ret.tty_nr = getInt(str,pos);
	ret.tpgid = getInt(str,pos);
	ret.flags = getInt(str,pos);
	ret.minflt = getLong(str,pos);		//$10
	ret.cminflt = getLong(str,pos);
	ret.majflt = getLong(str,pos);
	ret.cmajflt = getLong(str,pos);
	ret.utime = getLong(str,pos);
	ret.stime = getLong(str,pos);
	ret.cutime = getLong(str,pos);
	ret.cstime = getLong(str,pos);
	ret.priority = getLong(str,pos);
	ret.nice = getLong(str,pos);
	ret.num_threads = getLong(str,pos);		//#20
	ret.itrealvalue = getLong(str,pos);
	ret.starttime = getLongLong(str,pos);
	ret.vsize = getLong(str,pos);
	ret.rss = getLong(str,pos);
	ret. rsslim = getLong(str,pos);
	ret.startcode = getLong(str,pos);
	ret.endcode = getLong(str,pos);
	ret.startstack = getLong(str,pos);
	ret.kstkesp = getLong(str,pos);
	ret.kstkeip = getLong(str,pos);			//#30
	ret.signal = getLong(str,pos);
	ret.blocked = getLong(str,pos);
	ret.sigignore = getLong(str,pos);
	ret.sigcatch = getLong(str,pos);
	ret.wchan = getLong(str,pos);
	ret.nswap = getLong(str,pos);
	ret.cnswap = getLong(str,pos);
	ret.exit_signal = getInt(str,pos);
	ret.processor = getInt(str,pos);
	ret.rt_priority = getInt(str,pos);			//#40
	ret.policy = getInt(str,pos);
	ret.delayacct_blkio_ticks = getLongLong(str,pos);
	ret.guest_time = getLong(str,pos);
	ret.cguest_time = getLong(str,pos);
	ret.start_data = getLong(str,pos);
	ret.end_data = getLong(str,pos);
	ret.start_brk = getLong(str,pos);
	ret.arg_start = getLong(str,pos);
	ret.arg_end = getLong(str,pos);
	ret.env_start = getLong(str,pos);			//#50
	ret.env_end = getLong(str,pos);
	ret.exit_code = getInt(str,pos);



	return ret;
}



