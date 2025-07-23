#pragma once
#include "posixDirIterator.hpp"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sys/sysinfo.h>

long int getUptime() {
	struct sysinfo s_info;
	sysinfo(&s_info);
	return s_info.uptime;
}

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

void skipToNext(const char *str, size_t &pos) {
	while (str[pos++] != ' ')
		;
}

int getInt(const char *str, size_t &pos) {
	int ret = atoi(str + pos);
	skipToNext(str, pos);
	return ret;
}

int getInt(const char *str, size_t &pos, size_t len) {
	int ret = atoi(str + pos);
	return ret;
}

long int getLong(const char *str, size_t &pos) {
	long int ret = atol(str + pos);
	skipToNext(str, pos);
	return ret;
}

long long int getLongLong(const char *str, size_t &pos) {
	long long int ret = atoll(str + pos);
	skipToNext(str, pos);
	return ret;
}

char getChar(const char *str, size_t &pos) {
	char ret = str[pos];
	pos += 2;
	return ret;
}

std::string getName(const char *str, size_t &pos) {
	std::string ret;
	pos++;
	while (str[pos] != ')') {
		ret += str[pos++];
	}
	pos += 2;
	return ret;

}

std::string getString(const char *str, size_t &pos) {
	std::string ret;
	while (str[pos] != ' ') {
		ret += str[pos++];
	}
	pos += 2;
	return ret;

}

struct preProcInfo {
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

preProcInfo parseProcPidStat(const char *path) {
	FILE *stream = fopen(path, "r");
	if (stream == nullptr) {
		throw std::system_error(errno, std::generic_category(), path);
	}
	char *str = nullptr;
	size_t size = 0;
	auto_guard guard([&stream, &str]() {
		free(str);
		fclose(stream);
	});
	getline(&str, &size, stream);
	preProcInfo ret;
	size_t pos = 0;
	ret.pid = getInt(str, pos); //#1
	ret.name = getName(str, pos);
	ret.state = getChar(str, pos);
	ret.ppid = getInt(str, pos);
	ret.pgrp = getInt(str, pos);
	ret.session = getInt(str, pos);
	ret.tty_nr = getInt(str, pos);
	ret.tpgid = getInt(str, pos);
	ret.flags = getInt(str, pos);
	ret.minflt = getLong(str, pos);		//$10
	ret.cminflt = getLong(str, pos);
	ret.majflt = getLong(str, pos);
	ret.cmajflt = getLong(str, pos);
	ret.utime = getLong(str, pos);
	ret.stime = getLong(str, pos);
	ret.cutime = getLong(str, pos);
	ret.cstime = getLong(str, pos);
	ret.priority = getLong(str, pos);
	ret.nice = getLong(str, pos);
	ret.num_threads = getLong(str, pos);		//#20
	ret.itrealvalue = getLong(str, pos);
	ret.starttime = getLongLong(str, pos);
	ret.vsize = getLong(str, pos);
	ret.rss = getLong(str, pos);
	ret.rsslim = getLong(str, pos);
	ret.startcode = getLong(str, pos);
	ret.endcode = getLong(str, pos);
	ret.startstack = getLong(str, pos);
	ret.kstkesp = getLong(str, pos);
	ret.kstkeip = getLong(str, pos);			//#30
	ret.signal = getLong(str, pos);
	ret.blocked = getLong(str, pos);
	ret.sigignore = getLong(str, pos);
	ret.sigcatch = getLong(str, pos);
	ret.wchan = getLong(str, pos);
	ret.nswap = getLong(str, pos);
	ret.cnswap = getLong(str, pos);
	ret.exit_signal = getInt(str, pos);
	ret.processor = getInt(str, pos);
	ret.rt_priority = getInt(str, pos);			//#40
	ret.policy = getInt(str, pos);
	ret.delayacct_blkio_ticks = getLongLong(str, pos);
	ret.guest_time = getLong(str, pos);
	ret.cguest_time = getLong(str, pos);
	ret.start_data = getLong(str, pos);
	ret.end_data = getLong(str, pos);
	ret.start_brk = getLong(str, pos);
	ret.arg_start = getLong(str, pos);
	ret.arg_end = getLong(str, pos);
	ret.env_start = getLong(str, pos);			//#50
	ret.env_end = getLong(str, pos);
	ret.exit_code = getInt(str, pos, size);

	return ret;
}

struct procInfo {
	std::string name;
	int pid;
	char state;
	long int activeTime;
	long int startTime;
	long int vsize;
	int processor;
};

procInfo getProcInfo(const char *path) {
	procInfo ret;
	auto info = parseProcPidStat(path);
	ret.name = info.name;
	ret.pid = info.pid;
	ret.state = info.state;
	ret.vsize = info.vsize;
	ret.processor = info.processor;
	ret.startTime = info.starttime;
	ret.activeTime = info.utime + info.stime + info.cutime + info.cstime;
	return ret;
}

struct cpuStat {
	std::string name;
	long int user;
	long int nice;
	long int system;
	long int idle;
	long int iowait;
	long int irq;
	long int softirq;
	long int steal;
	long int guest;
	long int guest_nice;
};

struct procStat {
	std::vector<cpuStat> cpus;

};

procStat parseProcStat() {
	procStat ret;
	FILE *stream = fopen("/proc/stat", "r");
	if (stream == nullptr) {
		throw std::system_error(errno, std::generic_category(), "/proc/stat");
	}
	char *line = nullptr;
	size_t size = 0;
	auto_guard guard([&stream, &line]() {
		free(line);
		fclose(stream);
	});
	getline(&line, &size, stream);
	while (line[0] == 'c' && line[1] == 'p' && line[2] == 'u') {
		cpuStat stat;
		size_t pos = 0;
		stat.name = getString(line, pos);
		stat.user = getLong(line, pos);
		stat.nice = getLong(line, pos);
		stat.system = getLong(line, pos);
		stat.idle = getLong(line, pos);
		stat.iowait = getLong(line, pos);
		stat.irq = getLong(line, pos);
		stat.softirq = getLong(line, pos);
		stat.steal = getLong(line, pos);
		stat.guest = getLong(line, pos);
		stat.guest_nice = getLong(line, pos);
		ret.cpus.push_back(stat);
		getline(&line, &size, stream);
	}

	return ret;
}

struct cpuTime {
	std::string name;
	long int activeTime;
	long int idleTime;
};

std::vector<cpuTime> getCpuTimes() {
	procStat stat = parseProcStat();
	std::vector<cpuTime> ret;
	for (auto &it : stat.cpus) {
		cpuTime tmp;
		tmp.name = it.name;
		tmp.idleTime = it.idle;
		tmp.activeTime = it.user + it.system + it.nice;
		ret.push_back(tmp);
	}
	return ret;
}

std::string formatVmem(long int mem) {
	if (mem < 9999) {
		return std::to_string(mem) + "B";
	} else if (mem < 99999) {
		return std::to_string(mem / 1024) + "KB";
	} else if (mem < 9999999) {
		return std::to_string(mem / 1024 / 1024) + "MB";
	} else {
		return std::to_string(mem / 1024 / 1024 / 1024) + "GB";
	}
}

