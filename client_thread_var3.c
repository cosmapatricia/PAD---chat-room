#include<stdio.h> 
#include<string.h>    
#include<sys/socket.h>    
#include<arpa/inet.h> 
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h> //for threading, compile with -pthread
#include <sys/types.h>
#include <sys/wait.h>

typedef struct {
        char username[512];
	char message[512];
	char password[512];
        int socket_fd;
	} client;

int main(int argc , char *argv[])
{
    int sock, *new_sock;
    struct sockaddr_in server;
    //Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Error creating socket\n");
	exit(3);
    }
    puts("Socket created\n");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    //Connect to remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) ==-1)
    {
        printf("Error connecting socket\n");
        exit(4);
    }
    puts("Connected\n");

    pid_t pid_transmit, pid_receive;
    pid_transmit=fork();
    if(pid_transmit == -1)
    {
    	printf("Error creating child process\n");
        exit(5);
    }
    if(pid_transmit == 0)
    {
        char username[512];
	char password[512];
        client *newClient=(client *)malloc(sizeof(client));
	if(newClient==NULL)
	{
		printf("Error alocating memory space\n");
		exit(6);
	}
      	newClient->socket_fd=0;   

        printf("Enter your username: ");
        fgets(username,sizeof(username),stdin);
        strtok(username,"\n");		                             //remove newline from string

	printf("Enter your password: ");
	fgets(password,sizeof(password),stdin);
	strtok(password,"\n");

	strcpy(newClient->username,username);   
        strcpy(newClient->password,password);
                        
        char message[1024];
        strcpy(message, strcat(username," has joined the chat room"));        
        strcpy(newClient->message,message);

        //Send some data
        if(send(sock, newClient, 1540, 0) == -1)
        {
              printf("Send failed\n");
              exit(7);
        }
               
        while(1)
        {
              fgets(message, sizeof(message),stdin);
              strtok(message,"\n"); 			
              strcpy(newClient->message,message);

              //Send some data
              if(send(sock, newClient, 1540, 0) == -1)
              {
                   printf("Send failed\n");
                   exit(8);
              }
        }
        close(sock);
        exit(0);
    }
            
    pid_receive=fork();
    if(pid_receive == -1)
    {
        printf("Error creating child process\n");
        exit(9);
    }
    if(pid_receive == 0)
    {
        char server_reply[1540];
        
                
        while(recv(sock, server_reply, 1540, 0) > 0)
        {
             printf("%s\n", server_reply);
        }      
        close(sock);                
        exit(0);
    }
           
    int status1, status2;
    wait(&status1);
    wait(&status2);

    close(sock);
    return 0;
}

