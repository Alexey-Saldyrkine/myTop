#pragma once
#include <memory>
#include "infoPanels.hpp"

struct mainWindow {
	WINDOW *mainWin;
	PANEL *mainPanel;
	std::unique_ptr<cpuInfoWin> cpu;
	std::unique_ptr<procInfoWin> proc;
	mainWindow() {
		initWindows();
	}
	~mainWindow() {
		delwin(mainWin);
	}

	void initWindows() {
		int width = COLS - 1, Length = LINES - 1;
		mainWin = newwin(Length, width, 0, 0);
		box(mainWin, '|', '-');
		mvwprintw(mainWin,0,0,"X");
		mainPanel = new_panel(mainWin);
		int cpuWidth = width, cpuLength = 3 + (Length - 3) * 0.2;
		cpu = std::make_unique<cpuInfoWin>(mainWin, cpuWidth, cpuLength, 0, 0);
		int procWidth = width, procLength = 3 + (Length - 5) * 0.8;
		proc = std::make_unique<procInfoWin>(mainWin, procWidth, procLength,
				cpuLength - 1, 0);
	}

	void resize() {
		del_panel(mainPanel);
		delwin(mainWin);
		initWindows();
	}

	void processKey(int c) {
		proc->processChar(c);
	}

	void processMouse(MEVENT event) {
		int x, y;
		getbegyx(proc->win, y, x);
		event.x -= x;
		event.y -= y;
		proc->processMouse(event);
	}

};
