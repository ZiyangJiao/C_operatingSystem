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
int
main(int argc, char *argv[]) {
    while(1) {
        struct sockaddr_un addr;
        int sfd;
        ssize_t numRead;
        char buf[1];
        sfd = socket(AF_UNIX, SOCK_STREAM, 0); /* Create client socket */
        if (sfd == -1)
            printf("socket error\n!");
/* Construct server address, and make the connection */
        memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, SOCKNAME, sizeof(addr.sun_path) - 1);

        if (connect(sfd, (struct sockaddr *) &addr,
                    sizeof(struct sockaddr_un)) == -1)
            printf("connect error!\n");
/* Copy stdin to socket */

//
//  memset(buf, 0, sizeof(buf));
//  read(STDIN_FILENO, buf, sizeof(buf));
//  write(sfd, buf, strlen(buf)+1);
        read(STDIN_FILENO, buf, sizeof(buf));
        write(sfd, buf, sizeof(buf));

    }

}
