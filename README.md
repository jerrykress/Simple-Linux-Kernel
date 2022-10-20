# Simple-Linux-Kernel

**Note on license: This project started as a part of the Concurrent Programming (COMS20001) course at University of Bristol. The original skeleton code included the structure of the project, hence the licensing in many files. My implementation included all hilevel and lolevel files under /system/kernel/ which were layed out by lecturer Daniel Page.**


Customised linux kernel with support for concurrent program execution using context switching and pid priority queue.
Customidsd basic GUI support that displays print messages from the executed C programs.
Basic I/O support such as mouse cursor and keyboard input.

![alt text](https://github.com/jerrykress/Simple-Linux-Kernel/blob/master/screenshot.png?raw=true)

## Requirements
```bash
# install qemu
sudo apt install qemu-system-arm
```

## Launching QEMU
```bash
# build the kernel images
make build
# launch a QEMU instance
make launch-qemu
# connect to display
telnet 127.0.0.1 1235
# attach debugger
make launch-gdb
# continue in GDB terminal
(gdb) continue
```

## Features
- Graphical user interface (GUI)
- Keyboard and mouse input
- Execute/Terminate user programs
- Concurrent multi-tasking through context switching
- Taskbar
- PID priority
- Simple shell terminal (lemon$)

## Lemon Shell
```bash
# Three sample user programs are provided which simply print "P1, P2, P3"
# Launch one of these user programs
lemon$ execute p1
# Launch another program
lemon$ execute p2
# Terminate a user program
lemon$ terminate p1
```

## Graphical User Interface
- Click star button to prioritise a PID
- Click check button to prioritise the output of a program
- Click stop sign button to terminate a user program
- Use arrow buttons or taskbar to switch between user programs


## Issues
- QEMU keymaps may be broken as they differ on each system
- Cursor leaves trail on screen when click and drag
