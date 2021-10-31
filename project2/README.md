# V6 File System - Project2
## Objective
- openfs file_name
	- Open file system
- initfs n1 n2
	- Initialize the file system
		- n1: the file system size in number of blocks (fsize)
		- n2: the number of blocks devoted to the i-nodes (isize)
	- Set all data block free
	- Fill free array, i-list
	- Create directory
	- Write superblock to the disk
- q
	- close file descriptor
	- exit the program
## How to run
### Compliling
gcc -o mod-v6 mod-v6.c
### Execution
./mod-v6