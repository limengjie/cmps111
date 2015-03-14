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

// modify the length for transmission
int h2n_len(size_t size) {
    int len = htonl((uint32_t)size);
    // printf("from client: len = %d\n", len);
    return len;
}

size_t encode(char * block, unsigned char * md, char * b64, int filed) {
	// encode md5
	char md5[16];
    MDString(block, md5);
    
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
    base64 = base64_encode(block, LEN, &blk_sz);
    strcpy(b64, base64);
    // printf("block size = %d\n", blk_sz);

    // modify length for transmission
    // int length = h2n_len(blk_sz);
    
    return blk_sz;
}

void packet(unsigned char * md, char * b64, size_t b64_size, char *msg) {
    // packet message
    int i, j;
    
    int k;

    //command part of the message
	k = strlen("INSERT");
    memcpy(msg, "INSERT", k);

	//comma
    msg[k++] = ',';

    //md5 string
    memcpy(msg+k, md, MD5_DIGEST);
    k += MD5_DIGEST;  

    //comma
    msg[k++] = ',';

    //base64 part
    memcpy(msg+k, b64, b64_size);
    k += b64_size;

    //comma
    msg[k++] = ',';

    // base64 data size
    int length = h2n_len(b64_size);
    memcpy(msg+k, (char*)&length, sizeof(int));

 
    return ;
}

int main(int argc , char *argv[])
{
	if (argc != 3) {
		perror("Usage: ingest [original_file] [file_recipe]");
		exit(1);
	}

	int n;
	int sock = connect_server();
	char server_reply[2000];
   
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
 	
    //read file to block
    int c, i, j = 0, blks, bytes;
    // char blocks[BLOCKS][LEN];
    char block[LEN+1];
    // while ((c = fgetc(fp)) != EOF && j < BLOCKS) {
    //     blocks[j][i++] = c;
    //     if (i == LEN - 1) {
    //         blocks[j++][i] = '\0';
    //         i = 0;
    //     }
    // }
    // while ((bytes = fread(blocks[j++], 1, LEN, fp)) > 0)
    // 	;
    // blks = j;
    // for (i = 0; i < blks; ++i) {
    //     printf("%d\n %s\n", i, blocks[i]);
    // }  
    // for (i = 0; i < blks; ++i) {
    memset(block, 0, LEN+1);
    while ((bytes = fread(block, 1, LEN, fp)) > 0) {

		block[bytes] = '\0';    
    	printf("Block: \n %s\n------------------\n", block);


    	char *b64_blk;
    	size_t b64_blk_len;

    	//generate base64 data of the block
    	b64_blk = base64_encode(block, LEN, &b64_blk_len);

		//md5 string of the block
		char msg_digest[16];
    	MDString(block, msg_digest);

    	//print md5 string
        int j;  
        printf("md:\n" );
	    for (j = 0; j < 16; ++j) {
	        printf("%02x", msg_digest[j]);
	    }
	    printf("\n");

	    // add message digest to file recipe
	    lseek(fileDes, 0, SEEK_CUR);
	    write(fileDes, msg_digest, 16);

	    // pack command and 3 parameters 
	    int msg_sz;
		msg_sz = INSERT_LEN + MD5_DIGEST + SIZE_OF_INT + b64_blk_len + COMMA;

		printf("message size: %d\n", msg_sz);



	    char * message = malloc(msg_sz);
        packet(msg_digest, b64_blk, b64_blk_len, message);



        if( write(sock , message , msg_sz) < 0)
        {
            puts("Send failed");
            return 1;
        }


        memset(block, 0, LEN+1);

    }   // end while   

    // close(fp);  
               
         
    // //Receive a reply from the server
    // if( (n=read(sock , server_reply , 2000)) < 0)
    // {
    //     puts("recv failed");
    //     // break;
    // }
     
    // puts("Server reply :");
    // server_reply[n] = '\0';
    // puts(server_reply);
     
    close(sock);
    return 0;
}
