#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/mm.h>

#include <asm/uaccess.h>

static uint nr_structs = 2000;
module_param(nr_structs, uint, 0644); 

static struct task_struct * kthread = NULL;

#define ARR_SIZE 8
typedef struct datatype_t {
    unsigned int array[ARR_SIZE];
} datatype;
unsigned int nr_pages;
unsigned int order;
unsigned int nr_structs_per_page;

static struct page * pages = NULL;
static struct datatype_t * dt = NULL;
static struct datatype_t * tmp = NULL;

static unsigned int
my_get_order(unsigned int value)
{
    unsigned int shifts = 0;

    if (!value)
        return 0;

    if (!(value & (value - 1)))
        value--;

    while (value > 0) {
        value >>= 1;
        shifts++;
    }

    return shifts;
}

static int
thread_fn(void * data)
{
    int i,j,n;
    printk("Hello from thread %s. nr_structs=%u\n", current->comm, nr_structs);
    //question 3
    printk("The kernel's page size: %lu\n",PAGE_SIZE);
    printk("The datatype struct's size(in bytes): %d\n",sizeof(datatype));
    printk("The number of datatype structs that will fit in a single page of memory: %lu\n",PAGE_SIZE/sizeof(datatype));
    
    //question 4
    //pages,order,per_page
    nr_structs_per_page = PAGE_SIZE/sizeof(datatype);
    nr_pages = nr_structs/nr_structs_per_page + ((nr_structs%nr_structs_per_page==0)?0:1);
    order = my_get_order(nr_pages);
    printk("nr_pages: %u\n",nr_pages);
    printk("nr_structs_per_page: %u\n",nr_structs_per_page);
    printk("order: %u\n",order);
    
    //question 5
    pages = alloc_pages(GFP_KERNEL,order);
    if(IS_ERR(pages)){
        printk("ERROR: allocate pages failed!\n");
        return -1;
    }
    for(i = 0; i<nr_pages; ++i){
        dt = (datatype*)__va(PFN_PHYS(page_to_pfn(pages+i)));
        for(j = 0; j<nr_structs_per_page; ++j){
            tmp = dt + j;
            for(n=0; n<ARR_SIZE; ++n){
                tmp->array[n] = i*nr_structs_per_page*8 + j*8 + n;
                if(j==0 && n==0){
                    //printk("%d-0-0: %d\n",i,tmp->array[n]);
                }
            }
        }
    }
    
    while (!kthread_should_stop()) {
        schedule();
    }
    
    //question 6 (run when exit)
    for(i = 0; i<nr_pages; ++i){
        dt = (datatype*)__va(PFN_PHYS(page_to_pfn(pages+i)));
        for(j = 0; j<nr_structs_per_page; ++j){
            tmp = dt + j;
            for(n=0; n<ARR_SIZE; ++n){
                if(tmp->array[n] != i*nr_structs_per_page*8 + j*8 + n){
                    printk("Error in PAGE:%d STRUCT:%d ARRAY:%d",i,j,n);
                }
            }
        }
    }
    

    printk("NO ERROR\n");
    __free_pages(pages,order);
    return 0;
}

static int
kernel_memory_init(void)
{
    printk(KERN_INFO "Loaded kernel_memory module\n");

    kthread = kthread_create(thread_fn, NULL, "k_memory");
    if (IS_ERR(kthread)) {
        printk(KERN_ERR "Failed to create kernel thread\n");
        return PTR_ERR(kthread);
    }
    
    wake_up_process(kthread);

    return 0;
}

static void 
kernel_memory_exit(void)
{
    kthread_stop(kthread);
    printk(KERN_INFO "Unloaded kernel_memory module\n");
}

module_init(kernel_memory_init);
module_exit(kernel_memory_exit);

MODULE_LICENSE ("GPL");