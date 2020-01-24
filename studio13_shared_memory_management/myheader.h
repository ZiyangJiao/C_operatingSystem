//
// Created by 焦紫阳 on 2019-11-04.
//
const char* shared_mem_name = "/shared_memory_region";
#define shared_mem_size 1000000
typedef struct shared_data{
    volatile int write_guard;
    volatile int read_guard;
    volatile int delete_guard;
    volatile int data[shared_mem_size];
}shared_data;

