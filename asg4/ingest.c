
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
    
    switch(type)
    {
        case INSERT:
            
            en_data = base64_encode(block_data, block_size, &output_len);
    
            *msg_len = CMD_LEN + output_len + DIGEST_LEN + COMMAS + BLOCK_LEN;
            char *msg = (char *) malloc(*msg_len);
            k=0;
            msg[k++] = INSERT;
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
        
        default:
            printf("Error: can not idenfy the message command.\n");
            free(msg);
            exit(-1);
            break;
    }
}




void ingest(FILE *orig_fp, FILE *recipe_fp)
{
    int bytes;
    unsigned char block_data[BLOCK_SIZE+1];
    memset(block_data, BLOCK_SIZE, 0);
    
    char client_type = 'i';
    if (rio_writen(sock, &client_type, 1) <0) {
        puts("Send client type failed");
        exit(-1);
    }
    
    while ((bytes = fread (block_data, 1, BLOCK_SIZE, orig_fp)) != 0)
    {
        MD5_CTX mdContext;
        
        MD5Init (&mdContext);
        MD5Update (&mdContext, block_data, BLOCK_SIZE);
        MD5Final (&mdContext);
        
        int i;
        for (i = 0; i < 16; i++)
            fprintf (recipe_fp, "%02x", mdContext.digest[i]);
        fprintf (recipe_fp, "\n");
        
        block_data[bytes] = '\0';
        
        
        char *msg;
        int msg_len;
        msg = generate_message(INSERT, block_data, mdContext.digest, BLOCK_SIZE, &msg_len);
        
        /*robust send n bytes to networks without buffer*/
        if (rio_writen(sock, (void*)msg, msg_len) <0) {
            puts("Send failed");
            exit(-1);
        }
        puts("Insert block request...");
        
        
        free(msg);

        
        //Receive a reply from the server
        char reply;
        int n;
        if( (n=rio_readn(sock, &reply, 1)) < 0)
        {
            puts("recv failed");
            break;
        }
        if (reply == INSERTED  )
            printf("Server reply : inserted.\n");
        else if(reply == DEDUPLICATED  )
        printf("Server reply : deduplicated.\n");
        
        //next time to read block from file
        memset(block_data, BLOCK_SIZE, 0);
    }
}

void write_orginalfile_metadata(char *filename, FILE* fp)
{
    struct stat fileStat;
    if(stat(filename,&fileStat) < 0)
    {
        printf("Error: can not read the metadata of original file.\n");
        exit(-1);
    }
    fprintf(fp, "0%o,", fileStat.st_mode & 0777);
    fprintf(fp, "%d,", fileStat.st_size);
    
    char buf[PATH_MAX + 1]; /* not sure about the "+ 1" */
    char *res = realpath(filename, buf);
    if (res)
    {
        fprintf(fp, "%s\n", buf);
    }
    else
    {
        printf("Error: can not find the path of the orignal file.\n");
        exit(-1);
    }
}



void usage()
{
    printf("Usage: ingest original-file file-recipe\n");
    exit(-1);
}


int  main (argc, argv)
int argc;
char *argv[];
{
    char *orig_filename, *recipe_filename;
    
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
        
        orig_filename = argv[1];
        recipe_filename = argv[2];
        
        FILE *orig_fp = fopen (orig_filename, "rb");
        if(orig_fp == NULL)
        {
            printf("Error: can not open the original file.\n");
            exit(-1);
        }
        
        FILE *recipe_fp = fopen (recipe_filename, "wb");
        if(recipe_fp == NULL)
        {
            fclose(orig_fp);
            printf("Error: can not open the recipe file.\n");
            exit(-1);
        }
        
        write_orginalfile_metadata(orig_filename, recipe_fp);
        
        ingest(orig_fp, recipe_fp);
        
        close(sock);
        fclose(orig_fp);
        fclose(recipe_fp);
    }
    return 0;
}

