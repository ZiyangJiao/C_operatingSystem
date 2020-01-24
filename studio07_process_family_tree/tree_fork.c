#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main (int argc,char *argv[]){
    if(argc != 2){
        printf("Usage: ./tree_fork N--generations\n");
    }
    if(atoi(argv[1]) > 10 || atoi(argv[1]) < 1){
        printf("Error: max generation is 10 and min generation is 1!\n");
    }
    
    int generations = atoi(argv[1]);
    int i = 0;
    pid_t pid[2];
    int level = 1;
    printf("I'm the root process.PID: %d\n",(int)getpid());
    if(generations =1){
        return 0;
    }
    while(i<2){
        if(i == 0){
            level++;
        }
        pid[i] = fork();
        
        if(pid[i] == 0){
            //child process
            printf("I'm the %d generation child! PID:%d\n",level,(int)getpid());
            if(level < generations){
                i = 0;
            }else{
                return 0;
            }
        }else if(pid[i] > 0){
            //parent process
            wait(0);
            i++;
        }else{
            //error
            printf("Error!\n");
        }
    }
    return 0;
}