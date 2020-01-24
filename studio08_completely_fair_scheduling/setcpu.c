#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.c>

int main (int argc,char *argv[]){
    if(argc!=2){
        printf("Usage: ./ cpuNumber\n");
        return -1;
    }
    cpu_set_t set;
    CPU_ZERO(&set);
    int cpu = atoi(argv[1]);
    CPU_SET(cpu,&set);
    sched_setaffinity(0,sizeof(cpu_set_t),&set);
    while(1){
        
    }
    return 0;
}
