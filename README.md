# myTop
my own implementation of top

Reads /proc to gather the data on processes.

Uses ncurses to display the information.

Supports hotkeys and mouse interaction.

Has sorting and filtering for processes.

Supports resizing of the terminal.

The Top 20% of the terminal display information on cpus.

The rest of the terminal displays information on the processes.

CPU information displays the name of the cpu, its usage in percent, and a bar to visually display the percent.

Process information has the following columns:
- pid - the process ID
- state - the process state
- vmem - the process virtual memory in bytes(B), kilobytes(KB), megabytes(MB) and gigabytes(GB)
- cpu# - the cpu number last executed on
- cpu% - the process cpu usage
- name - the process name. Either the exec name or the command line

Cpu usage if calculated as the amount of time spent active in an interval of time divided by the interval of time. This include child process active time. 

##Hotkeys
- q - exit
- f - enter filter edit mode. While in filter edit mode, if any characters between ' ' and '~' inclusively is pressed then it will be added to the filter. Backspace to remove the last character. Enter to exit filter edit mode.
- p - sort by pid in ascending order. If pressed twice in sequence it will sort by pid in descending order
- s - sort by state in ascending order. If pressed twice in sequence it will sort by state in descending order. The ascending order is R < S < any other state.
- v - sort by vmem in descending order. If pressed twice in sequence it will sort by vmem in ascending order.
- r - sort by cpu number in ascending order. If pressed twice in sequence it will sort by cpu number in descending order.
- c - sort by cpu usage in descending order. If pressed twice in sequence it will sort by cpu usage in ascending order.
- n - sort by name in ascending order. If pressed twice in sequence it will sort by name in descending order.
- m - switch the name of processes between the filename of the executable and the command line it was launched with

## mouse support
If mouse events are available, then the following mouse interaction are possible:
- clicking on the X at the top right of the screen will exit the program
- clicking on any of the columns names will sort them the same way as pressing their respective hotkey, including inverting the sorting.
- clicking on the Filter line will enter filter edit mode. If you are already in filter edit mode, then clicking on the Filter line will exit filter edit mode.
- clicking anywhere while in filter edit mode will result in you exiting filter edit mode and the action of any previous mouse action will occur, other than entering filter edit mode.    
