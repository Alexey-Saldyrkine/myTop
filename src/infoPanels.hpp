#pragma once
#include "procParser.hpp"
#include <ncurses.h>
#include <ncurses/panel.h>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <unistd.h>

struct cpuInfoWin {
	WINDOW *win;
	PANEL *panel;
	long int prevUpTime = getUptime();
	std::unordered_map<std::string, long int> prevActiveTime;
	long int hz = sysconf(_SC_CLK_TCK);
	cpuInfoWin(WINDOW *parent, int w, int l, int x, int y) {
		win = newwin(l, w, x, y);
		box(win, '|', '-');
		panel = new_panel(win);
	}
	~cpuInfoWin() {
		del_panel(panel);
		delwin(win);
	}

	void printData() {
		int wy, wx;
		getmaxyx(win, wy, wx);
		auto data = getCpuTimes();
		int cols = data.size() / (wy - 2)
				+ (data.size() % (wy - 2) == 0 ? 0 : 1);
		int y = 1;
		int x = 1;
		long int upTime = getUptime();
		if (upTime != prevUpTime) {
			for (auto &it : data) {
				std::string str(it.name);

				double precent = ((double) (it.activeTime
						- prevActiveTime[it.name]) / hz) / (upTime - prevUpTime)
						* 100;
				prevActiveTime[it.name] = it.activeTime;
				str += ": " + std::to_string(precent).substr(0, 5) + "%%";

				mvwprintw(win, y++, x, str.c_str());
				if (y >= wy - 1) {
					y = 1;
					x += wx / cols;
				}
			}
			prevUpTime = upTime;
		}
	}

	operator WINDOW*() {
		return win;
	}
	operator PANEL*() {
		return panel;
	}
};

struct dataTable {
	WINDOW *win;
	int x, y;
	std::vector<std::string> colNames;

	dataTable(WINDOW *w, int wx, int wy) :
			win(w), x(wx), y(wy) {
	}

	void printLine(std::vector<std::string> data) {

	}
};

enum sortModeEnum {
	None, pidUp, pidDown, cpuUp, cpuDown,
};

struct procInfoWin {
	WINDOW *win;
	PANEL *panel;
	long int hz = sysconf(_SC_CLK_TCK);
	long int upTime;
	int listOffset = 0;
	sortModeEnum sortMode = sortModeEnum::None;
	std::unordered_map<int, std::pair<procInfo, double>> procMap;
	procInfoWin(WINDOW *parent, int w, int l, int x, int y) {
		win = newwin(l, w, x, y);
		box(win, '|', '-');
		panel = new_panel(win);
		upTime = getUptime();
		mvwprintw(win, 1, 1, "pid\tstate\tmem\tcpu#\tcpu%\tname");
	}
	~procInfoWin() {
		del_panel(panel);
		delwin(win);
	}

	void updateData() {
		path procPath("/proc");
		upTime = getUptime();
		std::unordered_set<int> pids;
		for (auto it : entryRange(procPath.c_str())) {
			if (isProcessName(it->d_name)) {

				path processPath(procPath);
				processPath.push(it->d_name);
				processPath.push("stat");
				//preProcInfo info = parseProcPidStat(processPath.c_str());
				procInfo info;

				try {
					info = getProcInfo(processPath.c_str());
					pids.insert(info.pid);
				} catch (...) {
					int pid = atoi(it->d_name);
					if (procMap.find(pid) != procMap.end()) {
						procMap.erase(pid);
					}
				}
				int pid = info.pid;
				double seconds = (double) upTime
						- ((double) info.startTime / hz);
				double percent = ((double) info.activeTime / hz) / seconds
						* 100;
				procMap[pid] = { info, percent };
			}
		}
		for (auto it = procMap.begin(); it != procMap.end();) {
			if (pids.find(it->first) == pids.end()) {
				it = procMap.erase(it);
			} else {
				it++;
			}
		}
	}

	void sortVec(std::vector<int> &vec) {
		switch (sortMode) {
		case sortModeEnum::None:
			break;
		case sortModeEnum::pidUp:
			std::sort(vec.begin(), vec.end(), [](int a, int b) {
				return a > b;
			});
			break;
		case sortModeEnum::pidDown:
			std::sort(vec.begin(), vec.end(), [](int a, int b) {
				return a < b;
			});
			break;
		case sortModeEnum::cpuUp:
			std::sort(vec.begin(), vec.end(),
					[&procMap = procMap](int a, int b) {
						return procMap[a].second < procMap[b].second;
					});
			break;
		case sortModeEnum::cpuDown:
			std::sort(vec.begin(), vec.end(),
					[&procMap = procMap](int a, int b) {
						return procMap[a].second > procMap[b].second;
					});
			break;
		}
	}

	std::vector<int> infoToVec() {
		std::vector<int> ret;
		for (auto &it : procMap) {
			ret.push_back(it.first);
		}
		sortVec(ret);
		return ret;
	}

	void printPidData(int pid, int x, int y) {
		auto &info = procMap[pid].first;
		double percent = procMap[pid].second;
		std::string str;
		str += std::to_string(info.pid) + "\t" + info.state + "\t"
				+ formatVmem(info.vsize) + "\t" + std::to_string(info.processor)
				+ "\t" + std::to_string(percent).substr(0, 5) + "%%\t"
				+ info.name;
		mvwprintw(win, x, y, str.c_str());
	}

	void printData() {
		int wy, wx;
		getmaxyx(win, wy, wx);

		unsigned int n = std::max(wy - 3, 1);
		updateData();
		auto vec = infoToVec();
		for (unsigned int i = 0; i < n; i++) {
			if (i + listOffset < vec.size()) {
				int pid = vec[i + listOffset];
				printPidData(pid, i + 2, 1);
				int x;
				getyx(win, x, x);
				wprintw(win, std::string(wx - x - 1, ' ').c_str());
			} else {
				mvwprintw(win, i + 2, 1, std::string(wx - 2, ' ').c_str());
			}
		}
	}

}
;

