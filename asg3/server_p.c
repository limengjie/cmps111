/*
    C socket server example
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write

//#include "hashTable.c"


extern int initialize(char *file, int length, int size);
extern int insert(char *key, void *value, int length);
extern int fetch (char *key, void *value, int *length);
extern int delete(char *key);

extern int fd;

#define INIT 0
#define GET 1
#define PUT 2
#define DEL 3
#define INVALID 99

#define MAX_KEY_SIZE 100
#define MAX_DATA_SIZE 1000
#define FILLED 0xDEADD00D

char * get_str(const char * input, 
    const char * str_start, const char * str_end)
{
  const char * start, * end;

  if((start = strstr(input, str_start)) != NULL)
  {
    start += strlen(str_start);
    if((end = strstr(start, str_end)) != NULL)
    {
      char *out = malloc(end - start + 1);
      if(out != NULL)
      {
        memcpy(out, start, (end - start));
        out[end - start] = '\0';
        return out;
      }
    }
  }
  return NULL;
}



int get_int(const char * input, 
    const char * str_start, const char * str_end)
{
    const char *start, *end;
    int res;
 
    if((start = strstr(input, str_start)) != NULL)
    {
      start += strlen(str_start);
      if((end = strstr(start, str_end)) != NULL)
      {
        char *out = malloc(end - start + 1);
        if(out != NULL)
        {
          memcpy(out, start, (end - start));
          out[end - start] = '\0';
          res = myatoi(out);
          free(out);
          return res;
        }
      }
    }
    return -1;
}
 
  int myatoi(char * input) {
          int res = 0 , i = 0;
          while (input[i] != '\0') {
                  if (input[i] >= '0' && input[i] <= '9')
                          res = input[i] - '0' + res * 10;
                  i++;
          }
          return res;
  }

int get_operation(char *cmd)
{
    char str[][7] = { "INIT" , "GET" , "PUT", "DEL"};
    int n;

    char fn_file[20];
    int op;
    if (!strncmp (str[0],cmd, 4)){
        op = INIT;

    }
    else if(!strncmp (str[1],cmd, 3)) {
        op = GET;
           }
    else if(!strncmp (str[2],cmd, 3)){
        op = PUT;

    }
    else if(!strncmp (str[3],cmd, 3)){
        op = DEL;
    }
    else{
        op = INVALID;
        
    }
    return op;
}



int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];
     
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
            while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
            {
                if(strlen(client_message)) {
                   // printf("receive: %s\n", client_message);
                    int cmd = get_operation(client_message);
                   // printf("cmd: %d\n", cmd);
                    fflush(0);

                    switch (cmd) {

                        case INIT:
                            printf("call initialize\n");
                            if(fd >= 0)
                                close(fd);
                        
                           // printf("parse %s\n", client_message);
                            char * filename = get_str(client_message, "<name>", "</name>");
                            int length = get_int(client_message, "<length>", "</length>");
                            int size = get_int(client_message, "<size>", "</size>");

                            printf("filename is %s\n", filename);
                            printf("length is %d, size is %d \n", length, size);

                            fd = initialize(filename, length , size);
                            free (filename);
                            break;
                        case GET:
                            printf("call fetch\n");
                            int slot;
                            char * get_key = get_str(client_message, "<key>", "</key>");
                            char * get_value = get_str(client_message, "<value>", "</value>");
                            int get_len = strlen(get_value);
                            int * get_len_ptr = &get_len;

                            slot = fetch (get_key, (void *)get_value, get_len_ptr);

                            if (slot != -1)
                                printf("%s is found at slot[%d]\n", get_key, slot);
                            else
                                printf("%s is not found\n", get_key);

                            free(get_key);
                            free(get_value);
                            break;
                        case PUT:
                            printf("call insert\n");
                            char * put_key = get_str(client_message, "<key>", "</key>");
                            char * put_value = get_str(client_message, "<value>", "</value>");
                            int put_len = strlen(put_value);
                           
                            insert(put_key, (void *)put_value, put_len);

                            free(put_key);
                            free(put_value);
                            break;
                        case DEL:
                            printf("call delete\n");
                            char * del_key = get_str(client_message, "<key>", "</key>");

                            if(delete(del_key))
                                printf("failed to delete\n");

                            free(del_key);
                            break;
                        default:
                            printf("Invalid command\n");
                            break;
                    }
                //Send the message back to client
                write(client_sock , client_message , strlen(client_message));
                memset(client_message, 0, 2000);
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

