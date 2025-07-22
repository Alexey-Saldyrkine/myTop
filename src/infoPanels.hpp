#pragma once
#include "procParser.hpp"
#include <ncurses.h>
#include <ncurses/panel.h>
#include <unordered_map>
#include <algorithm>

struct cpuInfoWin {
	WINDOW *win;
	PANEL *panel;

	cpuInfoWin(WINDOW *parent, int w, int l, int x, int y) {
		win = newwin(l, w, x, y);
		box(win, '|', '-');
		panel = new_panel(win);
	}

	void printData() {
		int wy, wx;
		getmaxyx(win, wy, wx);
		auto data = getCpuTimes();
		int cols = data.size() / (wy - 2)
				+ (data.size() % (wy - 2) == 0 ? 0 : 1);
		int y = 1;
		int x = 1;

		for (auto &it : data) {
			std::string str(it.name);
			double precent = 100 * (double) it.activeTime
					/ (it.activeTime + it.idleTime);
			str += ": " + std::to_string(precent).substr(0, 5) + "%%";
			mvwprintw(win, y++, x, str.c_str());
			if (y >= wy - 1) {
				y = 1;
				x += wx / cols;
			}
		}
	}

	operator WINDOW*() {
		return win;
	}
	operator PANEL*() {
		return panel;
	}
};

enum sortModeEnum {
	None, pidUp, pidDown,
};

struct procInfoWin {
	WINDOW *win;
	PANEL *panel;
	long int prevUptime;
	long int upTime;
	int listOffset = 0;
	sortModeEnum sortMode = sortModeEnum::None;
	std::unordered_map<int, procInfo> procMap;
	std::unordered_map<int, long int> prevTime;
	procInfoWin(WINDOW *parent, int w, int l, int x, int y) {
		win = newwin(l, w, x, y);
		box(win, '|', '-');
		panel = new_panel(win);
		prevUptime = getUptime();
		upTime = prevUptime + 1;
		mvwprintw(win,1,1,"pid\tstate\tmem\tprocessor\tcpu");
	}

	void updateData() {
		path procPath("/proc");
		upTime = getUptime();
		for (auto it : entryRange(procPath.c_str())) {
			if (isProcessName(it->d_name)) {

				path processPath(procPath);
				processPath.push(it->d_name);
				processPath.push("stat");
				//preProcInfo info = parseProcPidStat(processPath.c_str());
				procInfo info = getProcInfo(processPath.c_str());
				int pid = info.pid;
				prevTime[pid] = info.activeTime - procMap[pid].activeTime;
				procMap[pid] = info;
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
		int dTime = upTime - prevTime[pid];
		prevTime[pid] = upTime;
		std::string str;
		auto &info = procMap[pid];
		double percent = (double) info.activeTime * 100 / (dTime);
		str += std::to_string(info.pid) + "\t" + info.state
				+ "\t" + std::to_string(info.vsize/1024) + "KB\t"
				+ std::to_string(info.processor) + "\t"
				+ std::to_string(percent).substr(0, 5) + "%%\t" + info.name;
		mvwprintw(win, x, y, str.c_str());
	}

	void printData() {
		int wy, wx;
		getmaxyx(win, wy, wx);

		int n = wy - 3;
		updateData();
		auto vec = infoToVec();
		for (int i = 0; i < n; i++) {
			if (i + listOffset >= 0 && i + listOffset < vec.size()) {
				int pid = vec[i + listOffset];
				printPidData(pid, i + 2, 1);
			}
		}
	}

}
;

