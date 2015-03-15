#include <stdio.h>
#include <stdlib.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write

#include "md5.h"
#include "base64.h"
#include "func.h"
// #include "singleInstanceStore.c"


// extern int initialize(char *file, int length, int size);
// extern int insert(char *key, void *value, int length);
// extern int fetch (char *key, void *value, int *length);
// extern int delete(char *key);
// extern int probe(char *key);

extern int initialize(char *file, int size);
extern long insert(int slot, char *key, void *value, int length);
extern int inquiry(unsigned slot, char * key);
extern int fd;

int setup_server() {
    // int socket_desc , client_sock , c , read_size;
    int socket_desc;
    // struct sockaddr_in server , client;
    struct sockaddr_in server;
    // char client_message[2000];
    // char server_message[2000];
     
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

    return socket_desc;
}




void parse (char * msg, unsigned char * msg_digest, char * block, size_t * size) {
    int i, j, k;
    // *size = (size_t)msg[strlen(msg) - 1];
    // get block length
    i = get_pos(msg, ',', 3);
    *size = n2h_len((int *)&msg[i]);
    // printf("size=%d\n", *size);
    // get message digest
    i = get_pos(msg, ',', 1);
    // printf("md5's pos = %d\n", i);
    // printf("size of size_t = %d\n", sizeof(size_t));
    // the res should be 7
    for (j = 0; j < MD_LEN; ++i, ++j) 
        msg_digest[j] = msg[i];
    // get base 64 block    
    i = get_pos(msg, ',', 2);
    // printf("b64's pos = %d\n", i); // i should be 24?
    for (j = 0; j < *size; ++j, ++i)
        block[j] = msg[i];
    // block[j] = '\0';
    // printf("before decode:%s\n", block);
}

unsigned xor_fold(unsigned char * msg_digest) {
    unsigned xor_r = 0x0, i;
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
   
    
    int client_sock, socket_desc, read_size, c;
    char client_message[137];


    char server_message[2000];
    struct sockaddr_in client;

    socket_desc = setup_server();
    
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

            // initialize the hash table and append log
            char * hashTable = "hashTable";
            // printf("hash table has %u entries\n", (unsigned)HT_ENTRIES);
            initialize(hashTable, ENTRIES);

            unsigned slot;
            int i;

            char *b64_blk; 
            char *de_blk;
            unsigned char * md_ptr;


            size_t len, blk_sz;

            //Receive a message from client
            
            memset(client_message, 0, 137);
            while( (read_size = recv(client_sock , client_message , 137, 0)) > 0 )
            {
                
                // printf("receive: %s\n", client_message);
                int cmd = get_operation(client_message);
                // printf("cmd: %d\n", cmd);
                //fflush(0);

                switch (cmd) {
                    // // case INQUIRY:
                    // case FETCH:
                    //     puts("call fetch");
                    //     //get md5
                    //     for (i = 0; i < MD_LEN; ++i)
                    //         md[i] = client_message[i+6];
                    //     slot = xor_fold(md5);
                    //     // search hash table
                    //     if (!found(slot, md))
                    //         sprintf(server_message, "NOT-F");
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
                        //print md5 string
                        int k = INSERT_LEN + 1;
                        printf("server md5:");
                        for (i = 0; i < MD5_LEN; ++i)
                            printf("%x", (unsigned char)client_message[k+i]);
                        printf("\n");

                        // find the slot in hash table
                        md_ptr  = (unsigned char *)&client_message[k];
                        slot = xor_fold(md_ptr);
                        slot %= ENTRIES;
                        printf("slot = %u\n", slot);


                        // base 64 block
                        k += MD5_LEN + 1;
                        int b64_start = k;
                        int b64_len = 0;

                        //get length of base 64 block
                        while (client_message[k++] != ',') 
                            b64_len++;                       
                        
                        // decode base64
                        size_t de_size;
                        de_blk = base64_decode(client_message + b64_start, b64_len, &de_size);


                        //print the data block
                        printf("after decode: %s\n", de_blk);

                        long act_slot; 
                        // insert the entry if it is not found in hash table
                        if (!inquiry(slot, md_ptr)) {
                            // insert the entry to hash table
                            if ((act_slot = insert(slot, md_ptr, (void*)de_blk, de_size)) == -1) {
                                perror("hash table is full");
                                exit(-1);
                            }
                            printf("insert to slot %ld actually\n", act_slot);
                        }

                        // base64_cleanup();
                        // free(de_blk);

                        break; 
                    default:
                        sprintf(server_message, "513 LUL WUT?");
                        puts("unidentified cmd\n");
                        fflush(0);
                        break;
                }
                //Send the message back to client
                //write(client_sock , server_message , strlen(server_message));
                //memset(server_message, 0, 2000);
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

