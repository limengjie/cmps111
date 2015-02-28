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






int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0, i;
    char recvBuff[1024];
    struct sockaddr_in serv_addr; 

    if(argc != 2)
    {
        printf("\n Usage: %s <ip of server>\n",argv[0]);
        return 1;
    } 

    memset(recvBuff, 0,sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, 0, sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(10732); 

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    }
    
    /*char wrtbuf[100]="GET";*/
    // char wrtbuf[100]; // changed
    // strcpy(wrtbuf, argv[2]);
    // for (i = 3; i < argc; ++i) {
    //     strcat(wrtbuf, " ");
    //     strcat(wrtbuf, argv[i]);        
    // }

    int maxlen = 1024;
    char wrtbuf[1024];

    /* now read lines from the client socket */
   while(1)  /* loop - read from socket */
   {
        scanf ("%[^\n]%*c", wrtbuf);
        /* Does client want to quit? */
        if (!strncmp(wrtbuf, "QUIT", 4)) {
            printf("Received /QUIT ");
          //  break;
        }
        else {
            int count = write(sockfd, wrtbuf, strlen(wrtbuf));
            printf("write buf: %s\n", wrtbuf);
            printf("send %d chars.\n", count);              
        }        
                
               
        /* reset line */
     //   memset(wrtbuf, 0, sizeof(wrtbuf));
    
    
    // close(client_local);        


   
    
    
        if ( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
        {
            recvBuff[n] = 0;
            if(fputs(recvBuff, stdout) == EOF)
            {
                printf("\n Error : Fputs error\n");
            }
        } 

        if(n < 0)
        {
            printf("\n Read error \n");
        } 
    } /* while(readnf) */
    return 0;
}
