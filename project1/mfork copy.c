/*
To run c program
1. gcc filename.c
2. ./a.out

Write a c program whose main program forks exactly 8 children. Each child should output what its
position in the list of children and print its number. For example, the second child should print
something like “I am child number: 2”. No child should fork a process; only the main program or the
original program should fork.
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
 
int main(void) {
  for(int i = 1; i <= 8; i++){
    pid_t pid = fork();
    
    if(pid == 0){
      printf("I am child number: %d \n", i);
      exit(0);
    }
    else{
      //printf("Parent process -> PID = %d\n", getpid());
      //printf("Waiting for child process to finish...\n");
      wait(NULL);
      //printf("Child process finished.\n");
    }
  }
  return EXIT_SUCCESS;
}