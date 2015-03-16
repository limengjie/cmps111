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
	k = FETCH_LEN;
    memcpy(msg, "FETCH", k);

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
	
	if (argc != 3) {
		perror("Usage: ingest [original_file] [file_recipe]");
		exit(1);
	}

	int sock = connect_server();
   
    

    char type = 'm';
    if( write(sock , &type, 1) < 0)
    {
            puts("Send failed");
            return 1;
    }


    // int n;
   
    // open a file-recipe
    char * recipe = argv[1];
    // printf("filename is %s\n", filename);
    FILE * fp;
    fp = fopen(recipe, "r");
    if (fp == NULL) {
        fprintf(stderr, "no such file!\n");
        exit(-1);
    }

    // create a new file
    char * newfile = argv[2];
    int fileDes = open(newfile, O_RDWR | O_CREAT | O_TRUNC );
    if (fileDes == -1) {
        fprintf(stderr, "fail to create a file!\n");
        exit(-1);
    }
    fchmod(fileDes, 0666);
 	
	


	
   
    //read file to block
    int bytes;
    unsigned char msg_digest[16];
    memset(msg_digest, 0, MD_LEN);
    while ((bytes = fread(msg_digest, 1, MD_LEN, fp)) > 0) {
    	
    	// //md5 string of the block
    	// MDString(block, msg_digest);

    	// //print md5 string
     //    int i;  
     //    printf("md:\n" );
	    // for (i = 0; i < 16; ++i) {
	    //     printf("%x", (unsigned char)msg_digest[i]);
	    // }
	    // printf("\n");
        
  		// pack command and message digest
	    int msg_sz;
		msg_sz = FETCH_LEN + MD5_LEN + COMMA;
		// printf("my message size: %d\n", msg_sz);

	    char * message = malloc(msg_sz);
        packet(msg_digest, message);

        // send message
        if( write(sock , message , msg_sz) < 0)
        {
            puts("Send failed");
            return 1;
        }


        memset(msg_digest, 0, MD_LEN);


		// char * b64_blk; 
        char * de_blk, blk[BLK_LEN + 1];
	    size_t de_sz, b64_sz;
	    int n;

	    //Receive a reply from the server
		char server_reply[119]; // 

	    if( (n = read(sock , server_reply , 119)) < 0)
	    {
	        puts("recv failed");
	        break;
	    }


	    if (strncmp(server_reply, "FOUND", 5))
	    {
	    	printf("no such block!\n");
	    	break;
	    }
	    else {
	    	// read base 64 block
	    	int k;
	    	k = FOUND_LEN + 1;
            int b64_start = k;
            b64_sz = 0;

       		//base64 data 
            while (server_reply[k++] != ',') 
            	b64_sz++;

            // decode the block
	    	de_blk = base64_decode(server_reply+b64_start, b64_sz, &de_sz);

            memcpy(blk, de_blk, de_sz);
            blk[BLK_LEN] = '\0';
            printf("after decoding, \nblock: %s \n", blk);


	    	// store the block in local file
	  		lseek(fileDes, 0, SEEK_CUR);
	  		write(fileDes, blk, strlen(blk));

	  		base64_cleanup();
            free(de_blk);			

	    } // end else

	    

    } // end while


     
    close(sock);
    return 0;
}
