/*    Simulated workload using OpenMP
 *
 * This program will create some number of seconds of work on each processor
 * on the system.
 *
 * This program requires the use of the OpenMP compiler flag, and that 
 * optimizations are turned off, to build correctly. E.g.: 
 * gcc -fopenmp workload.c -o workload
 */

#define _GNU_SOURCE
#include <stdio.h> // for printf()
#include <sched.h> // for sched_getcpu()
#include <unistd.h>

// 500 million iterations should take several seconds to run
#define ITERS 500000000
#define LOCKED 0
#define UNLOCKED 1
volatile int mylock = UNLOCKED;

void lock(volatile int *ptr) {
	while(1){
		int expected = UNLOCKED;
		int desired = LOCKED;
		if(__atomic_compare_exchange( ptr, &expected, &desired, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)){
			usleep(1);
			printf("CPU:%d get the lock!\n",sched_getcpu());
			return;
		}
	}
}

void unlock(volatile int *ptr) {
	int expected = LOCKED;
	int desired = UNLOCKED;
	if(!__atomic_compare_exchange( ptr, &expected, &desired, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)){
		printf("CPU:%d Illegally attempt to unlock\n",sched_getcpu());
		return;
	}
	printf("CPU:%d unlock success\n",sched_getcpu());
	return;

}

void critical_section( void ){
	int index = 0;
	while(index < ITERS){ index++; }
}

int main (int argc, char* argv[]){

	// Create a team of threads on each processor
	#pragma omp parallel
	{
		// Each thread executes this code block independently
		lock(&mylock);
		critical_section();
		unlock(&mylock);

		printf("CPU %d finished!\n", sched_getcpu());
	}

	return 0;
}