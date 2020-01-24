//
// Created by Ziyang Jiao on 2019-11-20.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#define backlog 10

#define PORT_NO 9999
int main(int argc, char* argv[]){
    int lfd = 0;
    int cfd = 0;
    int ret = 0;
    int slength = 0;
    int i = 0;
    int opt = 0;
    size_t optlen = 0;
    socklen_t len = 0;
    struct sockaddr_in straddr;

    //get the socket file description
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd < 0){
        perror("Create Socket fd error\n");
        exit(EXIT_FAILURE);
    }

    //set socket parameter
    bzero((char *)&straddr, sizeof(straddr));
    straddr.sin_family = AF_INET;
    straddr.sin_addr.s_addr = htonl(INADDR_ANY);
    straddr.sin_port = PORT_NO;//The htons() function converts the unsigned short integer hostshort from host byte order to network byte order.

    //set port reuse
//    opt = 1;
//    optlen = sizeof(int);
//    setsockopt(lfd, SOL_SOCKET, SO_REUSEPORT, &opt, optlen);

    /*
     * The SO_KEEPALIVE option causes a packet (called a 'keepalive probe')
     * to be sent to the remote system if a long time (by default, more than
     * 2 hours) passes with no other data being sent or received. This packet
     * is designed to provoke an ACK response from the peer. This enables
     * detection of a peer which has become unreachable (e.g. powered off or
     * disconnected from the net).
     */
    /* set long live attribution */
//    opt = 1;
//    optlen = sizeof(int);
//    setsockopt(lfd, SOL_SOCKET, SO_KEEPALIVE, &opt, optlen);

    //bind
    ret = bind(lfd, (struct sockaddr*) &straddr, sizeof(straddr));
    if(ret < 0){
        perror("Bind lfd error!\n");
        exit(EXIT_FAILURE);
    }

    //listen
    /* The backlog argument defines the maximum length to which the queue of pending connections for sockfd may grow. */
    listen(lfd, backlog + 2);

    //select initialization
    int clients[backlog];
    for(i = 0; i<backlog; i++){
        clients[i] = -1;
    }
    int maxfd = lfd;
    int nready;
    fd_set tmpset;
    fd_set reset;
    FD_ZERO(&tmpset);
    FD_ZERO(&reset);
    FD_SET(0, &reset);
    FD_SET(lfd, &reset);

    while (1){
        tmpset = reset;
        nready = select(FD_SETSIZE, &tmpset, NULL, NULL, NULL);
        if(nready == 0){
            continue;
        }
        //check lfd
        if(FD_ISSET(lfd,&tmpset)){
            struct sockaddr_in newaddr;
            memset(&newaddr, '0', sizeof(newaddr));
            optlen = sizeof(newaddr);
            cfd = accept(lfd, (struct sockaddr*)&newaddr, &optlen);
            if(cfd == -1){
                perror("Accept error!\n");
                continue;
            }
            //add new cfd to connection array
            for(i = 0; i<backlog; i++){
                if(clients[i] == -1){
                    clients[i] = cfd;
                    break;
                }
            }
            //update maxfd
            if(cfd > maxfd){
                maxfd = cfd;
            }
            //add new cfd to reset
            FD_SET(cfd, &reset);
            fprintf(stdout, "IP:%s, port:%d\n", inet_ntoa(newaddr.sin_addr), ntohs(newaddr.sin_port));
            write(cfd, "Hello! New client!", sizeof("Hello! New client!"));
            //check rest
            if(--nready == 0){
                continue;
            }
        }

        //check stdin
        if(FD_ISSET(0,&tmpset)){
            char buf[25];
            read(0, buf, sizeof(buf));
            fprintf(stdout,"stdin receive:%s",buf);
            if(strncmp("quit", buf, 4) == 0){
                break;
            }
            //check rest
            if(--nready == 0){
                continue;
            }
        }

        //iterate check clients fd
        for(i = 0; i<backlog; i++){
            if(clients[i]!= -1 && FD_ISSET(clients[i], &tmpset)){
                char buf[25];
                memset(buf,'0', sizeof(buf));
                ret = read(clients[i], buf, sizeof(buf));
                //read error
                if(ret == -1){
                    perror("Read clients error!\n");
                    break;
                }
                //client disconnect
                if(ret == 0){
                    fprintf(stdout,"client disconnect!\n");
                    FD_CLR(clients[i],&reset);
                    close(clients[i]);
                    clients[i] = -1;
                    if(--nready == 0){
                        break;
                    } else{
                        continue;
                    }
                }
                //normal circumstance
                fprintf(stdout, "receive message:%s", buf);
                write(clients[i], buf, sizeof(buf));
                if(strncmp("quit", buf, 4) == 0){
                    fprintf(stdout,"client disconnect!\n");
                    FD_CLR(clients[i],&reset);
                    close(clients[i]);
                    clients[i] = -1;
                }
                if(--nready == 0){
                    break;
                }
            }

        }

    } //end while

    close(lfd);
    return 0;
}
