#include <stdio.h>
#include <stdlib.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write

#include "md5.h"
#include "base64.h"


extern int initialize(char *file, int length, int size);
extern int insert(char *key, void *value, int length);
extern int fetch (char *key, void *value, int *length);
extern int delete(char *key);
extern int probe(char *key);

extern int fd;


void parse (char * msg, unsigned char * msg_digest, char * block, size_t * size) {
    int i, j, k;
    *size = (size_t)msg[strlen(msg) - 1];
    // printf("size=%d\n", *size);
    i = 7; // size of "INQUIRY"
    for (k = 0; k < MD_LEN; ++i, ++k) 
        msg_digest[k] = msg[i];
    ++i;
    for (j = 0; j < *size; ++j, ++i)
        block[j] = msg[i];
    // block[j] = '\0';
    // printf("before decode:%s\n", block);
}

int xor_fold(char * msg_digest) {
    int xor_r = 0x0, i;
    int * p = (int *)msg_digest;
    for (i = 0; i < MD_LEN/4; ++i) 
        xor_r ^= p[i];
    // printf("xor result = %x\n", xor_r);
    return xor_r;
}

// void packet_back(char * b64, size_t size, char * msg) {
//     char str[] = "FOUND"；
//     int i, j;
//     for (i = 0; i < strlen(str); ++i)
//         msg[i] = str[i];
//     msg[i++] = ',';
//     for (j = 0; j < size; ++j)
//         msg[i] = b64[j];
//     msg[i++] = ',';
//     char length = (char)size;
//     msg[i++] = length;
//     msg[i] = '\0';
// }

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];
    char server_message[2000];
     
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
    server.sin_port = htons( 10732 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    
    
    fd = -1;
    while (1) {
        //accept connection from an incoming client
        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
        if (client_sock < 0)
        {
            perror("accept failed");
            return 1;
        }
        puts("Connection accepted");
        
        int pid;
        pid = fork();
        
        
        if (pid == 0) {//child
            
            
            close(socket_desc);
            
            
            //Receive a message from client
            
            memset(client_message, 0, 2000);
            while( (read_size = recv(client_sock , client_message , MSG_LEN, 0)) > 0 )
            {
                if(strlen(client_message)) {
                    // printf("receive: %s\n", client_message);
                    int cmd = get_operation(client_message);
                    // printf("cmd: %d\n", cmd);
                    fflush(0);

                    switch (cmd) {
                        // // case INQUIRY:
                        // case FETCH:
                        //     puts("call fetch");
                        //     //get md5
                        //     char md5[100];
                        //     int i;
                        //     for (i = 0; i < MD_LEN; ++i)
                        //         md5[i] = client_message[i+6];
                        //     int slot;
                        //     slot = xor_fold(md5);
                        //     // search hash table
                        //     if (!found(slot, md5))
                        //         sprintf(server_message, "NOT-FOUND");
                        //     else {
                        //         // get_block
                        //         char * block;
                        //         // encode base64
                        //         char * base64;
                        //         size_t blk_sz;
                        //         base64 = base64_encode(block, strlen(block), &blk_sz);
                        //         packet_back(base64, blk_sz, server_message);
                        //     }
                        //     break;
                        case INSERT:
                            puts("call insert");
                            int slot, i;
                            char blk[400], * o_blk;
                            unsigned char md[100];
                            size_t len;
                            parse(client_message, md, blk, &len);
                            slot = xor_fold(md);
                            puts("server md5:");
                            for (i = 0; i < 16; ++i)
                                printf("%x",md[i]);
                            puts(" ");
                            // decode base64
                            size_t d_size;
                            o_blk = base64_decode(blk, len, &d_size);
                            printf("after decode: %s\n", o_blk);
                            sprintf(server_message, "accepted");
                            fflush(0);
                            break; 
                        default:
                            sprintf(server_message, "513 LUL WUT?");
                            puts("unidentified cmd\n");
                            fflush(0);
                            break;
                    }
                //Send the message back to client
                write(client_sock , server_message , strlen(server_message));
                memset(server_message, 0, 2000);
                } // endIf
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
        else if (pid > 0)
        {
            close(client_sock);
            
        }
            
            
        
        
    }
    
  
     
    return 0;
}

