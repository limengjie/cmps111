#include <stdio.h> //printf
#include <string.h>    //strlen
#include <stdlib.h>
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <unistd.h>
#include <fcntl.h>

#include "md5.h"
#include "base64.h"
#include "func.h"


void packet(unsigned char * md, char * msg) {
    int k;

    // command
    k = INQUIRY_LEN;
    memcpy(msg, "INQRY", k);

    //comma
    msg[k++] = ',';

    //md5 string
    memcpy(msg+k, md, MD5_LEN);
    k += MD5_LEN; 
     
   
    // printf("actual message sz = %d\n", i);
}


int connect_server() {
	// create socket and connect to server
    int sock;
    struct sockaddr_in server;
    // char message[1000] , server_reply[2000];
    // char b64_blk[400];
    // unsigned char msg_digest[100];
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 10732 );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");

    return sock;
}

int main(int argc , char *argv[])
{
    
    if (argc != 2) {
        perror("Usage: inquiry [file_recipe]");
        exit(1);
    }


    int sock = connect_server();
   
    
    // let server distinguish different type of requests
    char type = 'm';
    if( write(sock , &type, 1) < 0)
    {
            puts("Send failed");
            return 1;
    }


    // open a file-recipe
    char * recipe = argv[1];
    // printf("filename is %s\n", filename);
    FILE * fp;
    fp = fopen(recipe, "r");
    if (fp == NULL) {
        fprintf(stderr, "no such file!\n");
        exit(-1);
    }   

    //read file to block
    int bytes;
    unsigned char msg_digest[16];
    memset(msg_digest, 0, MD_LEN);
    while (bytes = fread(msg_digest, 1, MD_LEN, fp) > 0) {

        // //print md5 string
        // int i;  
        // printf("md:\n" );
        // for (i = 0; i < 16; ++i) {
        //     printf("%x", (unsigned char)msg_digest[i]);
        // }
        // printf("\n");



        // pack command and message digest
        int msg_sz;
        msg_sz = INQUIRY_LEN + MD5_LEN + COMMA;
        // printf("my message size: %d\n", msg_sz);

        char * message = malloc(msg_sz);
        packet(msg_digest, message);

        // printf("client md5:");
        // for (i = 0; i < MD5_LEN; ++i)
        //     printf("%x", (unsigned char)message[8+i]);
        // puts(" ");


        // send message
        if( write(sock , message , msg_sz) < 0)
        {
            puts("Send failed");
            return 1;
        }
        
        memset(msg_digest, 0, MD_LEN);
        

        //Receive a reply from the server
        char server_reply[119]; // 
        int n;

        if( (n = read(sock , server_reply , 119)) < 0)
        {
            puts("recv failed");
            // break;
        }
         
        puts("Server reply :");
        server_reply[n] = '\0';
        puts(server_reply);


    }


}