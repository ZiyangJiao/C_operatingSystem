//
// Created by ziyang jiao on 2019-11-04.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h> //mmcpy()
#include <unistd.h> //ftruncate()
#include <sys/types.h>
#include <sys/mman.h> //mmap()
#include <fcntl.h> //shm_open()
#include <sys/stat.h>
#include <errno.h> //perror
#include "myheader.h"

int main(int argc, char* argv[]){
    int index = 0;

    /**
     * create shared memory file descriptor using shm_open();
     * @1:path of that file
     * @2:specify that this shared memory is both readable and writable and that this call should create the region if it does not already exist
     * @3:defines user permissions to the shared region
     */
    int fd = shm_open(shared_mem_name,O_CREAT|O_RDWR,S_IRWXU);
    //set size of the shared memory
    ftruncate(fd, sizeof(shared_data));
    //map to space address
    void* mapped = mmap(NULL, sizeof(shared_data),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    //check for errors in the mmap()
    if(mapped == MAP_FAILED){
        perror("Errors in mmap()!\n");
        return -1;
    }
    //cast void* to customized structure pointer
    shared_data* shared = (shared_data*)mapped;
    if(shared == NULL){
        perror("Errors in mmap()!\n");
        return -1;
    }
    //define an array the same size as the data[] array in shared structure
    int* array = (int*)malloc(sizeof(int)*shared_mem_size);
    if(array == NULL){
        perror("Errors in malloc() for array!\n");
        return -1;
    }
    //set rand seed
    srand(3);
    //The leader creates the shared memory region, initializes the guard variables in it to 0, and waits for the follower to be created.
    shared->write_guard = 0;
    while(shared->write_guard == 0){
        //wait for follower
    }
    //write data to shared region
    for(index=0; index<shared_mem_size; ++index){
        array[index] = rand();
    }
    memcpy((void*)shared->data,(void*)array, sizeof(int)*shared_mem_size);
    //The leader writes the data to the struct, notifies the follower to start reading, and waits for the follower to finish reading.
    shared->read_guard = 1;
    while(shared->delete_guard == 0){
        //wait for follower to complete print array
    }
    //destroy the shared memory region
    shm_unlink(shared_mem_name);
    return 0;

}

