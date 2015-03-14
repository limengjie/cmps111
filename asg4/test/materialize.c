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
	int i, k;
    char command[] = "FETCH";
    for (i = 0; i < strlen(command); ++i)
        msg[i] = command[i];
    msg[i++] = ',';
    for (k = 0; k < MD_LEN; ++k)
        msg[i++] = md[k];
    msg[i++] = ',';
    msg[i] = '\0';
    // printf("message len= %d\n", i);
}

size_t parse(char * msg, char * b64) {
	size_t len = 0;
	char str[] = "FOUND";
	if (!strncmp(msg, str, 5)) {
		len = (size_t)msg[strlen(msg)-1];
		int i;
		for (i = 0; i < len; ++i)
			b64[i] = msg[i+5];
		b64[i] = '\0';
	}
	return len;
}

size_t decode(char * b64, size_t size, char * block) {
	// decode base64
    size_t d_size;
    block = base64_decode(b64, size, &d_size);
    printf("after decode: %s\n", block);

    return d_size;
}

int main(int argc , char *argv[])
{
	if (argc != 3) {
		perror("Usage: ingest [original_file] [file_recipe]");
		exit(1);
	}
	// create socket and connect to server
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
    char b64_blk[400], blk[400];
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
    
    int n;
   
    // open a file
    char * recipe = argv[1];
    // printf("filename is %s\n", filename);
    FILE * fp;
    fp = fopen(recipe, "r");
    if (fp == NULL) {
        fprintf(stderr, "no such file!\n");
        exit(1);
    }

    // create a file-recipe
    char * newfile = argv[2];
    int fileDes = open(newfile, O_RDWR | O_CREAT | O_TRUNC );
    if (fileDes == -1) {
        fprintf(stderr, "fail to create a file!\n");
        exit(2);
    }
    fchmod(fileDes, 0666);
 	
    //read the file to blocks
    int c, i, j, blks;
    unsigned char md5_blks[BLOCKS][MD_LEN];
    while ((c = fgetc(fp)) != EOF && j < BLOCKS) {
        md5_blks[j][i++] = c;
        if (i == MD_LEN - 1) {
            // blocks[j++][i] = '\0';
            ++j;
            i = 0;
        }
    }
    blks = j;
    // for (i = 0; i < blks; ++i) {
    //     printf("%d\n %s\n", i, blocks[i]);
    // }  
    for (i = 0; i < 5; ++i) {
    	printf("%d loop:\n", i);
    	// memset(msg_digest, 0, 100);
    	// memset(b64_blk, 0, 400);
        memset(message, 0, 1000);
        packet(msg_digest, message);
        if( write(sock , message , strlen(message)) < 0)
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
    }
    size_t blk_sz, b64_sz;
    b64_sz = parse(server_reply, b64_blk); 
    blk_sz = decode(b64_blk, b64_sz, blk);
    lseek(fileDes, 0, SEEK_CUR);
    write(fileDes, blk, blk_sz);
    // puts("Server reply :");
    // server_reply[n] = 0;
    // puts(server_reply);
     
    close(sock);
    return 0;
}
