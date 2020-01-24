//
// Created by Ziyang Jiao on 2019-11-20.
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
#include <poll.h>

#define backlog 10
#define PORT_NO 9999

int set_socket(){
    //lfd
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd < 0){
        perror("Create Socket fd error\n");
        exit(EXIT_FAILURE);
    }

    //sockopt
//    int on = 1;
//    int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(int));
//    if(ret < 0){
//        perror("Setsockopt error\n");
//        exit(EXIT_FAILURE);
//    }

    //addr attribution
    struct sockaddr_in servaddr;
    bzero((char *)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = PORT_NO;

    //bind
    int ret = bind(lfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(ret < 0){
        perror("bind error\n");
        exit(EXIT_FAILURE);
    }

    //listen
    listen(lfd, backlog + 2);

    return lfd;
}

void run_poll(int lfd){
//    /*
//     * struct pollfd {
//     *          int   fd;         /* file descriptor */
//     *          short events;     /* requested events */
//     *          short revents;    /* returned events */
//     * }
//     */
    struct pollfd clients[backlog + 2];
    int i;
    for(i=0; i<backlog + 2; i++){
        clients[i].fd = -1;
    }
    clients[0].fd = lfd;
    clients[0].events = POLLIN;
    clients[1].fd = 0; //stdin
    clients[1].events = POLLIN;
    int nready = 0;
    int boundry = 1; //current index of the rightest element in array

    while (1){
        /* int poll(struct pollfd *fds, nfds_t nfds, int timeout)' */
        /* The caller should specify the number of items in the fds array in nfds. */
        /* The field fd contains a file descriptor for an open file.  If this field is negative, then the corresponding events field is ignored and the revents field returns zero. */
        nready = poll(clients, boundry+1, -1);
        if(nready == -1){
            perror("poll error\n");
            exit(EXIT_FAILURE);
        }
        if(nready == 0){
            continue;
        }

        //check lfd
        if(clients[0].revents == POLLIN){
            struct sockaddr_in newaddr;
            memset(&newaddr, '0', sizeof(newaddr));
            size_t len = sizeof(newaddr);
            int cfd = accept(lfd, (struct sockaddr*)&newaddr, &len);
            if(cfd == -1){
                perror("Accept error!\n");
                return;
            }
            //add new cfd to connection array
            for(i = 0; i<backlog + 2; i++){ //find a position to insert new fd in client array
                if(clients[i].fd == -1){
                    clients[i].fd = cfd;
                    clients[i].events = POLLIN;
                    break;
                }
            }
            //update boundry
            if(i > boundry){
                boundry = i;
            }

            //show info
            fprintf(stdout, "IP:%s, port:%d\n", inet_ntoa(newaddr.sin_addr), ntohs(newaddr.sin_port));
            write(cfd, "Hello!client!", sizeof("Hello!client!"));

            if(--nready == 0){
                continue;
            }
        }

        //check stdin
        if(clients[1].revents == POLLIN){
            char bufstdin[250];
            int ret = read(0, bufstdin, sizeof(bufstdin));
            if(ret == -1){
                perror("stdin read error\n");
                exit(EXIT_FAILURE);
            }
            fprintf(stdout,"stdin receive:%s",bufstdin);

            if(strncmp("quit", bufstdin, 4) == 0){
                break;
            }
            //check rest
            if(--nready == 0){
                continue;
            }
        }


        //check clients
        for(i=2; i<backlog+2; i++){
            int tmpfd = clients[i].fd;
            if(tmpfd == -1){
                continue;
            }

//            if(clients[i].revents == POLLIN){
//                char buf[15];
//                int ret = read(tmpfd, buf, sizeof(buf));
//                if(ret == -1){
//                    perror("POLLIN read error\n");
//                    exit(EXIT_FAILURE);
//                }
//                if(strncmp("quit", buf, 4) == 0){
//                    fprintf(stdout,"client disconnect!\n");
//                    close(tmpfd);
//                    clients[i].fd = -1;
//                }
//                if(--nready == 0){
//                    break;
//                } else{
//                    continue;
//                }
//            }


            if(clients[i].revents == POLLIN){
                char buf[15];
                int ret = read(tmpfd, buf, sizeof(buf));
                if(ret == -1){
                    perror("read error\n");
                    exit(EXIT_FAILURE);
                }
                if(ret == 0){
                    close(tmpfd);
                    clients[i].fd = -1;
                    fprintf(stdout,"POLLRDHUP received! client disconnect!\n");
                    if(--nready == 0){
                        break;
                    } else{
                        continue;
                    }
                }


                //normal circumstance
                fprintf(stdout, "receive message:%s", buf);
                write(tmpfd, buf, sizeof(buf));
                if(strncmp("quit", buf, 4) == 0){
                    fprintf(stdout,"POLLRDHUP received! client disconnect!\n");
                    close(tmpfd);
                    clients[i].fd = -1;
                }
                if(--nready == 0){
                    break;
                }

            }
        }

    }
}

int main(int argc, char* argv[]){
    int lfd = set_socket();

    run_poll(lfd);

    close(lfd);

    return 0;
}
