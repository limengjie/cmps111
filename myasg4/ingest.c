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



void packet(unsigned char * md, char * b64, size_t b64_size, char *msg) {
    // packet message
    // int i, j;
    
    int k;

    //command part of the message
	k = INSERT_LEN;
    memcpy(msg, "INSERT", k);

	//comma
    msg[k++] = ',';

    //md5 string
    memcpy(msg+k, md, MD5_LEN);
    k += MD5_LEN;  

    //comma
    msg[k++] = ',';

    //base64
    memcpy(msg+k, b64, b64_size);
    k += b64_size;

    //comma
    msg[k++] = ',';

    // base64 length
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


	int sock = connect_server();
	char server_reply[2000];
 

    char type = 'i';
    if( write(sock , &type, 1) < 0)
    {
            puts("Send failed");
            return 1;
    }


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
    int i, j, bytes;
    char block[BLK_LEN+1];
    
    memset(block, 0, BLK_LEN+1);
    while ((bytes = fread(block, 1, BLK_LEN, fp)) > 0) {

		block[bytes] = '\0';    
    	printf("Block: \n %s\n", block);


    	char *b64_blk;
    	size_t b64_blk_len;

    	//generate base64 data of the block
    	b64_blk = base64_encode(block, BLK_LEN, &b64_blk_len);

        // printf("b64 len = %d\n", b64_blk_len);

		//get message digest 
		unsigned char msg_digest[16];
    	MDString(block, msg_digest);

    	// //print md5 string
     //    int j;  
     //    printf("message digest:\n" );
	    // for (j = 0; j < 16; ++j) {
	    //     printf("%x", (unsigned char)msg_digest[j]);
     //        // msg_digest[i] += '0';
	    // }
	    // printf("\n--------------------------------\n");

	    // add message digest to file recipe
	    lseek(fileDes, 0, SEEK_CUR);
	    write(fileDes, msg_digest, 16);

	    // pack command and 3 parameters 
	    int msg_sz;
		msg_sz = INSERT_LEN + MD5_LEN + SIZE_OF_INT + b64_blk_len + COMMA*3;

		// printf("message size: %d\n", msg_sz);

	    char * message = malloc(msg_sz);
        packet(msg_digest, b64_blk, b64_blk_len, message);


        // send message
        if( write(sock , message , msg_sz) < 0)
        {
            puts("Send failed");
            return 1;
        }


        memset(block, 0, BLK_LEN+1);

        int n;
         
        //Receive a reply from the server
        if( (n = read(sock , server_reply , 119)) < 0)
        {
            puts("recv failed");
            // break;
        }
         
        puts("Server reply :");
        server_reply[n] = '\0';
        puts(server_reply);

    }   // end while   

    // close(fp);  
               
    
     
    close(sock);
    return 0;
}
