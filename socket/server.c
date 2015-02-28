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

#define INIT 0
#define GET 100
#define PUT 200
#define DEL 300
#define INVALD 900


int get_operation(char *cmd)
{
    char str[][7] = { "INIT" , "GET" , "PUT", "DEL"};
    int n;

    char fn_file[20];
    int op;
    if (!strncmp (str[0],cmd, 4)){
        op = INIT;

    }
    else if(!strncmp (str[1],cmd, 3)) {
        op = GET;
           }
    else if(!strncmp (str[2],cmd, 3)){
        op = PUT;

    }
    else if(!strncmp (str[3],cmd, 3)){
        op = DEL;
    }
    else{
        op = INVALD;
        
    }
    return op;
}







int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char sendBuff[1025];
    char recvBuff[1025];
    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    memset(recvBuff, '0', sizeof(recvBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(10732); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 

        
        int n = read(connfd, recvBuff, strlen(recvBuff));
        printf("the recved: %s\n", recvBuff);
        
        
        
        int cmd = get_operation(recvBuff);
        printf("cmd: %d", cmd);
        fflush(0);
        
        
        ticks = time(NULL);
        snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
        write(connfd, sendBuff, strlen(sendBuff)); 

        close(connfd);
        sleep(1);
     }
}
