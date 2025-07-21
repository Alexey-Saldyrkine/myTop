#include <iostream>
#include "procParser.hpp"
using namespace std;

int main() {

	path procPath("/proc");

	for (auto it : entryRange(procPath.c_str())) {
		if (isProcessName(it->d_name)) {

			path processPath(procPath);
			processPath.push(it->d_name);
			processPath.push("stat");

			procInfo info = parseProcPidStat(processPath.c_str());
			if (info.state == 'R' || info.name == "htop") {
				cout << it->d_name << ":" << endl;
				cout << info.pid << " " << info.name << " " << info.state <<" "<<info.processor<<" "<<info.starttime
						<< endl;
			}

		}
	}

}
