//
// Created by 焦紫阳 on 2019-11-18.
//
// C program to implement one side of FIFO
// This side writes first, then reads
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int main()
{
    FILE *stream;

    // FIFO file path
    char * myfifo = "./myfifo";

    // Creating the named file(FIFO)
    // mkfifo(<pathname>, <permission>)
    int res = mkfifo(myfifo, S_IRUSR | S_IWUSR);
    if(res == EEXIST){
        printf("fifo already exists!\n");
    }

    char buf[50];
    while (1)
    {
        // Open FIFO for read only

        stream = fopen(myfifo, "r");

        // Take an input buf from user.
        // 50 is maximum length
        //fgets(buf, sizeof(buf), stdin);

        // Write the input buf on FIFO
        // and close it
        //write(fd, arr2, strlen(arr2)+1); //The C library function size_t strlen(const char *str) computes the length of the string str up to, but not including the terminating null character.
        //close(fd);

        // Open FIFO for Read only
        //fd = open(myfifo, O_RDONLY);

        // Read from FIFO
        while( fgets(buf, sizeof(buf), stream) != NULL ) {
            printf("%s", buf);
        }

        // Print the read message
        //printf("reader: %s\n", buf);
        //close(fd);
        fclose(stream);
        //close(myfifo);
    }
    return 0;
}
