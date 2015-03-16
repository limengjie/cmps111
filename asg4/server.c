/*
    C socket server example
*/
 
#include<stdio.h>
#include<stdlib.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write


#include <assert.h>


#include "base64.h"
#include "hashstore.h"


#include "network_io_unbuffer.h"


#define CMD_LEN 1
#define BLOCK_SIZE 1024
#define DIGEST_LEN 16
#define BLOCK_LEN sizeof(int)
#define COMMAS  3



#define INSERT_MESSAGE_LEN 1392

#define BASE64_BLK_SIZE 1368

#define FETCH_MESSAGE_LEN 18

#define INSERT '1'
#define FETCH '2'


unsigned char process_message(struct hash_table *table, char *msg, int client_sock)
{
    unsigned char *base64_start;
    unsigned char *md5_start;
    int *blocklen_start;
    int base64_len;
    
    unsigned char result;
    
    int k;
    switch (msg[0]) {
        case INSERT:
            k = 1;
            assert(msg[k++] == ',');
            
            base64_start = &msg[k];
            while (msg[k++] != ',') ;
            base64_len = k - 3;
            
            
            table->base64_blk_size = base64_len;
//           
//            printf("block:\n");
//            print_decode_base64_block(base64_start, base64_len);
//            printf("-----------------\n\n");
            
            md5_start = (char *)&msg[k];
            k += DIGEST_LEN;
            
            assert(msg[k++]==',');
            
            blocklen_start = (int *)&msg[k];
            int data_block_length = ntohl(*blocklen_start);
            //printf("block size: %d \n", data_block_length);
            
            result = insert(table, md5_start, base64_start, data_block_length);
            return result;
            
            break;
            
        case FETCH:
            
            k = 1;
            assert(msg[k++] == ',');
            
            md5_start = (char *)&msg[k];
            k += DIGEST_LEN;
            
//            base64_start = &msg[k];
//            while (msg[k++] != ',') ;
//            base64_len = k - 3;
//            
            table->base64_blk_size = BASE64_BLK_SIZE;
            //
            //            printf("block:\n");
            //            print_decode_base64_block(base64_start, base64_len);
            //            printf("-----------------\n\n");
            
            
//            
//            blocklen_start = (int *)&msg[k];
//            int data_block_length = ntohl(*blocklen_start);
            //printf("block size: %d \n", data_block_length);
            
//            int i;
//            printf("md5: ");
//            for (i=0; i<DIGEST_LEN; i++) {
//                printf("%02x", md5_start[i]);
//            }
//            printf("\n");
            
            char *result_msg = fetch(table, md5_start);
            
            if (result_msg != NULL) {
                int msg_len = 3 + BASE64_BLK_SIZE + sizeof(unsigned int);
                rio_writen(client_sock, result_msg, msg_len);
                free(result_msg);
                
            }
            else
            {
                char msg = '0';//NO_FOUND
                rio_writen(client_sock, &msg, 1);
            }
            return result;
            
            break;

            
            
        default:
            printf("error");
            break;
    }
}

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char *client_message;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return -1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    
    

    struct hash_table* table = initialize("hash_table.txt", HASH_TABLE_SIZE);
    puts("hash table and data store setup....");
    
    
    while (1) {
        //accept connection from an incoming client
        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
        if (client_sock < 0)
        {
            perror("accept failed");
            return -1;
        }
        puts("Connection accepted");
        
        int pid;
        pid = fork();
        
        
        if (pid == 0) {//child
            close(socket_desc);
            
            char client_type;
            if (rio_readn(client_sock , &client_type , 1) !=  1)
            {
                puts("illegal client connection.");
                exit(-1);
            }
            
            if (client_type == 'i' ) {
                
                client_message = malloc(INSERT_MESSAGE_LEN);
                
                //Receive a message from client
                memset(client_message, 0, INSERT_MESSAGE_LEN);
                
                /*read a message without buffer*/
                while( (read_size = rio_readn(client_sock , (void *)client_message , INSERT_MESSAGE_LEN)) ==  INSERT_MESSAGE_LEN)
                {
                    //get a message from client
                    unsigned char result = process_message(table, client_message, client_sock);
                    
                    //send a message back to client
                    rio_writen(client_sock , &result , 1);
                    memset(client_message, 0, INSERT_MESSAGE_LEN);
                }
                free(client_message);
            }
            else if(client_type == 'm')
            {
                client_message = malloc(FETCH_MESSAGE_LEN);
                //Receive a message from client
                memset(client_message, 0, FETCH_MESSAGE_LEN);
                
                /*read a message without buffer*/
                while( (read_size = rio_readn(client_sock , (void *)client_message , FETCH_MESSAGE_LEN)) ==  FETCH_MESSAGE_LEN)
                {
                    //get a message from client
                    unsigned char result = process_message(table, client_message, client_sock);
                    
                    //send a message back to client
                    //write(client_sock , &result , 1);
                    
                    memset(client_message, 0, FETCH_MESSAGE_LEN);
                }
                free(client_message);
            }
            else
            {
                puts("illegal client request.");
                exit(-1);
            }
            
            if(read_size == 0)
            {
                puts("Client disconnected");
                fflush(stdout);
            }
            else if(read_size == -1)
            {
                perror("recv failed");
            }
            exit(0);

        }
        else if(pid >0)
        {
            close(client_sock);
            
        }
    }
    
  
     
    return 0;
}

