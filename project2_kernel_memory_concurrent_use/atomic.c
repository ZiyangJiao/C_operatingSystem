/* uses concurrent (and to the extent possible, parallel) processing to compute all the prime numbers up to a specified upper bound.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/kthread.h>
#include <asm/spinlock.h>
#include <linux/timekeeping.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/slab.h>


/*
 *parameter 1: the number of threads to be used (must be 1 or greater);
 *parameter 2: an upper bound on the range of primes to compute (must be 2 or greater).
 */
static unsigned long num_threads = 1;
static unsigned long upper_bound = 10;

//assign value from input
module_param(num_threads,ulong,0000);
module_param(upper_bound,ulong,0000);

//required global variables
int *counter; //record the number of "cross-out" for each thread
atomic_t *number; //store all numbers corresponding with users' input
int cur; //record the position of the current number that is being processed
atomic_t done; //an atomic variable that records the status of each kthread
struct task_struct **task;

//extended variables
volatile int arrive_barrier; //record the number of threads that has already arrived barrier
struct timespec *start_module; //record the module start time 
struct timespec *start_compute; //record the time that the last thread arrives barrier
struct timespec *end_compute; //record the time that the last thread finishes computation
int flag; //represent the status of barrier: 0 is start, 1 is end

//lock
DEFINE_SPINLOCK(lock); 

static void mybarrier(int bflag){
    if(bflag == 0){
        printk(KERN_INFO "thread %u is arriving barrier----the first time",current->pid);
    }else if(bflag == 1){
        printk(KERN_INFO "thread %u is arriving barrier----the second time",current->pid);
    }
    //printk(KERN_INFO "thread %u is running",current->pid);
    spin_lock(&lock);
    arrive_barrier++;
    if(bflag == 0){
        ktime_get_ts(start_compute); //update until the last thread arrives
    }else if(bflag == 1){
        ktime_get_ts(end_compute); //update until the last thread finishes
        //arrive_barrier--;
    }
    spin_unlock(&lock);
    if(bflag == 0){
        while(arrive_barrier < num_threads) {}
    }else if(bflag == 1){
        while(arrive_barrier < num_threads*2) {}
    }
    
}

static void compute(void *count){
    int current_pos = -1;
    int distance = 0;
    //(*(int *)count) = 0;
    while(true){
        //set start position for this thread
        spin_lock(&lock);
        current_pos = cur;
        cur++;
        //improve performance by filterring repeated visit
        while(atomic_read(&number[cur]) == 0 && cur < upper_bound - 1){
            cur++;
        }
        spin_unlock(&lock);
        if(cur >= upper_bound - 1) {
            return;
        }
        //compute distance to the next number
        distance = current_pos + 2;
        current_pos += distance;
        //execute sieve of Eratosthenes algorithm
        while(current_pos < upper_bound - 1){
            //ctoss-out numbers without using lock
            //spin_lock(&lock);
            //number[current_pos] = 0;
			atomic_set(&number[current_pos], 0);
            //spin_unlock(&lock);
            //update position to next number
            current_pos += distance;
            //update the number of cross-out
            (*(int *)count) = (*(int *)count) + 1;
        }
        
    }
}

static int mythread_func(void *count){
    mybarrier(0);
    compute(count);
    mybarrier(1);
    atomic_add(1,&done);
    printk(KERN_INFO "threads(pid = %u) completed\n",current->pid);
    return 0;
}


static void showinfo(void) {
    int iter = 0;
    int isprime = 0;
    //int notprime = 0;
    int crossout = 0;
    int index = 0;
    
    struct timespec arrive_time; //start_compute_time - start_module_time
    struct timespec compute_time; //end_compute_time - start_compute_time
    struct timespec module_time; //time from inserting module to finishing computing
    
    //compute arrive time
    if((*start_compute).tv_nsec > (*start_module).tv_nsec){
        arrive_time.tv_nsec = (*start_compute).tv_nsec - (*start_module).tv_nsec;
        arrive_time.tv_sec = (*start_compute).tv_sec - (*start_module).tv_sec;
    }else{
        arrive_time.tv_nsec = (*start_compute).tv_nsec - (*start_module).tv_nsec + 1000000000L;
        arrive_time.tv_sec = (*start_compute).tv_sec - (*start_module).tv_sec - 1L;
    }
    
    //compute computation time
    if((*end_compute).tv_nsec > (*start_compute).tv_nsec){
        compute_time.tv_nsec = (*end_compute).tv_nsec - (*start_compute).tv_nsec;
        compute_time.tv_sec = (*end_compute).tv_sec - (*start_compute).tv_sec;
    }else{
        compute_time.tv_nsec = (*end_compute).tv_nsec - (*start_compute).tv_nsec + 1000000000L;
        compute_time.tv_sec = (*end_compute).tv_sec - (*start_compute).tv_sec - 1L;
    }
    
    //compute total time
    if((*end_compute).tv_nsec > (*start_module).tv_nsec ){
        module_time.tv_nsec = (*end_compute).tv_nsec - (*start_module).tv_nsec;
        module_time.tv_sec = (*end_compute).tv_sec - (*start_module).tv_sec;
    }else{
        module_time.tv_nsec = (*end_compute).tv_nsec - (*start_module).tv_nsec + 1000000000L;
        module_time.tv_sec = (*end_compute).tv_sec - (*start_module).tv_sec -1L;
    }
    
    //count the number of prime
    for (index = 0; index < upper_bound + 1 -2; index++) {
				if (atomic_read(&number[index]) != 0) {
					isprime++;
				}
			}
    //count the total number of cross-out
    for(index = 0; index < num_threads; index++){
        crossout += counter[index];
    }
    
    //iterate through the array and both count and print out (to the system log) all the non-zero numbers in it (i.e., the primes) in a nicely formatted style (e.g., 8 per line).
    printk(KERN_WARNING "# Is Prime: %d and these prime are:\n",isprime);
    for(index = 0; index < (upper_bound + 1 - 2); index++){
        if(atomic_read(&number[index]) != 0){
            printk(KERN_CONT "%*d,",7,atomic_read(&number[index]));
            iter++;
            if(index != 0 && iter%8 == 0){
                printk(KERN_CONT "\n");
            }
        }
    }
    //"how many non-primes there were in the array"
    printk(KERN_WARNING "# Is Not Prime: %ld\n",upper_bound - isprime -1);
    
    //total cross-out
    printk(KERN_WARNING "Total cross-out: %d\n",crossout);
    
    //"print out (to the system log) the values of the module parameters (upper bound and number of threads)."
    printk(KERN_WARNING "upper bound: %lu AND number of threads: %lu\n",upper_bound, num_threads);
    
    //"how many times numbers were unnecessarily crossed out"
    printk(KERN_WARNING "Unnecessarily cross-out: %d\n",crossout - (int)(upper_bound - isprime -1));
    
    //printk(KERN_WARNING "Module start time: %ld . %ld\n",(*start_module).tv_nsec, (*start_module).tv_nsec);
    
    //how long was spent setting up the module (from the initialization time stamp to the time stamp taken at the first barrier) 
    printk(KERN_WARNING "Time from the initialization time stamp to the time stamp taken at the first barrier: %ld ns\n",arrive_time.tv_sec*1000000000L+ arrive_time.tv_nsec);
    //how long was spent processing primes (from the time stamp taken at the first barrier to the time stamp taken at the second barrier).
    printk(KERN_WARNING "Time from the time stamp taken at the first barrier to the time stamp taken at the second barrier: %ld ns\n",compute_time.tv_sec*1000000000L+ compute_time.tv_nsec);
    //completion time (initialization time plus prime computation time) 
    printk(KERN_WARNING "Total time for module insmod to compute completion: %ld ns\n",module_time.tv_sec*1000000000L+module_time.tv_nsec);

    //printk("%ld.%ld\n",(*start_module).tv_sec, (*start_module).tv_nsec);
    //printk("%ld.%ld\n",(*start_compute).tv_sec, (*start_compute).tv_nsec);
    //printk("%ld.%ld\n",(*end_compute).tv_sec, (*end_compute).tv_nsec);
}

static int simple_init(void){
    int index = 0;

    //initialize the time variable
    start_module = (struct timespec *) kmalloc(sizeof(struct timespec), GFP_KERNEL);
    start_compute = (struct timespec *) kmalloc(sizeof(struct timespec), GFP_KERNEL);
    end_compute = (struct timespec *) kmalloc(sizeof(struct timespec), GFP_KERNEL);
    if(IS_ERR(start_module)||IS_ERR(start_compute)||IS_ERR(end_compute)){
        printk(KERN_WARNING "The time pointers error!\n");
		return 0;
    }
    (*start_module).tv_sec=0;
    (*start_module).tv_nsec=0;
    (*start_compute).tv_sec=0;
    (*start_compute).tv_nsec=0;
    (*end_compute).tv_sec=0;
    (*end_compute).tv_nsec=0;
    //record module start time 
    //"the module's init function should record this time stamp value as the first thing it does, before doing anything else"
    ktime_get_ts(start_module);
    //ktime_get_ts(start_compute);
    //ktime_get_ts(end_compute);
    //"initializing the global time stamp variables to zero may help to simplify your barrier code since at each barrier a different time stamp variable will need to be recorded."
    //start_compute = 0;
    //end_compute = 0;
    
    //initialize
    //counter = 0;
    //number = 0;
    //task = 0;
    cur = 0;
    arrive_barrier = 0;
     
    //if error, status is done
    atomic_set(&done, num_threads);
    
    printk(KERN_INFO "START Lab2: Kernel Module Concurrent Memory Use\n");
    
    //check input
    if (num_threads < 1) {
		printk(KERN_WARNING "The number of threads should more than 1 !\n");
		return 0;
	}
    if (upper_bound < 2) {
		printk(KERN_WARNING "The upper bound of number should be larger than 2 !\n");
		return 0;
	}
		 
    //allocate memory for number array
    number = (atomic_t *) kmalloc(sizeof(atomic_t)*(upper_bound-1), GFP_KERNEL);
				if (IS_ERR(number)) {
					printk(KERN_WARNING "allocate memory for number failed!\n");
					return 0;
				}
   
    //allocate memory for counter array
    counter = (int *) kmalloc(sizeof(int)*num_threads, GFP_KERNEL);
				if (IS_ERR(counter)) {
					printk(KERN_WARNING "allocate memory for counter failed!\n");
					return 0;
				}
							
	task = (struct task_struct **) kmalloc(sizeof(struct task_struct)*num_threads, GFP_KERNEL);
	if (IS_ERR(task)) {
		printk(KERN_WARNING "allocate memory for task failed!\n");
		return 0;
	}
							
     //initialize number array
    for(index = 0; index < upper_bound -1; index++){
        //number[index] = index + 2;
		atomic_set(&number[index], index+2);
    }
    
     //initialize counter array
    for(index = 0; index < num_threads; index++){
        counter[index] = 0;
							
    }
    
    //reset status
    atomic_set(&done, 0);
    
    //create kthreads
    for (index = 0; index < num_threads; index++) {
    		task[index] = kthread_run(mythread_func, &counter[index], "kthread_%d", index);
	}
	return 0;
}
/* exit function - logs that the module is being removed */
static void  simple_exit(void)
{
    printk(KERN_WARNING "Lab2 module is being removed!\n");
    if(IS_ERR(start_module)||IS_ERR(start_compute)||IS_ERR(end_compute)){
        printk(KERN_WARNING "The time pointers error!\n");
		return;
    }
    //check computation status
    if (atomic_read(&done) != num_threads) {
		printk(KERN_WARNING "No adequate time to complete computation!\n");
        return;
	}else if(IS_ERR(number)){
        printk(KERN_WARNING "Error in number array!\n");
        return;
    }else if(IS_ERR(counter)){
        printk(KERN_WARNING "Error in counter array!\n");
        kfree(number);
		number = 0;
		return;	
    }else if(IS_ERR(task)){
        printk(KERN_WARNING "Error in task pointer array!\n");
        kfree(number);
        number = 0;
		kfree(counter);
		counter = 0;
		return;
    }else{
        //print info
        showinfo();
        //free resource
        kfree(counter);
        kfree(number);
        kfree(task);
        number = 0;
        counter = 0;
        task = 0;
        return;
    }

}

module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Ziyang Jiao");
MODULE_DESCRIPTION ("Kernel Module Concurrent Memory Use");
