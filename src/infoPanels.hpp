#pragma once
#include "procParser.hpp"
#include <ncurses.h>
#include <ncurses/panel.h>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <unistd.h>

std::string stringProgressBar(WINDOW *win, int len, double percent) {
	if (percent > 1)
		percent = 1;
	len -= 2;
	std::string str { "[" };
	int barLen = len * percent;
	str += std::string(barLen, '|');
	str += std::string(len - barLen, ' ');
	str += "]";
	return str;
}

struct cpuInfoWin {
	WINDOW *win;
	PANEL *panel;
	long int prevUpTime = getUptime();
	std::unordered_map<std::string, long int> prevActiveTime;
	long int hz = sysconf(_SC_CLK_TCK);
	cpuInfoWin(WINDOW *parent, int w, int l, int x, int y) {
		win = newwin(l, w, x, y);
		box(win, '|', '-');
		mvwprintw(win, 0, 0, "[X]");
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

				double percent = ((double) (it.activeTime
						- prevActiveTime[it.name]) / hz)
						/ (upTime - prevUpTime);
				prevActiveTime[it.name] = it.activeTime;
				if (it.name == "cpu") {
					percent /= (data.size());
				}
				str += " " + std::to_string(percent * 100).substr(0, 4) + "%% ";
				str += stringProgressBar(win, wx / cols - str.size() - 1,
						percent);

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
	WINDOW *win = nullptr;
	int x = 0, y = 0;
	std::vector<std::string> colNames;
	std::vector<int> colWidth;
	int sum = 0;
	int solidCount = 0;
	unsigned int softWidth = 0;
	std::string filter = "";
	dataTable(std::vector<std::string> colN, std::vector<int> colW) :
			colNames(colN), colWidth(colW) {
		for (int i : colWidth) {
			if (i != 0) {
				solidCount++;
			}
			sum += i;
		}

	}

	void init(WINDOW *w) {
		win = w;
		getmaxyx(win, y, x);
		softWidth = (x - sum - 2) / (colNames.size() + 1 - solidCount);
		printHeaders();
		updateFilterBar();
	}

	void updateFilterBar() {
		std::string str { "Filter:" };
		str += filter;

		int len = x - 2 - str.size();
		if (len < 0) {
			str = str.substr(0, x - 2);
		} else {
			str += std::string(len, ' ');
		}
		mvwprintw(win, 1, 1, str.c_str());
	}
	void processFilterKey(int c) {
		if (' ' <= c && c <= '~') {
			filter += (char) c;
		} else if (c == KEY_BACKSPACE) {
			if (filter.size() > 0)
				filter.pop_back();
		}
		updateFilterBar();
	}

	void printHeaders() {
		std::string str;
		for (unsigned int i = 0; i < colNames.size(); i++) {
			unsigned int width = (
					(i < colWidth.size() && colWidth[i] != 0) ?
							colWidth[i] + 1 : softWidth);
			if (colNames[i].size() > width) {
				str += colNames[i].substr(0, width);
			} else {
				str += colNames[i];
				int spaceLen = std::max<int>(0, width - colNames[i].size());
				str += std::string(spaceLen, ' ');
			}
		}
		mvwprintw(win, 2, 1, str.c_str());

	}

	bool testFilter(const std::vector<std::string> &data) {
		bool ret = false;
		for (const auto &str : data) {
			if (str.find(filter) != std::string::npos) {
				ret = true;
				break;
			}
		}
		return ret;
	}

	void printLine(int i, const std::vector<std::string> &data) {
		std::string str;
		for (unsigned int i = 0; i < data.size(); i++) {
			unsigned int width = (
					(i < colWidth.size() && colWidth[i] != 0) ?
							colWidth[i] + 1 : softWidth);
			if (data[i].size() > width) {
				str += data[i].substr(0, width);
			} else {
				str += data[i];
				int spaceLen = std::max<int>(0, width - data[i].size());
				str += std::string(spaceLen, ' ');
			}
		}
		mvwprintw(win, i, 1, str.c_str());

	}
};

enum sortModeEnum {
	None,
	pidUp,
	pidDown,
	cpuUp,
	cpuDown,
	nameUp,
	nameDown,
	stateUp,
	stateDown,
	vmemUp,
	vmemDown,
	cpuNumUp,
	cpuNumDown
};

int stateToScore(char c) {
	switch (c) {
	case 'R':
		return 0;
		break;
	case 'S':
		return 1;
		break;
	case 'Z':
		return 2;
		break;
	default:
		return 99;
		break;
	}
}

struct procInfoWin {
	WINDOW *win;
	PANEL *panel;
	long int hz = sysconf(_SC_CLK_TCK);
	long int upTime;
	int listOffset = 0;
	bool toggleName = 1;
	bool filterEditMode = 0;
	sortModeEnum sortMode = sortModeEnum::None;
	std::unordered_map<int, procInfo> procMap;
	dataTable table { { "pid", "state", "vmem", "cpu#", "cpu%%", "name" }, { 6,
			6, 6, 5, 6 } };
	std::vector<int> colChar { 'p', 's', 'v', 'r', 'c', 'n' };
	procInfoWin(WINDOW *parent, int w, int l, int x, int y) {
		win = newwin(l, w, x, y);
		box(win, '|', '-');
		panel = new_panel(win);
		upTime = getUptime();
		table.init(win);
	}
	~procInfoWin() {
		del_panel(panel);
		delwin(win);
	}

	template<sortModeEnum base, sortModeEnum toggle>
	void toggleSortMode() {
		if (sortMode == base) {
			sortMode = toggle;
		} else {
			sortMode = base;
		}
	}

	void processMouse(MEVENT event) {
		if (filterEditMode) {
			filterEditMode = false;
		}
		if (event.y == 1 && filterEditMode == false) {
			filterEditMode = true;
		} else if (event.y == 2) {
			int x = event.x;
			unsigned int i = 0;
			for (int w : table.colWidth) {
				w += 1;
				if (x > w) {
					x -= w;
					i++;
				} else {
					processChar(colChar[i]);
					return;
				}
			}
			if (x > 0) {
				processChar(colChar.back());
			}
		}
	}

	void processChar(int c) {

		if (c == KEY_UP) {
			listOffset--;
			if (listOffset < 0)
				listOffset = 0;
		} else if (c == KEY_DOWN) {
			listOffset++;
		}
		if (filterEditMode == 1) {
			switch (c) {
			case '\n':
			case '\r':
			case KEY_ENTER:
				filterEditMode = false;
				break;
			default:
				table.processFilterKey(c);
				break;
			}
		} else {
			switch (c) {
			case 'f':
				filterEditMode = 1;
				break;
			case 'o':
				sortMode = sortModeEnum::None;
				break;
			case 'm':
				toggleName = !toggleName;
				break;
			case 'c':
				listOffset = 0;
				toggleSortMode<sortModeEnum::cpuDown, sortModeEnum::cpuUp>();
				break;
			case 'p':
				listOffset = 0;
				toggleSortMode<sortModeEnum::pidDown, sortModeEnum::pidUp>();
				break;
			case 'n':
				listOffset = 0;
				toggleSortMode<sortModeEnum::nameUp, sortModeEnum::nameDown>();
				break;
			case 's':
				listOffset = 0;
				toggleSortMode<sortModeEnum::stateUp, sortModeEnum::stateDown>();
				break;
			case 'v':
				listOffset = 0;
				toggleSortMode<sortModeEnum::vmemDown, sortModeEnum::vmemUp>();
				break;
			case 'r':
				listOffset = 0;
				toggleSortMode<sortModeEnum::cpuNumUp, sortModeEnum::cpuNumDown>();
				break;
			}

		}

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
					processPath.pop();
					processPath.push("cmdline");
					info.cmdLine = getProcPidCmdline(processPath.c_str());
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
				info.cpuUsage = percent;
				procMap[pid] = info;
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
						if (procMap[a].cpuUsage != procMap[b].cpuUsage) {
							return procMap[a].cpuUsage < procMap[b].cpuUsage;
						} else {
							return a < b;
						}
					});
			break;
		case sortModeEnum::cpuDown:
			std::sort(vec.begin(), vec.end(),
					[&procMap = procMap](int a, int b) {
						if (procMap[a].cpuUsage != procMap[b].cpuUsage) {
							return procMap[a].cpuUsage > procMap[b].cpuUsage;
						} else {
							return a < b;
						}
					});
			break;
		case sortModeEnum::nameDown:
			std::sort(vec.begin(), vec.end(),
					[&procMap = procMap](int a, int b) {
						if (procMap[a].name != procMap[b].name) {
							return procMap[a].name > procMap[b].name;
						} else {
							return a < b;
						}
					});
			break;
		case sortModeEnum::nameUp:
			std::sort(vec.begin(), vec.end(),
					[&procMap = procMap](int a, int b) {
						if (procMap[a].name != procMap[b].name) {
							return procMap[a].name < procMap[b].name;
						} else {
							return a < b;
						}
					});
			break;
		case sortModeEnum::stateDown:
			std::sort(vec.begin(), vec.end(),
					[&procMap = procMap](int a, int b) {
						if (stateToScore(procMap[a].state)
								!= stateToScore(procMap[b].state)) {
							return stateToScore(procMap[a].state)
									> stateToScore(procMap[b].state);
						} else {
							return a < b;
						}
					});
			break;
		case sortModeEnum::stateUp:
			std::sort(vec.begin(), vec.end(),
					[&procMap = procMap](int a, int b) {
						if (stateToScore(procMap[a].state)
								!= stateToScore(procMap[b].state)) {
							return stateToScore(procMap[a].state)
									< stateToScore(procMap[b].state);
						} else {
							return a < b;
						}
					});
			break;
		case sortModeEnum::vmemDown:
			std::sort(vec.begin(), vec.end(),
					[&procMap = procMap](int a, int b) {
						if (procMap[a].vsize != procMap[b].vsize) {
							return procMap[a].vsize > procMap[b].vsize;
						} else {
							return a < b;
						}
					});
			break;
		case sortModeEnum::vmemUp:
			std::sort(vec.begin(), vec.end(),
					[&procMap = procMap](int a, int b) {
						if (procMap[a].vsize != procMap[b].vsize) {
							return procMap[a].vsize < procMap[b].vsize;
						} else {
							return a < b;
						}
					});
			break;
		case sortModeEnum::cpuNumUp:
			std::sort(vec.begin(), vec.end(),
					[&procMap = procMap](int a, int b) {
						if (procMap[a].processor != procMap[b].processor) {
							return procMap[a].processor < procMap[b].processor;
						} else {
							return a < b;
						}
					});
			break;
		case sortModeEnum::cpuNumDown:
			std::sort(vec.begin(), vec.end(),
					[&procMap = procMap](int a, int b) {
						if (procMap[a].processor != procMap[b].processor) {
							return procMap[a].processor > procMap[b].processor;
						} else {
							return a < b;
						}
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

	std::vector<std::string> pidToData(int pid) {
		auto info = procMap[pid];
		std::string name = (
				toggleName == 0 ?
						(info.cmdLine.size() > 0 ? info.cmdLine : info.name) :
						info.name);
		return {std::to_string(info.pid), std::string(1, info.state), formatVmem(
					info.vsize), std::to_string(info.processor), std::to_string(
					info.cpuUsage).substr(0, 4) + "%%", name};
	}

	void printPidData(const std::vector<std::string> &data, int x) {
		table.printLine(x, data);
	}

	void printData() {
		int wy, wx;
		getmaxyx(win, wy, wx);

		unsigned int n = std::max(wy - 4, 1);
		updateData();
		int pidFilterOffset = 0;
		auto vec = infoToVec();
		for (unsigned int i = 0; i < n; i++) {
			if (i + listOffset + pidFilterOffset < vec.size()) {
				int pid = vec[i + listOffset + pidFilterOffset];
				std::vector<std::string> data = pidToData(pid);
				bool earlyCountinueFlag = false;
				while (table.testFilter(data) == false) {
					pidFilterOffset++;
					if (i + listOffset + pidFilterOffset >= vec.size()) {
						earlyCountinueFlag = true;
						break;
					}
					pid = vec[i + listOffset + pidFilterOffset];
					data = pidToData(pid);
				}
				if (earlyCountinueFlag) {
					n++;
					continue;
				}
				printPidData(data, i + 3);
				int y, x;
				getyx(win, y, x);
				y = y + 0;
				wprintw(win, std::string(wx - x - 1, ' ').c_str());
			} else {
				mvwprintw(win, i + 2, 1, std::string(wx - 2, ' ').c_str());
			}
		}
	}

}
;

