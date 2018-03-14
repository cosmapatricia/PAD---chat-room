#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h> //for threading, compile with -pthread
#include <sys/types.h>
#include <sys/wait.h>

// void *transmit(void *socket_desc)
// {
//     //Get the socket descriptor
//     int sock = *(int*)socket_desc;
//     char message[1024];
//     
//     printf("Enter message: ");
//     
//     while(1)
//     {
//         fgets(message, sizeof(message),stdin);
//             //Send some data
//         if( send(sock , message , sizeof(message) , 0) < 0)
//         {
//             puts("Send failed");
//             exit(1);
//         }
//         printf("Enter message: ");
//     }
// 
//     //Free the socket pointer
//     close(sock);
//     free(socket_desc);
//     
// 
//     return NULL;    
// }

// void *receive(void *socket_desc)
// {
//     //Get the socket descriptor
//     int sock = *(int*)socket_desc;
//     char server_reply[1024];
//     
//     while(recv(sock , server_reply , 1024, 0)>0)
//     {
//         printf("Server Reply: %s\n", server_reply);
//     }
//         
// 
//     //Free the socket pointer
//     close(sock);
//     free(socket_desc);
//     
// 
//     return NULL;    
// }


typedef struct {
	int socket_fd;
	char message[1024];
	char username[1024];
	char password[1024];
	}client;


int sock, *new_sock;
struct sockaddr_in server;
pthread_t transmit_thread;
pthread_t receive_thread;
pthread_attr_t attr;


int main(int argc , char *argv[])
{
    

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);
    
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        exit(1);
    }

    puts("Connected\n");
    puts("Bienvenido al Chatroom, puedes empezar a escribir en la sala!");


//         new_sock = malloc(1);
//         *new_sock = sock;
// 
//         if( pthread_create( &transmit_thread , NULL,  transmit, (void*) new_sock) < 0)
//         {
//             perror("could not create thread");
//             exit(1);
//         }
//         
//         if( pthread_create( &receive_thread , NULL,  receive, (void*) new_sock) < 0)
//         {
//             perror("could not create thread");
//             exit(1);
//         }
        
        //Daca deschizi 3 clienti, trimiti mesaje din toti si dupa il inchizi pe primu si incerci sa trm mesaj dintr-un client ramas, celalalt client nu-l primeste WTF
            pid_t pid_transmit, pid_receive;
            
            if( ( pid_transmit=fork() ) < 0)
            {
                perror("Eroare");
                exit(1);
            }
            if(pid_transmit==0)
            {
            	char username[1024];
				char password[1024];
            	client *newClient=(client *)malloc(sizeof(client));
            	newClient->socket_fd=0;   
            	puts("Enter your username:");
            	fgets(username,sizeof(username),stdin);
            	strtok(username,"\n");		//remove newline from string
				puts("Enter your password:");
		        fgets(password,sizeof(password),stdin);
		        strtok(password,"\n");
		        strcpy(newClient->username,username);   
                strcpy(newClient->password,password);
                        
                char message[1024];
                strcat(message, strcat(username," has joined the chat room")); 
                
                strcpy(newClient->message,message);
                        //Send some data
                    if( send(sock , newClient , sizeof(client) , 0) < 0)
                    {
                        puts("Send failed");
                        exit(1);
                    }
                    printf("Send: %s: %s\n",newClient->username,newClient->message);
                               
                while(1)
                {
                    printf("Enter message: ");
                    fgets(message, sizeof(message),stdin);
                    strtok(message,"\n"); 			
                    strcpy(newClient->message,message);
                        //Send some data
                    if( send(sock , newClient , sizeof(client) , 0) < 0)
                    {
                        puts("Send failed");
                        exit(1);
                    }
                    printf("Send: %s: %s\n",newClient->username,newClient->message);
                }
            
                close(sock);
                exit(0);
            }
            
            if( ( pid_receive=fork() ) < 0)
            {
                perror("Eroare");
                exit(1);
            }
            if(pid_receive==0)
            {
                client *server_reply=(client *)malloc(sizeof(client));;
                
                while(recv(sock , server_reply , sizeof(client), 0)>0)
                {
                    printf("Server Reply: %s: %s\n", server_reply->username, server_reply->message);
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

