# Simple-Linux-Kernel

Customised linux kernel with support for concurrent program execution using context switching and pid priority queue.
Customidsd basic GUI support that displays print messages from the executed C programs.
Basic I/O support such as mouse cursor and keyboard input.

![alt text](https://github.com/jerrykress/Simple-Linux-Kernel/blob/master/screenshot.png?raw=true)

## Requirements
```bash
# install qemu
sudo apt install qemu-system-arm
```

## How to Use
```bash
# build the kernel images
make build
# launch a QEMU instance
make launch-qemu
# connect to instance
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

## Issues
