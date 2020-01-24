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
     * @2:specify that this shared memory is both readable and writable
     * @3:defines user permissions to the shared region
     */
    int fd = shm_open(shared_mem_name,O_RDWR,S_IRWXU);
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
    //The follower is created, notifies the leader to start writing, and waits for the data to be written to the shared struct.
    shared->write_guard = 1;
    while(shared->read_guard == 0){
        //wait leader to complete write
    }
    //define an array the same size as the data[] array in shared structure
    int* array = (int*)malloc(sizeof(int)*shared_mem_size);
    if(array == NULL){
        perror("Errors in malloc() for array!\n");
        return -1;
    }
    /*question 4:
    //get leader array's value
    for(index=0; index<shared_mem_size; ++index){
        array[index] = shared->data[index];
    }
    //print out the local array
    for(index=0; index<shared_mem_size; ++index){
        if(index!=0 && index%8 == 0){
            printf("\n");
        }
        printf("%d",array[index]);
    }
     */
    //question 5:
    memcpy((void*)array,(void*)shared->data, sizeof(int)*shared_mem_size);
    /**
     * //or though element-wise assignment
     *  for(index=0; index<shared_mem_size; ++index){
     *     array[index] = shared->data[index];
     *  }
     */
    //notify leader that it is finished, and unlinks itself
    shared->delete_guard = 1;
    return 0;

}


