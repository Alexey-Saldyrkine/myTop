#include <iostream>

#include "mainWindow.hpp"
using namespace std;

int main() {

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	mousemask(BUTTON1_CLICKED, nullptr);

	mainWindow win;
	bool exitFlag = true;
	while (exitFlag) {

		win.cpu->printData();
		win.proc->printData();
		update_panels();
		doupdate();
		timeout(1000);
		int c = getch();
		if (c == 'q' && win.proc->filterEditMode == 0) {
			exitFlag = false;
		} else if (c == KEY_MOUSE) {
			MEVENT event;
			if (getmouse(&event) == OK) {
				if (event.x >= 0 && event.x <= 2 && event.y == 0) {
					exitFlag = false;
				} else {
					win.processMouse(event);
				}
			}
		} else if (c == KEY_RESIZE) {
			clear();
			win.resize();
		} else {
			win.processKey(c);
		}
	}
	endwin();

}
