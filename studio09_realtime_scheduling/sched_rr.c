#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc,char *argv[]){
    //verify the number of parameter
    if(argc!=3){
        printf("Usage: ./  cpuNumber  priorityNumber\n");
        return -1;
    }
    
    //verify the assigned priority
    int priority = atoi(argv[2]);
    int low = sched_get_priority_min(SCHED_RR);
    int high = sched_get_priority_max(SCHED_RR);
    if(priority < low || priority > high ){
        printf("Invalid priority number!\n");
        return -1;
    }
    
    
    //set priority
    struct sched_param scheduler;
    scheduler.sched_priority = priority;
    if(sched_setscheduler(0,SCHED_RR,&scheduler) == -1){
         printf("set SCHED_RR failed!\n");
         return -1;
    }
    
    //set cpu
    cpu_set_t set;
    CPU_ZERO(&set);
    int cpu = atoi(argv[1]);
    CPU_SET(cpu,&set);
    sched_setaffinity(0,sizeof(cpu_set_t),&set);
    
    //run loop
    int index = 0;
    while(index != 500000000){
        index++;
        int tmp = index + 3 * 2;
    }
    
    return 0;
}
