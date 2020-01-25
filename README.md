### C_operatingSystem
**Studio 3: Linux System Calls**

1. Make system calls with the libc wrapper

2. Make system calls with the native Linux interface

3. Write your own system call

   <ul>
     <li>Write a C function implementation for each new syscall</li>
   	<li>Define a new system call number for each new syscall</li>
   	<li>Update the ARM architecture system call table with each new syscall</li>
   	<li>Update the total number of syscalls value that stored by the kernel</li>
   </ul>

**Studio 4: Userspace Benchmarking**

1. Benchmark programs using command line tools

   <ul><li>time</li></ul>

2. Benchmark programs using Linux's clock functions

   <ul>
     <li>Resolution: clock_getres()</li>
   	<li>time: clock_gettime()</li>
   </ul>

**Studio 5: Tracing the Linux Kernel**

1. Use the Linux system call tracer `strace`
2. Use the Linux kernel tracer (`ftrace`) via two utilities called `trace-cmd` and `kernelshark`

**Studio 6: Loadable Kernel Module**

1. Build and install kernel modules
2. Write a kernel module and use it to observe changes in a kernel variable

**Studio 7: Process Family Tree**

1. Write simple userspace programs to learn more about working with processes
2. Learn how to do simple kernel module I/O
3. Write a kernel module that explores the `task_struct` process data structure

**Studio 8: Completely Fair Scheduler (CFS)**

1. Create different workloads to influence the scheduler
2. Analyze scheduling behavior from userspace

**Studio 9: Real-Time Scheduling**

1. Write programs that run under the SCHED_RR (and optionally SCHED_FIFO) real-time scheduler
2. Use the kernel tracer to examine how this program runs

**Studio 10: Kernel Races, Atomicity, and Synchronization**

1. Create a race condition in a kernel module
2. Remove the race condition by switching to an atomic data type
3. Optionally, use kernel synchronization to resolve the race condition instead

**Studio 11: Build Your Own Locks**

1. Build a userspace spin lock, using atomic instructions
2. Build a userspace sleep lock, using atomic instructions and futexes

**Studio 12: Kernel Memory Management**

1. Allocate and deallocate memory for different numbers of objects, using the kernel-level page allocator

2. Use address translation structures to manipulate the memory allocated via the kernel-level page allocator

   *a. Convert from `struct page ` to a page frame number*

   *b. Convert from a page frame number to a physical memory address*

   *c. Convert from a physical memory address to a virtual memory address*

- `unsigned long page_to_pfn(struct page * page);`
- `unsigned long PFN_PHYS(unsigned long pfn);`
- `void * __va(unsigned long physical_address);`

**Studio 13: Shared Memory**

1. Create fixed-size shared memory regions across processes
2. Implement a basic but robust concurrency protocol to manage concurrent reads and writes
3. Clean up the shared memory regions safely
4. Benchmark shared memory speed

**Studio 14: VFS Layer**

1. Write a simple kernel module that accesses the filesystem mounted on your Raspberry Pi, via a kernel thread's process descriptor (`task_struct`).
2. Extend that kernel module to explore some of the VFS data structures, including directory entries for the current working directory and the root directory among others.
3. Extend your kernel module further to do the same for userspace task.

**Studio 15: Linux Signal Handling**

1. Generate and recieve program signals
2. Create custom signal handlers

**Studio 16: Linux Pipes, FIFOs, and Sockets**

1. Create and pass data through pipes with the `pipe()` system call
2. Create and pass data through FIFOs with the `mkfifo()` function
3. Implement a rudimentary active object pattern with processes and FIFOs
4. Use sockets to send data between processes locally within a machine
5. Use sockets to send data between processes running on different machines

**Studio 17: I/O Event Handling**

1. Use *select*, *poll*, and *epoll* to multiplex I/O events from multiple file descriptors, including sockets
2. Explore two event notification models offered by *epoll*

**Project 1: Kernel Monitoring Framework**
1. Use kernel timers to schedule recurring events (e.g., thread wakeups) in the future.
2. Use kernel threads to perform deferrable work inside the kernel.
3. Create a simple monitoring framework that periodically captures basic information from kernelspace, using kernel timers and kernel threads to manage the frequency and context of such information gathering.
4. Verify and evaluate the operation of your monitoring framework through tracing techniques.

**Project 2:Kernel Module Concurrent Memory Use**
1. Implement a kernel module that uses concurrent (and to the extent possible, parallel) processing to compute all the prime numbers up to a specified upper bound.
2. Again use basic multi-threaded synchronization and concurrency techniques, but in a more sophisticated configuration in which the threads will cooperate to complete a common task (computing prime numbers).
3. Manage kernel memory dynamically within the kernel module.

**Project 3: Memory Management and Paging**
1. implement a kernel module that leverages the `mmap()` system call to perform virtual memory mappings for a process. Processes will interact with your module via a special device file, in this case `/dev/paging`. When a process issues the `mmap()` call to this file, your module code will be invoked.
2. This module will perform two main tasks when it is invoked in this fashion: (1) allocate physical memory for the process, and (2) map a new virtual address from this process to the new physical memory that you allocated for that task.
3. This module will be configured to operate in one of two modes: (1) demand paging, or (2) pre-paging. Details of how these modes should be implemented are discussed in detail in document.
4. Study the performance differences between those two different modes, focusing on how they affect the system call execution time for the `mmap` call, and the runtime of a matrix multiplication application that uses the memory you map for it.
