#include <iostream>

#include "mainWindow.hpp"
using namespace std;

int main() {

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	mainWindow win;
	bool exitFlag = true;
	while (exitFlag) {

		win.cpu->printData();
		win.proc->printData();
		update_panels();
		doupdate();
		timeout(500);
		int c = getch();
		if (c == 'q') {
			exitFlag = false;
		} else {
			win.processKey(c);
		}
	}
	endwin();

	/*
	 cout<<"cpu: \n";
	 for(auto it: getCpuTimes()){
	 cout<<it.name<<" actv: "<<it.activeTime<<" idle: "<<it.idleTime
	 <<" %"<<((double)it.activeTime/(it.activeTime+it.idleTime)*100)<<endl;
	 }
	 cout<<"proc: \n";
	 path procPath("/proc");

	 for (auto it : entryRange(procPath.c_str())) {
	 if (isProcessName(it->d_name)) {

	 path processPath(procPath);
	 processPath.push(it->d_name);
	 processPath.push("stat");

	 preProcInfo info = parseProcPidStat(processPath.c_str());
	 if (info.state == 'R' || info.name == "htop") {
	 cout << it->d_name << ":" << endl;
	 cout << info.pid << " " << info.name << " " << info.state <<" "<<info.processor<<" "<<info.starttime
	 << endl;
	 }

	 }
	 }*/

}
