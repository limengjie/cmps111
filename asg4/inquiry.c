
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h> /*stat()*/
#include <limits.h> /* PATH_MAX */


#include<sys/socket.h>  //socket
#include<arpa/inet.h>   //inet_addr



#include "md5.h" 
#include "base64.h"
#include "hashstore.h"


#include "network_io_unbuffer.h"

#define INSERT '1'
#define INQUIRY '2'

#define CMD_LEN 1
#define BLOCK_SIZE 1024
#define DIGEST_LEN 16
#define BLOCK_LEN sizeof(int)
#define COMMAS  3



#define BASE64_BLK_SIZE 1368



int sock;

int network_setup()
{
    struct sockaddr_in server;
    
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
        return(-1);
    }
    
    puts("Socket created");
    
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
    
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("Error: can not create socket");
        return(-1);
    }
    
    puts("Connected\n");
    return sock;
}



char* generate_message(int type, char *block_data, char *digest, int block_size, int *msg_len)
{
    int k;
    size_t output_len;
    char *en_data;
    char *msg;
    
    switch(type)
    {
        case INSERT:
            
            en_data = base64_encode(block_data, block_size, &output_len);
    
            *msg_len = CMD_LEN + output_len + DIGEST_LEN + COMMAS + BLOCK_LEN;
            msg = (char *) malloc(*msg_len);
            k=0;
            msg[k++] = INQUIRY;
            msg[k++] = ',';
            
//            printf("Block data: \n %s\n", block_data);
//            printf("Base64 coding text size: %d\n", output_len);
//            printf("Message Length: %d\n", *msg_len);
            
       
            memcpy(msg+k, en_data, output_len);
            base64_cleanup();
            free(en_data);
            
            k += output_len;
            msg[k++] = ',';
            
            
            memcpy(msg + k, digest, DIGEST_LEN);
            k+=DIGEST_LEN;
            
            msg[k++] = ',';
            
            int tmp = htonl((uint32_t)block_size);
            
            memcpy(msg + k, &tmp, sizeof(int));
            k += sizeof(int);
            return msg;
            
            break;
        
            
            
        case INQUIRY:
            en_data = base64_encode(block_data, block_size, &output_len);
            
            *msg_len = CMD_LEN + 1 + DIGEST_LEN ;
            msg = (char *) malloc(*msg_len);
            k=0;
            msg[k++] = INQUIRY;
            msg[k++] = ',';
            
            
            memcpy(msg + k, digest, DIGEST_LEN);
            k+=DIGEST_LEN;
            
            return msg;
            
            
        default:
            printf("Error: can not idenfy the message command.\n");
            free(msg);
            exit(-1);
            break;
    }
}

int toInt(char c)
{
    if (c >= '0' && c <= '9') return      c - '0';
    if (c >= 'A' && c <= 'F') return 10 + c - 'A';
    if (c >= 'a' && c <= 'f') return 10 + c - 'a';
    return -1;
}

void gen_md5(char *input, unsigned char *output)
{
    for (size_t i = 0; i != DIGEST_LEN; ++i)
    {
        output[i] = (unsigned char)(16 * toInt(input[2*i]) + toInt(input[2*i+1]));
        //printf("%02x", output[i]);
    }
    //printf("\n");
}





void write_decode_base64_block(char *data_start, int data_len, FILE *fp, unsigned long *file_size)
{
    unsigned char *de_data;
    size_t de_data_len;
    
    de_data = base64_decode(data_start, data_len, &de_data_len);
    
//    int i;
//    for (i=0; i<de_data_len; i++) {
//        printf("%c", de_data[i]);
//    }
//    printf("\n");
//    fflush(0);
//
    int write_bytes = *file_size;
    if (*file_size > de_data_len) {
        write_bytes = de_data_len;
        *file_size -= de_data_len;
    }
    
    fwrite (de_data, 1, write_bytes, fp);
    
    
    base64_cleanup();
    free(de_data);
    
}




void materialize(FILE *recipe_fp, FILE *recreate_fp)
{
    int bytes;
    unsigned char block_data[BLOCK_SIZE+1];
    memset(block_data, BLOCK_SIZE, 0);
    
    char client_type = 'm';
    if (rio_writen(sock, &client_type, 1) <0) {
        puts("Send client type failed");
        exit(-1);
    }
    
    int protector;
    unsigned long file_size;
    char file_name[200];
    
    fscanf(recipe_fp, "%o,%ld,%s",&protector, &file_size, file_name);
    
    int i;
    char line[20];
    unsigned char block_md5[DIGEST_LEN];
    
    while (fscanf(recipe_fp, "%s",line) != EOF) {
        
        gen_md5(line, block_md5);
        printf("%s\n", line);
        
        
        char *msg;
        int msg_len;
        
        msg = generate_message(INQUIRY, NULL, block_md5, 0, &msg_len);
        
        /*robust send n bytes to networks without buffer*/
        if (rio_writen(sock, (void*)msg, msg_len) <0) {
            puts("Send failed");
            exit(-1);
        }
        puts("Inquiry block request...");
        
        free(msg);
        
        
        //Receive a reply from the server
        char reply;
        int n;
        if( (n=rio_readn(sock, &reply, 1)) < 0)
        {
            puts("recv failed");
            break;
        }
        if (reply == '1'  )//found
        {
            printf("Server reply : get block.\n");
            char *reply_msg;
            int msg_len = 3 + BASE64_BLK_SIZE + sizeof(unsigned int);
            
            reply_msg = malloc(msg_len);
            reply_msg[0] = '1';
            if(rio_readn(sock, &reply_msg[1], msg_len-1))
            {
                write_decode_base64_block(&reply_msg[2], BASE64_BLK_SIZE, recreate_fp, &file_size);
                //free(reply_msg);
            }
            
        }
        else if(reply == '0'  )//not found
        {
            
            printf("Server reply : no block error.\n");
            
        }
        else
        {
            printf("Server reply : other reason error.\n");
        }
        
        //next time to read block from file
        memset(block_data, BLOCK_SIZE, 0);
        
    }
}
    
    

void usage()
{
    printf("Usage: materialize recipe-file recreated-file\n");
    exit(-1);
}


int main (argc, argv)
int argc;
char *argv[];
{
    char *recreate_filename, *recipe_filename;
    
    if (argc != 3)
        usage();
    else
    {
        sock = -1;
        sock = network_setup();
        if(sock == -1)
        {
            printf("Error: can not setup the network.\n");
            exit(-1);
        }
        
        recipe_filename = argv[1];
        recreate_filename = argv[2];
        
        FILE *recipe_fp = fopen (recipe_filename, "rb");
        if(recipe_fp == NULL)
        {
            printf("Error: can not open the recipe file.\n");
            exit(-1);
        }
        
        FILE *recreate_fp = fopen (recreate_filename, "wb");
        if(recreate_fp == NULL)
        {
            fclose(recipe_fp);
            printf("Error: can not create the recreate file.\n");
            exit(-1);
        }
        
        materialize(recipe_fp, recreate_fp);
        
        close(sock);
        fclose(recreate_fp);
        fclose(recipe_fp);
    }
    return 0;
}

