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
extern int fetch (unsigned slot, char *key, void *value, int *length);
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
    xor_r %= ENTRIES;
    return xor_r;
}

void packet_back(char * b64, size_t size, char * msg) {
    int k;

    // command
    k = FOUND_LEN;
    memcpy(msg, "FOUND", k);

    //comma
    msg[k++] = ',';

    //b64
    memcpy(msg+k, b64, size);
    k += size;
    // printf("size = %d\n", size);

    //comma
    msg[k++] = ',';

    int length = h2n_len(size);
    memcpy(msg+k, (char*)&length, sizeof(int));

    k += sizeof(int);

    // printf("total msg sz is %d\n", k);

}



int main(int argc , char *argv[])
{
   
    
    int client_sock, socket_desc, read_size, c;



    char server_message[119];
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


            char *client_message;
            int message_len;
            char type;


            while(recv(client_sock, &type, 1, 0)>0)
            {
                if(type == 'i')
                    message_len = 137;
                else if(type == 'm')
                    message_len = 22;
                else
                    {
                        printf("can not identify the type of client.\n");
                        exit(-1);
                    }

                break;
            }




            client_message = (char *)malloc(message_len);

            // initialize the hash table and append log
            char * hashTable = "hashTable";
            // printf("hash table has %u entries\n", (unsigned)HT_ENTRIES);
            initialize(hashTable, ENTRIES);

            unsigned slot;
            int i, k;

            char *b64_blk, block[BLK_LEN]; 
            char *de_blk;
            unsigned char * md_ptr;


            size_t blk_sz, de_size, b64_sz;

            // unsigned char md5[16]; // test


            //Receive a message from client
            
            memset(client_message, 0, message_len);

            while( (read_size = recv(client_sock , client_message , message_len, 0)) > 0 )
            {
                
                // printf("receive: %s\n", client_message);
                int cmd = get_operation(client_message);
                // printf("cmd: %d\n", cmd);
                //fflush(0);

                memset(server_message, 0, 119);
                switch (cmd) {
                    case INQUIRY:
                        puts("call inquiry");
                         //print md5 string
                        k = INQUIRY_LEN + 1;
                        

                        // find the slot in hash table
                        md_ptr  = (unsigned char *)&client_message[k];
                        slot = xor_fold(md_ptr);
                        // printf("slot = %u\n", slot);
                        // printf("server md5:");
                        // for (i = 0; i < MD5_LEN; ++i)
                        //     printf("%x", (unsigned char)client_message[k+i]);
                        // puts(" ");

                        // inquiry
                        if (inquiry(slot, md_ptr) == -1) 
                            sprintf(server_message, "NOT-FOUND");
                        else
                            sprintf(server_message, "FOUND");
                        break;
                    case FETCH:
                        puts("call fetch");
                        //get md5
                        k = FETCH_LEN + 1;
                        md_ptr  = (unsigned char *)&client_message[k];
                        // for (i = 0; i < MD5_LEN; ++i)
                        //     printf("%x", md_ptr[i]);
                        // puts(" ");
                        
                        slot = xor_fold(md_ptr);
                        // printf("got slot %u\n", slot);
                        // search hash table
                        if (inquiry(slot, md_ptr) == -1) {
                            sprintf(server_message, "NOT-FOUND");
                        }
                        else { // if blk is found, encode and send it back 
                            // get_block
                             int ht_slot = fetch (slot, md_ptr, (void*)block, &blk_sz);

                            // encode base64
                            b64_blk = base64_encode(block, BLK_LEN, &blk_sz);


                            // pack the message to send
                            packet_back(b64_blk, blk_sz, server_message);
                        } // end else

                        // free(block);
                        break;
                    case INSERT:
                        puts("call insert");
                        //print md5 string
                        k = INSERT_LEN + 1;
                        // printf("server md5:");
                        // for (i = 0; i < MD5_LEN; ++i)
                        //     printf("%x", (unsigned char)client_message[k+i]);
                        // printf("\n");

                        // find the slot in hash table
                        md_ptr  = (unsigned char *)&client_message[k];
                        slot = xor_fold(md_ptr);
                        // slot %= ENTRIES;
                        // printf("slot = %u\n", slot);


                        // base 64 block
                        k += MD5_LEN + 1;
                        int b64_start = k;
                        b64_sz = 0;

                        //get length of base 64 block
                        while (client_message[k++] != ',') 
                            b64_sz++;                       
                        
                        // decode base64
                        de_blk = base64_decode(client_message + b64_start, b64_sz, &de_size);

                        // // test
                        // MDString(de_blk, md5);  
                        // for (i = 0; i < 16; ++i)
                        //     printf("%x", md5[i]);
                        // puts("test");//////////


                        //print the data block
                        printf("after decode: %s\n", de_blk);

                        long act_slot; 
                        // insert the entry if it is not found in hash table
                        if (inquiry(slot, md_ptr) == -1) {
                            // insert the entry to hash table
                            if ((act_slot = insert(slot, md_ptr, (void*)de_blk, de_size)) == -1) {
                                perror("hash table is full");
                                exit(-1);
                            }
                            printf("insert to slot %ld actually\n", act_slot);
                        }

                        sprintf(server_message, "accepted");

                        base64_cleanup();
                        free(de_blk);

                        break; 
                    default:
                        sprintf(server_message, "513 LUL WUT?");
                        puts("unidentified cmd\n");
                        fflush(0);
                        break;
                }
                //Send the message back to client
                write(client_sock , server_message , 119);
                memset(server_message, 0, 119);
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
            free(client_message);
            exit(0);

        }
        else if (pid > 0)
        {
            close(client_sock);
            
        }
            
            
        
        
    }
    
  
     
    return 0;
}

