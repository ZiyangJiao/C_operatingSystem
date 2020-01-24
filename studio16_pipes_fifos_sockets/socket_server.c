//
// Created by 焦紫阳 on 2019-11-18.
//
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/un.h>
#include <ctype.h>


#define SOCKNAME "./mysock"
int main(){
    int sfd = 0;
    int cfd = 0;
    //const char* SOCKNAME = "./mysock";
    struct sockaddr_un addr;
    //First, create a socket with the socket() system call. To create a local connection use the domain AF_LOCAL (or equivalently AF_UNIX), the connection type SOCK_STREAM, and protocol zero.
    sfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sfd == -1)
        printf("socket error\n");

    memset(&addr, 0, sizeof(struct sockaddr_un)); /* Clear structure */
    addr.sun_family = AF_LOCAL; /* UNIX domain address */
    /*
     * sun_path[0] is used to differentiate between a bound pathname socket address and an abstract socket address.
     * If sun_path[0] is 0, the address is abstract. Otherwise, the entire sun_path (including sun_path[0]) is a filesystem path.
     */
    /*
     * name.sun_family = AF_LOCAL;
     * strncpy (name.sun_path, filename, sizeof (name.sun_path));
     * name.sun_path[sizeof (name.sun_path) - 1] = '\0';
     */
    strncpy(addr.sun_path, SOCKNAME, sizeof(addr.sun_path) - 1);
    /*
     * struct sockaddr" is a generic definition. It's used by any socket function that requires an address.
     * "struct sockaddr_un" (a "Unix sockets" address) is a specific kind of address family.
     * The more commonly seen "struct sockaddr_in" (an "Internet socket" address) is another specific kind of address family.
     */
    if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
        printf("bind error\n");

    listen(sfd, 10);

    while(1)
    {
        cfd = accept(sfd, (struct sockaddr *) NULL, NULL);
        FILE *stream;
        int buf[1];
        stream = fdopen(cfd, "r");
        //fscanf(stream,"%*s %d",buf);
        if (fscanf(stream, "%d", buf) == 1) {
//            for(int i=0; i< ; i++){
                printf("%d\n", buf[0]);
//            }

        }

        close(cfd);
        sleep(1);
    }
    unlink(SOCKNAME);
    return 0;

}
