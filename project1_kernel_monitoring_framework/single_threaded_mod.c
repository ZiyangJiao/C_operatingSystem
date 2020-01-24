//single_threaded_module
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
//create task info structure
static struct task_struct *task_pointer;
//record the number of ierations
int iter = 1;

//restart function, which is invoked when timer expire every time 
static enum hrtimer_restart
mytimer_restart(struct hrtimer* mytimer){
    //wake up the kernel thread
    int res = wake_up_process(task_pointer);
    //verify knernel thread status
    if(res == 0){
        printk(KERN_INFO "The thread was already running!\n");
    }
    //display assistant info
    printk(KERN_INFO "------------I am in hrtimer------------\n");
    //reset timer
    hrtimer_forward_now(mytimer,myinterval);
    return HRTIMER_RESTART;
}

//kernel thread function, which prints status info and wait until is waked up by timer.
static int
mythreads(void* p){
    printk(KERN_INFO "Function mythreads is running now! it is running on %d CPU\n",smp_processor_id());
    
    //judge if the module is being rmmoved
    while(!kthread_should_stop()){
        printk(KERN_INFO "It is the %d times iteration! The info of nvcsw:%lu! The info of nivcsw:%lu! \n",iter,current->nvcsw,current->nivcsw);
        iter++;
        //suspend  execution
        set_current_state(TASK_INTERRUPTIBLE);
		schedule();
    }
    printk(KERN_INFO "mythread is stopping now!\n");
    return 0;
}

/* init function - logs that initialization happened, returns success */
static int 
single_init(void)
{
    printk(KERN_INFO "single threaeded module initialized\n");
    //initialize timer
    hrtimer_init(&mytimer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
    //set timer callback function and interval
    mytimer.function = mytimer_restart;
    myinterval = ktime_set(log_sec,log_nsec);
    //start timer
    hrtimer_start(&mytimer,myinterval,HRTIMER_MODE_REL);
    //create kernel thread
    task_pointer = kthread_run(mythreads,NULL,"mythread");
    return 0;
}

/* exit function - logs that the module is being removed */
static void 
single_exit(void)
{
    //stop kernel thread
    if(!IS_ERR(task_pointer)){
    	kthread_stop(task_pointer);
        printk(KERN_INFO "mythread is stopped successfully!\n");
    }
	//cancel timer
    hrtimer_cancel(&mytimer);
    printk(KERN_INFO "single threaeded monitoring module is being removed\n");
}

module_init(single_init);
module_exit(single_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Ziyang Jiao");
MODULE_DESCRIPTION ("Single Threaded Monitoring");
