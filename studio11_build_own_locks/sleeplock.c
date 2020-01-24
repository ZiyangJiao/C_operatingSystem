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
#include <sys/syscall.h>
#include <linux/futex.h>
#include <sys/time.h>
#include <limits.h> //for INT_MAX
#include <unistd.h>

// 500 million iterations should take several seconds to run
#define ITERS 500000000
#define LOCKED 0
#define UNLOCKED 1
volatile int mylock = UNLOCKED;

void lock(volatile int *ptr) {
	int status = __atomic_sub_fetch(ptr, 1, __ATOMIC_ACQ_REL);
	while (status < 0) {
		__atomic_store_n(ptr, -1, __ATOMIC_RELEASE);
	    syscall(SYS_futex, ptr, FUTEX_WAIT, -1, NULL);	
		status = __atomic_sub_fetch(ptr, 1, __ATOMIC_ACQ_REL);
	}
	printf("CPU%d get the lock\n", sched_getcpu());
}

void unlock(volatile int *ptr) {
	int status = __atomic_add_fetch(ptr, 1, __ATOMIC_ACQ_REL);
	//exist waiting thread
	if (status != 1) {
		__atomic_store_n(ptr, 1, __ATOMIC_RELEASE);
		//wake up another thread
		syscall(SYS_futex, ptr, FUTEX_WAKE, INT_MAX);
	}
	printf("CPU:%d unlock success\n", sched_getcpu());
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