/*
* --------------------------------------------------------------------
* Class: CS5348.001 
* Project: Project 2
* Authors: William Chang, Ksheeraja Medisetty, Surya Palaniswamy
* Language: C
* --------------------------------------------------------------------
*
* Purpose: Implementation a v6 file system
*
* Objectives:
* 
*      - openfs
*         • Open file system
*      - initfs n1 n2
*         • Initialize the file system
*            • n1:  the file system size in number of blocks(fsize)
*            • n2:  the number of blocks devoted to the i-nodes(isize)
*         • Set all data block free
*         • Fill free array, i-list
*         • Create directory
*         • Write superblock to the disk
*      - q
*         • close file descriptor
*         • exit the program
*
* How to run
* 
*   - Compliling
*     -> gcc -o mod-v6 mod-v6.c
*
*   - Execution
*     -> ./mod-v6
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h> //Signal handling functions
#include <string.h>
#include <ctype.h> //Character type functions
#include <unistd.h>
#include <fcntl.h>
#include <errno.h> //to get system error messages


#define BLOCK_SIZE 1024 //1K bytes
#define INODE_SIZE 64 //64Bytes
#define FREE_SIZE 251 // {1023 - 3*(1 Byte - char) + 3*(4 Bytes - int) + 1*(4 Bytes - time[]) } / 4
#define INODES_PER_BLOCK 16 // 1024 / 64 = 16
#define FILE_NAME_MAX_SIZE 28


char delimiter[] = " ";

//the format of super block
struct superblock
{
    unsigned int isize; //the number of blocks in inode-list, starting from block 2 to y (4 Bytes)
    unsigned int fsize; //file system size in number of blocks, check for bad block numbers (4 Bytes)
    unsigned int nfree; //The number of elements of next array are valid?? (4 Bytes)
    unsigned int free[FREE_SIZE]; //the number of free blocks(block bitmap) 記錄的是使用與未使用的 block 號碼
    char flock; // lock during free list manipulation (1 Bytes)
    char ilock; // lock during I list manipulation (1 Bytes)
    char fmod; // super block modified flag (1 Bytes)
    unsigned short time[2]; //the last time the super-block of the file system was changed (4 Bytes)
}; // Block size is 1024 Bytes; only 1023 Bytes are used


//the format of i-node
typedef struct
{
    unsigned short flags; //2 Bytes(16Bits) 
    char nlinks; //number of links to file
    char uid; //who own this file
    char gid; //group ID of owner
    char size0; //To support a file size of 4 GB
    unsigned short size1; /* It will use triple level chaining at addr[10] for large files 
			and single chaining for small files. so a 256 * 256 * 256 * 1024 = 16 GB  */
    unsigned short addr[9]; //data block address
    unsigned short actime[2]; //time of last access
    unsigned short modtime[2]; //time of last modification
}inode; //64 Bytes in size

//Directory struct
typedef struct {
    unsigned int inode; // 4 Bytes
    unsigned char filename[FILE_NAME_MAX_SIZE]; //14 Bytes??
}dir; //32 Bytes long


//Global variables
int fd;
char *currentDirectoryName;
int numberOfInodes;
int currentDirectoryInode = 1;
struct superblock sb;

//functions declarations
void initfs(char* args);
int execCommand(char cmd[100]);
void openfs(char* filename);
void test(char* args);
void quit(void);
void create_dir();



void initfs(char* args){
    unsigned short i;

    args = strtok(NULL, delimiter);
    if(args == NULL){
        printf("-n1 parameter missing-");
        return;
    }
    sb.fsize = atoi(args);

    args = strtok(NULL, delimiter);
    if(args == NULL){
        printf("-n2 parameter missing-");
        return;
    }
    sb.isize = atoi(args);

    

    // Initializing free array to 0 to remove junk data
    for(i = 0; i < FREE_SIZE; i++ ){
        sb.free[i] = 0;
    }
    // add all blocks to the free array
    sb.nfree = 0;
    
    sb.flock = '0';
    sb.ilock = '0';
    sb.fmod = '0';
    sb.time[0] = 0;
    sb.time[1] = 1970;


    
    
}

void create_dir(){}

void openfs(char* args){
    char* filename;

    args = strtok(NULL, delimiter);
    if(args == NULL){
        printf("--parameter missing--");
        return;
    } 
    filename = args;
    //open the file
    fd = open(filename, 2);

    if(fd == -1){
        fd = 0;
        printf("Fail to open: %s\n", strerror(errno));
        return;
    }

    lseek(fd, BLOCK_SIZE, SEEK_SET);
    read(fd, &sb, BLOCK_SIZE);
    return;
}

/*******************************************************************************
 It takes the block number, data to be written, number of bytes to be written 
 and writes given data from the starting of the block.
********************************************************************************/
long writeToBlock (int blockNumber, void * buffer, int nbytes)
{
    lseek(fd, BLOCK_SIZE * blockNumber, SEEK_SET);
    return write(fd, buffer, nbytes);
}

/*******************************************************************************
 It takes the block number, Offset value, data to be written, number of bytes 
 to be written and writes given data from the offset value of the block.
********************************************************************************/
long writeToBlockOffset(int blockNumber, int offset, void * buffer, int nbytes)
{
    lseek(fd,(BLOCK_SIZE * blockNumber) + offset, SEEK_SET); // lseek to offset of the block number
    return write(fd, buffer, nbytes);
}

/*******************************************************************************
 It adds a given block number to free array of the superblock and increments nfree value.
 If nfree value reaches the free array size, we write the nfree value into the 
 first 4 bytes and then free array into next 251*4 bytes of the block free[0] 
 and then set the nfree value to 1
********************************************************************************/
void addFreeBlock(int blockNumber){
    if(superBlock.nfree == FREE_SIZE)
    {
        writeToBlock(blockNumber, &superBlock.nfree, 4);
        superBlock.nfree = 0;
        //write to the new block
        writeToBlockOffset(blockNumber, 4, superBlock.free, FREE_SIZE * 4);
    }
    superBlock.free[superBlock.nfree] = blockNumber;
    superBlock.nfree++;
}

// To check if the file system is loaded or not
int fileSystemLoaded(){
    if(fd == 0){
        printf("File system is not loaded, pleas load or initialize file system.\n");
        return 0;
    }
    return 1;
}

void quit(void){
    //printf("Number of executed commands: %d\n", cmd_count);
    close(fd);
    printf("----Quit V6 File System-----");
    exit(0);
}
void help(void){
    printf("Welcome to v6 file system.\n");
    printf("Following are supported command:\n");
    printf("\n- cmd: initfs n1 n2\n");
    printf("    n1: file size in number of blocks.\n");
    printf("    n2: the number of blocks devoted to i-nodes\n");
    printf("    e.g. initfs 200 20\n");
    printf("\n- cmd: openfs file_name\n");
    printf("    e.g. openfs test.txt\n");
    printf("\n- cmd: q\n");
    printf("    exit the v6 file system\n");
}

int execCommand(char cmd[100]){
    //int cmd_size = strlen(cmd); //the length of command
    char* cPtr = strtok(cmd, delimiter); // strtok() -> splits the command into command and parameters
    if(strcmp(cPtr,"initfs")==0)
		initfs(cPtr);
    else if(strcmp(cPtr,"q")==0){
        quit();
        return 0;
    }
    else if(strcmp(cPtr, "openfs") == 0){
        openfs(cPtr);
    }
    else if(strcmp(cPtr, "test")==0)
        test(cPtr);
    else if(strcmp(cPtr, "help") == 0)
        help();
    else
        printf("Command not found");
    return 1;
}

int main(int argc, char *argv[])
{
    const char* cmd[4];
    cmd[0] = "initfs";
    cmd[1] = "openfs";
    cmd[2] = "q";
    cmd[3] = "help";

    currentDirectoryName = (char*) malloc(sizeof(char*) * 100);
    strcpy(currentDirectoryName,"/");
    
    /* temp buffer to capture the extra \n */
    char tempbuffer[1];
    char* command = (char*) malloc(sizeof(char*) * 100);

    printf("---------Welcome to V6 file system------------\n");
    printf("-----Type 'help' for detail description-------\n");

    for(;;)
	{	
		printf("\n\nv6-FS %s \n>>",currentDirectoryName);
		scanf("%[^\n]*c",command);
		scanf("%c",tempbuffer);
		if(execCommand(command))
			continue;
		else
			break;
	}
	return 0;
}