//
// Created by Ziyang Jiao on 2019-11-21.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define backlog 10
#define PORT_NO (9999)

int set_socket(){
    //lfd
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd < 0){
        perror("Create Socket fd error\n");
        exit(EXIT_FAILURE);
    }

    //sockopt
    int on = 1;
    int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(int));
    if(ret < 0){
        perror("Setsockopt error\n");
        exit(EXIT_FAILURE);
    }

    //addr attribution
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htonl(PORT_NO);

    //bind
    ret = bind(lfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(ret < 0){
        perror("bind error\n");
        exit(EXIT_FAILURE);
    }

    //listen
    listen(lfd, backlog + 2);

    return lfd;
}

void run_epoll(int lfd){
    //create epoll set
    int epollfd = epoll_create(backlog+2);
    if(epollfd < 0){
        perror("epoll_create error\n");
        exit(EXIT_FAILURE);
    }

    //add lfd
    struct epoll_event ev;
    ev.data.fd = lfd;
    ev.events = EPOLLIN;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, lfd, &ev) == -1){
        perror("epoll add lfd error\n");
        exit(EXIT_FAILURE);
    }

    //add server stdin
    struct epoll_event server_stdin;
    server_stdin.data.fd = 0;
    server_stdin.events = EPOLLIN;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, 0, &server_stdin) == -1){
        perror("epoll add server_stdin error\n");
        exit(EXIT_FAILURE);
    }

    //create clients array
    struct epoll_event clients[backlog + 2];

    while (1){
        int nready = 0;
        /* The memory area pointed to by events will contain the events that will be available for the caller. */
        nready = epoll_wait(epollfd, clients, backlog + 2, -1);
        if(nready == -1){
            perror("epoll wait error\n");
            exit(EXIT_FAILURE);
        }

        int i = 0;
        for(i = 0; i<nready; i++){
            //check lfd
            if(clients[i].data.fd == lfd){
                struct sockaddr_in newaddr;
                memset(&newaddr, 0, sizeof(newaddr));
                int cfd = accept(lfd, (struct sockaddr*)&newaddr, sizeof(newaddr));
                if(cfd == -1){
                    perror("Accept error!\n");
                    return;
                }

                //add new connection to epollfd
                struct epoll_event ev;
                ev.data.fd = cfd;
                ev.events = EPOLLIN;
                if(epoll_ctl(epollfd, EPOLL_CTL_ADD, cfd, &ev) == -1){
                    perror("add new connection to epollfd error\n");
                    exit(EXIT_FAILURE);
                }
                //show info
                fprintf(stdout, "IP:%s, port:%d\n", inet_ntoa(newaddr.sin_addr), ntohs(newaddr.sin_port));

            } else if(clients[i].data.fd == 0){ //server stdin
                    char buf[250];
                    int ret = read(1, buf, sizeof(buf));
                    if(ret == -1){
                        perror("stdin read error\n");
                        exit(EXIT_FAILURE);
                    }
                    fprintf(stdout,"stdin receive:%s",buf);

                    if(strncmp('quit', buf, 4) == 0){//quit from server
                        fprintf(stdout,"server shutdown");
                        close(epollfd);
                        return;
                    }

            } else{ //clients
                int tmpfd = clients[i].data.fd;
                char buf[250];
                int ret = read(tmpfd, buf, sizeof(buf));
                if(ret == -1){
                    perror("client message read error\n");
                    exit(EXIT_FAILURE);
                }
                if(ret ==0 || strncmp('quit', buf, 4) == 0){ //client quit
                    //delete from epollfd set
                    struct epoll_event ev;
                    ev.data.fd = tmpfd;
                    if(epoll_ctl(epollfd, EPOLL_CTL_DEL, tmpfd, &ev) == -1){
                        perror("delete client from epollfd set error\n");
                        exit(EXIT_FAILURE);
                    }
                    printf("a client disconnect\n");
                    close(tmpfd);
                    continue;
                }
                //normal message
                printf("receive from client:%s\n", buf);
                write(tmpfd, buf, strlen(buf));
            }
        }


    }

    close(epollfd);
    return;
}

int main(int argc, char* argv[]){
    int lfd = set_socket();

    run_epoll(lfd);

    return 0;
}