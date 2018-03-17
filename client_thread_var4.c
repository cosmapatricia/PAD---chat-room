#include<stdio.h> 
#include<string.h>    
#include<sys/socket.h>    
#include<arpa/inet.h> 
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>        //for threading, compile with -pthread
#include <sys/types.h>
#include <sys/wait.h>

void end_transmission(int sign)
{
    //printf("End of transmission.\n");
    exit(0);
}

int main(int argc , char *argv[])
{
    int sock, *new_sock;
    struct sockaddr_in server;
    //Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Error creating socket\n");
	exit(1);
    }
    //puts("Socket created");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    //Connect to remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) ==-1)
    {
        printf("Error connecting socket\n");
        exit(2);
    }
    puts("Connected");

    pid_t pid_transmit, pid_receive;
    pid_transmit=fork();
    if(pid_transmit == -1)
    {
    	printf("Error creating child process\n");
        exit(3);
    }
    if(pid_transmit == 0)            
    {
        struct sigaction act;
        act.sa_handler=end_transmission;
        act.sa_flags=0;
        if(sigaction(SIGUSR1, &act, NULL)<0)
        {
            printf("Error setting the handler for SIGUSR1\n");
            exit(7);
        }
        
        char username[512];
	char password[512];  
        
        printf("Enter your username: ");
        fgets(username,sizeof(username),stdin);
        strtok(username,"\n");		                             //remove newline from string

	printf("Enter your password: ");
	fgets(password,sizeof(password),stdin);
	strtok(password,"\n");

                        
        char message[1025];
        strcpy(message,username);  
	strcat(message,":");      
        strcat(message,password);

        //Send login info
        if(send(sock, message, 1025, 0) == -1)
        {
              printf("Send failed\n");
              exit(4);
        }
               
        while(1) //while(fgets)
        {
              fgets(message,sizeof(message),stdin);
              strtok(message,"\n"); 			

              //Send some data
              if(send(sock, message, 1025, 0) == -1)
              {
                   printf("Send failed\n");
                   exit(5);
              }
        }
        close(sock);
        exit(0);
    }
            
    pid_receive=fork();
    if(pid_receive == -1)
    {
        printf("Error creating child process\n");
        exit(6);
    }
    if(pid_receive == 0)
    {
        char server_reply[1025];
                
        while(recv(sock, server_reply, 1025, 0) > 0)
        {
	     if(strcmp(server_reply,"Wrong password!")==0)
	     {
                 printf("%s\n", server_reply);
                 if(kill(pid_transmit, SIGUSR1)<0)
                 {
                    printf("Error sending SIGUSR1 to the transmission process\n");
                    exit(8);
                 }
		 close(sock);
		 exit(0);
	     }
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

