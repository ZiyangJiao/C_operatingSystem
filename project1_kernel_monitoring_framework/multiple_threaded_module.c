//multiple_threaded_module
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/moduleparam.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

#include <linux/kthread.h>

//timer interval parameter
static unsigned long log_sec = 1;
static unsigned long log_nsec = 0;
//set module parameter
module_param(log_sec,ulong,0000);
module_param(log_nsec,ulong,0000);
//create timer and interval
static struct hrtimer mytimer;
static ktime_t myinterval;
//create task info structure for four threads
static struct task_struct *task_pointer[4];

//int iter = 1;
//int cores = 0;

//timer restart function, which is invoked when timer expire every time 
static enum hrtimer_restart
mytimer_restart(struct hrtimer *mytimer){
    int cores;
    //wake up four threads one by one
    for(cores = 0; cores<4; cores++){
        int res = wake_up_process(task_pointer[cores]);
        if(res == 0){
            printk(KERN_INFO "This thread was already running! Core:%d\n",smp_processor_id());
        }
    }
    //wake_up_process(task_pointer);
    //reset timer
    hrtimer_forward_now(mytimer,myinterval);
    return HRTIMER_RESTART;
}

//kernel thread function, which prints status info and wait until is waked up by timer.
static int
mythreads(void* p){
    int iter;
    iter = 0;
    printk(KERN_INFO "mythreads is running now! it is running on %d CPU\n",smp_processor_id());

    while(!kthread_should_stop()){
        printk(KERN_INFO "It is the %d times iteration in cpu %d! The info of nvcsw:%lu! The info of nivcsw:%lu! \n",iter,smp_processor_id(),current->nvcsw,current->nivcsw);
        iter++;
        //suspend  execution
        set_current_state(TASK_INTERRUPTIBLE);
		schedule();
    }
    printk(KERN_INFO "mythread is stopping now! Core:%d\n",smp_processor_id());
    return 0;
}

/* init function - logs that initialization happened, returns success */
static int 
simple_init(void)
{
    int cores;
    printk(KERN_INFO "mytimer module initialized\n");
    //initialize timer
    hrtimer_init(&mytimer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
    //set timer callback function and interval
    mytimer.function = mytimer_restart;
    myinterval = ktime_set(log_sec,log_nsec);
    //start timer
    hrtimer_start(&mytimer,myinterval,HRTIMER_MODE_REL);
    
    //create kernel thread on each core

    for(cores = 0; cores <4; cores++){
        task_pointer[cores] = kthread_create(mythreads,NULL,"mythread_core%d",cores);
		kthread_bind(task_pointer[cores],cores);
		wake_up_process(task_pointer[cores]);
    }
    return 0;
}

/* exit function - logs that the module is being removed */
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
	//cancel timer  
    hrtimer_cancel(&mytimer);
    printk(KERN_INFO "multiple threaeded monitoring module is being removed\n");
}


module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Ziyang Jiao");
MODULE_DESCRIPTION ("Multiple Threaded Monitoring");
