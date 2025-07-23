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
		} else if (c == KEY_RESIZE) {
			clear();
			win.resize();
		} else {
			win.processKey(c);
		}
	}
	endwin();

}
