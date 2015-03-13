#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <unistd.h>
#include <fcntl.h>

#include "md5.h"
#include "base64.h"
 
// #define INQUIRY 0
// #define FETCH 1
// #define INSERT 2
// #define DELETE 3
// #define INVALID 99

// #define BLOCKS 500
// #define LEN 80


// int get_operation(char *cmd)
// {
//     char str[][7] = { "INQUIRY" , "FETCH" , "INSERT", "DELETE"};
//     int n;

//     char fn_file[20];
//     int op;
//     if (!strncmp (str[0],cmd, 7)){
//         op = INQUIRY;

//     }
//     else if(!strncmp (str[1],cmd, 5)) {
//         op = FETCH;
//            }
//     else if(!strncmp (str[2],cmd, 6)) {
//         op = INSERT;

//     }
//     else if(!strncmp (str[3],cmd, 6)) {
//         op = DELETE;
//     }
//     else{
//         op = INVALID;
        
//     }
//     return op;
// }

// char * get_str(const char * input, 
//     const char * str_start, const char * str_end)
// {
//   const char * start, * end;

//   if((start = strstr(input, str_start)) != NULL)
//   {
//     start += strlen(str_start);
//     if((end = strstr(start, str_end)) != NULL)
//     {
//       char *out = malloc(end - start + 1);
//       if(out != NULL)
//       {
//         memcpy(out, start, (end - start));
//         out[end - start] = '\0';
//         return out;
//       }
//     }
//   }
//   return NULL;
// }

// int get_blks(char * command, char ** blocks) {
//     // open a file
//     char * filename = get_str(command, "<name>", "</name>");
//     // printf("filename is %s\n", filename);
//     FILE * fp;
//     fp = fopen(filename, "r");
//     if (fp == NULL) {
//         fprintf(stderr, "no such file!\n");
//         exit(1);
//     }

//     // read and divde a file into serveral blocks
//     int c, i, j, blks;
//     // char blocks[BLOCKS][LEN];
//     while ((c = fgetc(fp)) != EOF && j < BLOCKS) {
//         blocks[j][i++] = c;
//         if (i == LEN - 1) {
//             blocks[j++][i] = '\0';
//             i = 0;
//         }
//     }
//     blks = j;
//     for (i = 0; i < blks; ++i) {
//         printf("%d\n %s\n", i, blocks[i]);
//     }
//     return blks;
// }

// void ingest(char * file, char * recipe) {
//     puts("call ingest");
//     FILE * fp;
//     fp = fopen(file, "r");
//     if (fp == NULL) {
//         fprintf(stderr, "no such file!\n");
//         exit(1);
//     }
//     // creat a file-recipe 
   
//     // char rfilename[20];
//     // sprintf(rfilename, "recipe-%s", file);
//     int fileDes = open(recipe, O_RDWR | O_CREAT | O_TRUNC );
//     if (fileDes == -1) {
//         fprintf(stderr, "fail to create a file!\n");
//         exit(2);
//     }
//     fchmod(fileDes, 0666);

//     int c, i, j, blks;
//     char blocks[BLOCKS][LEN];
//     // char * output;
//     // size_t blk_sz;
//     unsigned char * msg_digest;
//     while ((c = fgetc(fp)) != EOF && j < BLOCKS) {
//         blocks[j][i++] = c;
//         if (i == LEN - 1) {
//             blocks[j][i] = '\0';
//             i = 0;
//             // output = base64_encode(blocks[j], strlen(blocks[j]), &blk_sz);
//             msg_digest = MDString(blocks[j]);
//             lseek(fileDes, 0, SEEK_CUR);
//             write(fileDes, msg_digest, strlen(msg_digest));
//             memset(msg_digest, 0, strlen(msg_digest));
//             printf("%d line: %d\n", j, strlen(msg_digest));
//             ++j;
//         }
//     }
//     blks = j;
// }

void packet(char * block, char * msg) {
    // encode md5
    unsigned char * msg_digest = MDString(block);
    // int i;
    // for (i = 0; i < 16; ++i) {
    //     printf("%x", msg_digest[i]);
    // }
    // printf("\n");

    // endoe base64
    char * output;
    size_t blk_sz;
    output = base64_encode(block, strlen(block), &blk_sz);
    // lseek(filed, 0, SEEK_CUR);
    // write(filed, output, blk_sz);
    // printf("before send:blk:%s\n", output);
    // printf("base64 encode: %s\n", output); 
    // printf("block size = %d\n", blk_sz);

    // packet message
    int i, j, k;
    char command[] = "INSERT";
    for (i = 0; i < 6; ++i)
        msg[i] = command[i];
    msg[i++] = ',';
    for (k = 0; k < MD_LEN; ++k)
        msg[i++] = msg_digest[k];
    msg[i++] = ',';
    for (j = 0; j < strlen(output); ++j)
        msg[i++] = output[j];
    msg[i++] = ',';
    // printf("block: %s\n", &msg[17]); 
    char length = (char)blk_sz;
    msg[i++] = length;
    msg[i] = '\0';
    // printf("length = %d\n", (int)msg[strlen(msg)-1]);
}

int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char command[100], message[1000] , server_reply[2000];
     
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
    //keep communicating with server
    while(1)
    {
        printf("Enter command: ");
        scanf ("%[^\n]%*c", command);
        int cmd, msg_len;
        cmd = get_operation(command);
        // printf("cmd(%d):%s\n", cmd, command);

        if (cmd == INSERT) {
            // open a file
            char * filename = get_str(command, "<name>", "</name>");
            // printf("filename is %s\n", filename);
            FILE * fp;
            fp = fopen(filename, "r");
            if (fp == NULL) {
                fprintf(stderr, "no such file!\n");
                exit(1);
            }


            // read and divde a file into serveral blocks
            // // creat a file-recipe at the mean time
            // int c, i, j, blks;
            // char blocks[BLOCKS][LEN];
            // char rfilename[20];
            // sprintf(rfilename, "recipe-%s", filename);
            // int fileDes = open(rfilename, O_RDWR | O_CREAT | O_TRUNC );
            // if (fileDes == -1) {
            //     fprintf(stderr, "fail to create a file!\n");
            //     exit(2);
            // }
            // fchmod(fileDes, 0666);
            int c, i, j, blks;
            char blocks[BLOCKS][LEN];
            while ((c = fgetc(fp)) != EOF && j < BLOCKS) {
                blocks[j][i++] = c;
                if (i == LEN - 1) {
                    blocks[j++][i] = '\0';
                    i = 0;
                }
            }
            blks = j;
            // for (i = 0; i < blks; ++i) {
            //     printf("%d\n %s\n", i, blocks[i]);
            // }  

            // packet and send message
            for (i = 0; i < 1; ++i) {
                memset(message, 0, 1000);
                packet(blocks[i], message);
                if( write(sock , message , strlen(message)) < 0)
                {
                    puts("Send failed");
                    return 1;
                }
            }      

            // call ingest
            char * recipe = "recipe.txt";
            ingest(filename, recipe);

            close(fp);  
        } //end if
         

                
        //Send some data
        if( write(sock , message , strlen(message)) < 0)
        {
            puts("Send failed");
            return 1;
        }
         
        //Receive a reply from the server
        if( (n=read(sock , server_reply , 2000)) < 0)
        {
            puts("recv failed");
            break;
        }
         
        puts("Server reply :");
        server_reply[n] = 0;
        puts(server_reply);
    }
     
    close(sock);
    return 0;
}

