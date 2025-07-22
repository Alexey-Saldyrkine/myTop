#pragma once
#include <memory>
#include "infoPanels.hpp"

struct mainWindow{
	WINDOW* mainWin;
	PANEL* mainPanel;
	std::unique_ptr<cpuInfoWin> cpu;
	std::unique_ptr<procInfoWin> proc;
	mainWindow(){
		int width = COLS-1, Length = LINES-1;
		mainWin = newwin(Length,width,0,0);
		box(mainWin,'|','-');
		mainPanel=new_panel(mainWin);
		int cpuWidth = width, cpuLength = 3+(Length-3)*0.2;
		cpu = std::make_unique<cpuInfoWin>(mainWin,cpuWidth,cpuLength,0,0);
		int procWidth = width, procLength = 3+(Length-5)*0.8;
		proc = std::make_unique<procInfoWin>(mainWin,procWidth,procLength,cpuLength-1,0);
	}
	~mainWindow(){
		delwin(mainWin);
	}

	void processKey(int c){
		switch(c){
			case KEY_UP:
				proc->listOffset--;
				break;
			case KEY_DOWN:
				proc->listOffset++;
				break;
			case 'p':
				proc->listOffset=0;
				if(proc->sortMode == sortModeEnum::pidDown){
					proc->sortMode = sortModeEnum::pidUp;
				}else{
					proc->sortMode = sortModeEnum::pidDown;
				}
				break;
		}
	}

};
