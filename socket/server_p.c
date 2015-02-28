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

#include "hashTable.c"

#define INIT 0
#define GET 1
#define PUT 2
#define DEL 3
#define INVALID 99

#define SUCCESS 0
#define ERROR   1

#define MAXLEN 1024

// /**
//  * readline() - read an entire line from a file descriptor until a newline.
//  * functions returns the number of characters read but not including the
//  * null character.
// **/
// int readline(int fd, char *str, int maxlen) 
// {
//   int n;           /* no. of chars */  
//   int readcount;   /* no. characters read */
//   char c;

//   for (n = 1; n < maxlen; n++) {
//     /* read 1 character at a time */
//     readcount = read(fd, &c, 1); /* store result in readcount */
//     if (readcount == 1) /* 1 char read? */
//     {
//       *str = c;      /* copy character to buffer */
//       str++;         /* increment buffer index */         
//       if (c == '\n')  is it a newline character? 
//          break;      /* then exit for loop */
//     } 
//     else if (readcount == 0) /* no character read? */
//     {
//       if (n == 1)   /* no character read? */
//         return (0); /* then return 0 */
//       else
//         break;      /* else simply exit loop */
//     } 
//     else 
//       return (-1); /* error in read() */
//   }
//   *str = 0;       /* null-terminate the buffer */
//   return (n);   /* return number of characters read */
// } /* readline() */

// /** 
//  * readnf() - reading from a file descriptor but a bit smarter 
// **/
// int readnf (int fd, char *line)
// {
//     if (readline(fd, line, MAXLEN) < 0)
//         return ERROR; 
//     return SUCCESS;
// }

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
        op = INVALID;
        
    }
    return op;
}

int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    int counter=0;
    int pid;
    

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(10732); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 

        
        if ((pid = fork()) == -1)
        {//error
            close(connfd);
            continue;
        }
        else if(pid > 0)
        {//parent
            close(connfd);

            counter++;
            printf("here parent process\n");
        }
        else if(pid == 0)
        {//child
            close(listenfd);

            char recvBuff[MAXLEN];
            char sendBuff[MAXLEN];
            time_t ticks;
            
            printf("here child process\n");
        
            memset(recvBuff, 0, sizeof(recvBuff));
            memset(sendBuff, 0, sizeof(sendBuff));
            counter++;
            
            int n;
            while( (n = read(connfd, recvBuff, sizeof(recvBuff)))  > 0 ){
            //while (readnf(connfd, recvBuff) != ERROR) {
                // int n = read(connfd, recvBuff, strlen(recvBuff));
               
                //printf("len = %u", strlen(recvBuff));


                if(strlen(recvBuff)) {
                    printf("the recved: %s\n", recvBuff);
                    int cmd = get_operation(recvBuff);
                    printf("cmd: %d\n", cmd);
                    fflush(0);

                    switch (cmd) {

                        case INIT:
                            printf("call initialize\n");
                            initialize("hashData", 0 , 1000);
                            break;
                        case GET:
                            printf("call fetch\n");
                            break;
                        case PUT:
                            printf("call insert\n");
                            break;
                        case DEL:
                            printf("call delete\n");
                            break;
                        default:
                            printf("Invalid command\n");
                            break;
                    }




                }
            
            
           

                snprintf(sendBuff, sizeof(sendBuff), "hi %d \r\n", counter);
                write(connfd, sendBuff, strlen(sendBuff));
                
              //   printf("test\n");
                memset(sendBuff, 0, sizeof(sendBuff));
                ticks = time(NULL);
                snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
                write(connfd, sendBuff, strlen(sendBuff));
             //   printf("test 1\n");
                 
             } //endWhile

            close(connfd);
            //sleep(1);
            exit(0);

        }
     }
     return 0;
}

