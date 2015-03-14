/*
    C socket server example
*/
 
#include<stdio.h>
#include<stdlib.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write

#define INIT 0
#define GET 100
#define PUT 200
#define DEL 300
#define INVALD 900


void op_init(){}
void op_get(char *key){}
void op_put(char *key, char *value){}
void op_del(char *key){}


int get_operation(char *cmd)
{
    char str[][7] = { "INIT" , "GET" , "PUT", "DEL"};
    int n;
    
    char fn_file[20];
    
    int k=0;
    while (cmd[k++] != ',')
        ;
    
    int *my_int_p = (int *) &cmd[k];
    int my_int = ntohl(*my_int_p);
    printf("GET DATA: %d \n", my_int);
    
    
    
    
    int op;
    if (!strncmp (str[0],cmd, 4)){
        op = INIT;
        op_init();

    }
    else if(!strncmp (str[1],cmd, 3)) {
        op = GET;
        op_get(cmd+3);
    }
    else if(!strncmp (str[2],cmd, 3)){
        op = PUT;
        op_put(cmd+3, cmd+5);
    }
    else if(!strncmp (str[3],cmd, 3)){
        op = DEL;
        char key[200];
        op_del(cmd+3);
    }
    else{
        op = INVALD;
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
                //get a message from client
                int op = get_operation(client_message);
                
                printf("cmd id: %d\n", op);
                
                //send a message back to client
                write(client_sock , client_message , strlen(client_message));
                memset(client_message, 0, 2000);
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

