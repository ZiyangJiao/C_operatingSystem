/* Studio10: based on hello world module file.
 * the file creates a race condition in a kernel module
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/types.h>

//create task info structure for four threads
static struct task_struct *task_pointer[4];

//Declare a global volatile int variable named shared_data
//The volatile qualifier tells the compiler that this variable will be modified outside of the current execution context, and has the effect of disabling certain optimizations.
atomic_t shared_data;

#define iters 1000000

//kernel thread function
static int
mythreads(void* p){
    int iter;
    iter = 0;
    printk(KERN_INFO "mythreads is running now! it is running on %d CPU\n",smp_processor_id());

    while(!kthread_should_stop()){
        //do something
        for( iter=0; iter<iters; iter++){        
            atomic_add(1, &shared_data);
        }
        printk(KERN_INFO "mythread complete computation now! Core:%d\n",smp_processor_id());

        //suspend  execution
        set_current_state(TASK_INTERRUPTIBLE);
		schedule();
    }
    printk(KERN_INFO "mythread is stopping now! Core:%d\n",smp_processor_id());
    return 0;
}
/* init function*/
static int 
simple_init(void)
{
    //create kernel threads
    int cores;
    atomic_set(&shared_data, 0);
    for(cores = 0 ; cores<4; cores++){
    	task_pointer[cores] = kthread_create(mythreads, NULL, "kthread_num: %d",cores);
        //pin thread to a specific core
        kthread_bind(task_pointer[cores],cores);
        wake_up_process(task_pointer[cores]);
        //printk(KERN_INFO "mythread%d start successfully!\n",cores);
    }
    return 0;
}

/* exit function*/
static void 
simple_exit(void)
{
    //stop kernel thread
    int cores;
    for(cores = 0 ; cores<4; cores++){
        if(!IS_ERR(task_pointer[cores])){
    	kthread_stop(task_pointer[cores]);
        printk(KERN_INFO "mythread%d is stopped successfully!\n",cores);
        }
    }

    printk(KERN_INFO "shared_data: %d\n", atomic_read(&shared_data));
    printk(KERN_INFO "race condition module is being removed\n");
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Jing Zhang  && Bingxin Liu && Ziyang Jiao");
MODULE_DESCRIPTION ("The file creates a race condition in a kernel module -- CSE 422 Studio10");