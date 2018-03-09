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


int main(int argc , char *argv[])
{
    int sock, *new_sock;
    struct sockaddr_in server;
    pthread_t transmit_thread;
    pthread_t receive_thread;
    pthread_attr_t attr;

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
                            
                char message[1024];                
                while(1)
                {
                    printf("Enter message: ");
                    fgets(message, sizeof(message),stdin);
                        //Send some data
                    if( send(sock , message , sizeof(message) , 0) < 0)
                    {
                        puts("Send failed");
                        exit(1);
                    }
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
                char server_reply[1024];
                
                while(recv(sock , server_reply , 1024, 0)>0)
                {
                    printf("Server Reply: %s\n", server_reply);
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

