//
// Created by ZiyangJiao on 2019-11-20.
//

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define HOST_IP "172.27.35.114"
//#define HOST_IP "128.252.48.34"
#define PORT_NO (9999)

int main(int argc, char* argv[]){
//    printf("L1");
//    int fd;
//    char buf[250];
//    struct sockaddr_in straddr;
//    struct hostent server;
//    int flag = 1;
//    printf("L25");
//    //create socket fd
//    fd = socket(AF_INET, SOCK_STREAM, 0);
//    if(fd < 0){
//        perror("Create Socket fd error\n");
//        exit(EXIT_FAILURE);
//    }
//
//    server = *(gethostbyname(HOST_IP));
//    printf("L34");
//    if(NULL== server.h_name){
//        perror("Server not exist!\n");
//        exit(EXIT_FAILURE);
//    }
//    printf("L39");
//    //set socket parameter
//    bzero((char*)&straddr, sizeof(straddr));
//    straddr.sin_family = AF_INET;
//    bcopy((char*)server.h_addr, (char*)straddr.sin_addr.s_addr, server.h_length);
//    straddr.sin_port = htons(PORT_NO);/* htonl, htons, ntohl, ntohs - convert values between host and network byte order */
//
//    if(connect(fd, (struct sockaddr *)&straddr, sizeof(straddr)) < 0){
//        perror("connect error!\n");
//        exit(EXIT_FAILURE);
//    }

    int flag = 1;
    int fd = 0;
    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("create socket error!\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = PORT_NO;

    if(inet_pton(AF_INET,HOST_IP,&serv_addr.sin_addr ) <=0){
        printf("adjust server address error!\n");
        return -1;
    }

    if(connect(fd,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0 ){
        //printf("connect server address error!\n");
        perror("connect server address error!\n");
        return -1;
    }


    while (flag == 1){
        char buf[15];
        memset(buf,'0', sizeof(buf));
        read(fd, buf, sizeof(buf));
        printf("From server:%s\n",buf);
        read(0, buf, sizeof(buf));
        //fprintf(stdout,"stdin receive:%s",buf);
        write(fd, buf, sizeof(buf));
        if(strncmp("quit", buf, 4) == 0){
            flag = 0;
        }
    }

    close(fd);
    printf("client quit!\n");
    return 0;


}