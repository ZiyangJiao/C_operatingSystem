#include <stdio.h>  //For printf()
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>


int main( int argc, char* argv[] ){
    pid_t p;
    int pipefd[2];  // pipefd[0] read, pipefd[1] write
    pipe (pipefd);
    p = fork();
    // Parent
    if (p > 0) {
        FILE *stream;
        char s[10];

        printf("[Parent] This is the parent after fork\n");
        close(pipefd[1]);
        printf("[Parent] Write end closed\n");
        stream = fdopen (pipefd[0], "r");
        while( fgets(s, 10, stream) != NULL ) {
            printf("%s", s);
        }
        // int c;
        // while ((c = fgetc (stream)) != EOF) {
        //     putchar (c);
        // }
        fclose(stream);
        close(pipefd[0]);
    }
    // Child
    else {
        FILE *stream;

        printf("[Child] This is the child after fork\n");
        close(pipefd[0]);
        printf("[Child] Read end closed\n");
        stream = fdopen (pipefd[1], "w");
        fprintf (stream, "Message 1 from Child\n");
        fprintf (stream, "Message 2 from Child\n");
        fprintf (stream, "Message 3 from Child\n");
        fclose(stream);
        close(pipefd[1]);
    }

    return 0;
}
