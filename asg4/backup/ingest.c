#include <stdio.h> //printf
#include <string.h>    //strlen
#include <stdlib.h>
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <unistd.h>
#include <fcntl.h>

#include "md5.h"
#include "base64.h"

int connect_server() {
	// create socket and connect to server
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
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

size_t encode(char * block, unsigned char * md, char * b64, int filed) {
	// encode md5
	char * md5;
    md5 = MDString(block);
    strcpy(md, md5);
    // md[16] = '\n';
	lseek(filed, 0, SEEK_CUR);
	write(filed, md, MD_LEN);
    // puts("encode md5");
    // int i;
    // for (i = 0; i < 16; ++i) {
    //     printf("%x", md[i]);
    // }
    // printf("\n");

    // endoe base64
    size_t blk_sz;
    char * base64;
    base64 = base64_encode(block, strlen(block), &blk_sz);
    strcpy(b64, base64);
    // printf("block size = %d\n", blk_sz);
    return blk_sz;
}

void packet(unsigned char * md, char * b64, size_t len, char * msg) {
    // packet message
    int i, j, k;
    char command[] = "INSERT";
    for (i = 0; i < strlen(command); ++i)
        msg[i] = command[i];
    msg[i++] = ',';
    for (k = 0; k < MD_LEN; ++k)
        msg[i++] = md[k];
    msg[i++] = ',';
    for (j = 0; j < strlen(b64); ++j)
        msg[i++] = b64[j];
    msg[i++] = ',';
    // printf("block: %s\n", &msg[17]); 
    char *length = (char*)&len;
    //printf("length become: %d\n", length);

    printf("block size: ");
    for(j=0; j<sizeof(size_t); j++)
    {
    	printf("%c", length[j]);
    	msg[i++] = length[j++];
	}

    msg[i++] = length;
    msg[i] = '\0';
    // printf("length = %d\n", (int)msg[strlen(msg)-1]);
    printf("message len= %d\n", i);

}

int main(int argc , char *argv[])
{
	if (argc != 3) {
		perror("Usage: ingest [original_file] [file_recipe]");
		exit(1);
	}

	int sock = connect_server();
	char message[1000] , server_reply[2000];
	// // create socket and connect to server
 //    int sock;
 //    struct sockaddr_in server;
 //    char message[1000] , server_reply[2000];
 //    // char b64_blk[400];
 //    // unsigned char msg_digest[100];
     
 //    //Create socket
 //    sock = socket(AF_INET , SOCK_STREAM , 0);
 //    if (sock == -1)
 //    {
 //        printf("Could not create socket");
 //    }
 //    puts("Socket created");
     
 //    server.sin_addr.s_addr = inet_addr("127.0.0.1");
 //    server.sin_family = AF_INET;
 //    server.sin_port = htons( 10732 );
 
 //    //Connect to remote server
 //    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
 //    {
 //        perror("connect failed. Error");
 //        return 1;
 //    }
     
 //    puts("Connected\n");
    
    int n;
   
    // open a file
    char * filename = argv[1];
    // printf("filename is %s\n", filename);
    FILE * fp;
    fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "no such file!\n");
        exit(-1);
    }

    // create a file-recipe
    char * rfilename = argv[2];
    int fileDes = open(rfilename, O_RDWR | O_CREAT | O_TRUNC );
    if (fileDes == -1) {
        fprintf(stderr, "fail to create a file!\n");
        exit(-1);
    }
    fchmod(fileDes, 0666);
 	
    //read the file to blocks
    int c, i, j, blks;
    char blocks[BLOCKS][LEN];
    while ((c = fgetc(fp)) != EOF && j < BLOCKS) {
        blocks[j][i++] = c;
        if (i == LEN - 1) {
            blocks[j++][i] = '\0';
            i = 0;
        }
    }
    blks = j;
    // for (i = 0; i < blks; ++i) {
    //     printf("%d\n %s\n", i, blocks[i]);
    // }  
    for (i = 3; i < 4; ++i) {
    	printf("%d loop:\n", i);
    	// memset(msg_digest, 0, 100);
    	// memset(b64_blk, 0, 400);
    	char b64_blk[400];
   		unsigned char msg_digest[100];
    	size_t length;
    	length  = encode(blocks[i], msg_digest, b64_blk, fileDes);
    	printf("len = %d\n", length);
        memset(message, 0, 1000);
        puts("b64:");
        printf("%s\n", b64_blk);  
        int j;  
        printf("md:\n" );
	    for (j = 0; j < 16; ++j) {
	        printf("%x", msg_digest[j]);
	    }
	    printf("\n");
        packet(msg_digest, b64_blk, length, message);
        printf("message: %s\n", message);
        if( write(sock , message , MSG_LEN) < 0)
        {
            puts("Send failed");
            return 1;
        }
    }      

    // close(fp);  
               
         
    //Receive a reply from the server
    if( (n=read(sock , server_reply , 2000)) < 0)
    {
        puts("recv failed");
        // break;
    }
     
    puts("Server reply :");
    server_reply[n] = '\0';
    puts(server_reply);
     
    close(sock);
    return 0;
}
