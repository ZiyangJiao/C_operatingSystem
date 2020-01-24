#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/memory.h>
#include <linux/mm.h>

#include <paging.h>

/*
 * Step 2:
 * Design a struct to track what physical memory has been allocated for a process.
 */
struct state{
    //The data structure must have an atomic reference counter (of type atomic_t) in it.
    atomic_t counter;
    //allocated pages
    struct page **pages;
};

/*
 * Step 4:
 * Add two new static atomic_t variables to your kernel module.
 * The first variable should be incremented by 1 every time you allocate a new struct page,
 * and the second variable should be incremented by 1 every time you free a struct page.
 * In your module's exit function, read these two values and print out their values. If your module is functioning correctly, they should be equal.
 */
atomic_t allocate_page_counter;
atomic_t free_page_counter;

/*
 * Step 5:
 * disable demand paging and enable "pre-paging" by passing a value of 0 to demand_paging when you insert the module via insmod.
 */
static unsigned int demand_paging = 1;
module_param(demand_paging, uint, 0644);

static int
do_fault(struct vm_area_struct * vma,
         unsigned long           fault_address)
{
    //Step 3:

    //calculate page offset
    unsigned long offset = (unsigned long)(fault_address - vma->vm_start) / PAGE_SIZE;
    //page frame number
    unsigned long pfn;
    //get the aligned address
    unsigned long aligned_address = PAGE_ALIGN(fault_address);
    //allocate a new page
    struct page *new_page = alloc_page(GFP_KERNEL);

    printk(KERN_INFO "paging_vma_fault() invoked: took a page fault at VA 0x%lx\n", fault_address);

    //check the return value of allocate_page()
    if(new_page == NULL){
        printk(KERN_INFO "paging_vma_fault() error: allocate physical memory fails for a page fault at VA 0x%lx\n", fault_address);
        //If memory allocation fails via alloc_page, return VM_FAULT_OOM
        return VM_FAULT_OOM;
    }
    //get the page frame number
    pfn = page_to_pfn(new_page);
    atomic_inc(&allocate_page_counter); //for step 4: update allocate_page_counter
    /*
     * remap_pfn_range(): Update the process' page tables to map the faulting virtual address to the new physical address you just allocated.
     * @parameter1: user vma to map to
     * @parameter2: target user address to start at
     * @parameter3: physical address of kernel memory
     * @parameter4: size of map area
     * @parameter5: page protection flags for this mapping
     */
    if(remap_pfn_range(vma, aligned_address, pfn, PAGE_SIZE, vma->vm_page_prot) == 0){ //success
        struct state *retrieved = (struct state*)vma->vm_private_data;
        //record the new allocated page into state
        retrieved->pages[offset] = new_page;
        //increase the counter in state struct
        //atomic_inc(&(retrieved->counter));

        //If everything succeeds, return VM_FAULT_NOPAGE
        return VM_FAULT_NOPAGE;
    }

    //If a failure occurs anywhere else, return VM_FAULT_SIGBUS
    return VM_FAULT_SIGBUS;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,11,0)
static int
paging_vma_fault(struct vm_area_struct * vma,
                 struct vm_fault       * vmf)
{
    unsigned long fault_address = (unsigned long)vmf->virtual_address
#else
static int
paging_vma_fault(struct vm_fault * vmf)

{
    struct vm_area_struct * vma = vmf->vma;
    unsigned long fault_address = (unsigned long)vmf->address;
#endif

    return do_fault(vma, fault_address);
}

/*
 * Step 2:
 * Update open() and close() function.
 * when a new vma is created as a copy of an existing vma, the kernel invokes the open callback, and when a vma is being deleted, the kernel invokes the close callback.
 * The open callback should simply retrieve a pointer to our new data structure via vma->vm_private_data and increment the reference count of its atomic variable.
 * The close callback should retrieve your data structure, decrement its reference count, and if it becomes zero, free the dynamically allocated memory.
 */
static void
paging_vma_open(struct vm_area_struct * vma)
{
    //retrieve state struct
    struct state *retrieved = (struct state*)vma->vm_private_data;
    //increment the reference count of its atomic variable
    atomic_add(1,&(retrieved->counter));
    printk(KERN_INFO "paging_vma_open() invoked\n");
}

/* For step 4: free pages
 * Input: vma pointer
 * Output: the number of pages allocated for the vma
 */
static unsigned long
get_pages_count(struct vm_area_struct * vma){
    return (vma->vm_end - vma->vm_start)%PAGE_SIZE != 0 ? (vma->vm_end - vma->vm_start)/PAGE_SIZE + 1L : (vma->vm_end - vma->vm_start)/PAGE_SIZE;
}
static void
paging_vma_close(struct vm_area_struct * vma)
{
    int i = 0;
    //struct page *tmp;
    //get the count of pages allocated for the vma
    unsigned long pg_count = get_pages_count(vma);
    //retrieve state struct
    struct state *retrieved = (struct state*)vma->vm_private_data;
    //decrement the reference count of its atomic variable
    //----test----//printk(KERN_INFO "free_page_counter: %d\n",atomic_read(&(retrieved->counter)));
    atomic_dec(&(retrieved->counter));

    //step 4:
    //if counter is zero, then free memory
    //----test----//printk(KERN_INFO "free_page_counter: %d\n",atomic_read(&(retrieved->counter)));
    if(atomic_read(&(retrieved->counter)) == 0){
        i = 0;
        //----test----//printk(KERN_INFO "before while\n");
        while(i<pg_count){
            //----test----// printk(KERN_INFO"in while\n");
            if(retrieved->pages[i]){
                //----test----//printk(KERN_INFO"in if\n");
                //pfn_to_page(): returns the page associated with a page frame number
                //tmp = pfn_to_page(retrieved->pages[i]);
                __free_page(retrieved->pages[i]);
                //update free_page_counter
                atomic_inc(&(free_page_counter));
            }
            i++;
        }
        //Free our tracker structure and any other dynamically allocated memory
        kfree(retrieved->pages);
        kfree(retrieved);
    }

    printk(KERN_INFO "paging_vma_close() invoked\n");
}

static struct vm_operations_struct
paging_vma_ops =
{
    //This function is invoked by the page fault handler when a page that is not present in physical memory is accessed.
    .fault = paging_vma_fault,
    .open  = paging_vma_open,
    .close = paging_vma_close
};

/* vma is the new virtual address segment for the process */
static int
paging_mmap(struct file           * filp,
            struct vm_area_struct * vma)
{

    struct page *tmp; //in order to receive return value of alloc_page()
    int i,res; //in order to iterate(i) and receive return value of remap_pfn_range()
    unsigned long pfn; //in order to store page frame number used in remap_pfn_range()
    /*
     * Step 3(initialization part):
     * Firstly, allocate memory for state struct;
     * Secondly, compute the count of pages belonging to the vma's virtual address scope;
     * Thirdly, allocate memory for state->pages according to the count getting from previous step;
     * Fourthly, initialize state->counter filed to one;
     * Lastly, assign the state struct to vma->vm_private_data
     */
    struct state *tracer = (struct state*)kmalloc(sizeof(struct state),GFP_KERNEL);
    unsigned long pg_count = get_pages_count(vma);
    tracer->pages = (struct page**)kmalloc(sizeof(struct page*)*pg_count, GFP_KERNEL);
    atomic_set(&(tracer->counter),1);
    vma->vm_private_data = (void*)tracer;


    /* prevent Linux from mucking with our VMA (expanding it, merging it
     * with other VMAs, etc.)
     */
    vma->vm_flags |= VM_IO | VM_DONTCOPY | VM_DONTEXPAND | VM_NORESERVE
              | VM_DONTDUMP | VM_PFNMAP;

    /* setup the vma->vm_ops, so we can catch page faults */
    vma->vm_ops = &paging_vma_ops;

    printk(KERN_INFO "paging_mmap() invoked: new VMA for pid %d from VA 0x%lx to 0x%lx\n",
        current->pid, vma->vm_start, vma->vm_end);

    //Step 5: add pre-paging mode
    if(demand_paging == 0){
        //start pre-paging
        for(i = 0; i < pg_count; i++){
            tmp = alloc_page(GFP_KERNEL);
            if(tmp == NULL){
                //If memory allocation fails via alloc_page(s), return -ENOMEM
                return -ENOMEM;
            }
            //get page frame number
            pfn = page_to_pfn(tmp);
            //update the process' page tables
            res = remap_pfn_range(vma, vma->vm_start + i*PAGE_SIZE, pfn, PAGE_SIZE, vma->vm_page_prot);
            //check error
            if(res == 0){ //success
                //record the new allocated page
                tracer->pages[i] = tmp;
            }else{ //error
                return -EFAULT;
            }
            //increase the allocate_page_count variable;
            atomic_inc(&allocate_page_counter);
        }

    }


    return 0;
}

static struct file_operations
dev_ops =
{
    //call when run mmap system call
    .mmap = paging_mmap,
};

static struct miscdevice
dev_handle =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = PAGING_MODULE_NAME,
    .fops = &dev_ops,
};
/*** END device I/O **/

/*** Kernel module initialization and teardown ***/
static int
kmod_paging_init(void)
{
    int status;
    atomic_set(&allocate_page_counter, 0);
    atomic_set(&free_page_counter, 0);

    /* Create a character device to communicate with user-space via file I/O operations */
    status = misc_register(&dev_handle);
    if (status != 0) {
        printk(KERN_ERR "Failed to register misc. device for module\n");
        return status;
    }

    printk(KERN_INFO "Loaded kmod_paging module\n");

    return 0;
}

static void
kmod_paging_exit(void)
{
    /* Deregister our device file */
    misc_deregister(&dev_handle);

    //step 4: check the value of allocate_page_counter and free_page_counter
    printk(KERN_INFO "allocate_page_counter:%d  free_page_counter:%d \n", atomic_read(&allocate_page_counter), atomic_read(&free_page_counter));
    printk(KERN_INFO "Unloaded kmod_paging module\n");
}

module_init(kmod_paging_init);
module_exit(kmod_paging_exit);

/* Misc module info */
MODULE_LICENSE("GPL");
