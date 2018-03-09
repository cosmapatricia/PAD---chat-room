#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include <unistd.h>
#include<stdlib.h>


int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char message[1024] , server_reply[1024];

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

    //keep communicating with server
    while(1)
    {

        printf("Enter message: ");
        fgets(message, sizeof(message),stdin);
        //scanf("%s" , message);

        /*while(strcmp(message,"end\n")!=0)
        {
            if( send(sock , message , sizeof(message) , 0) < 0)
            {
                puts("Send failed");
                exit(1);
            }
            printf("\nEnter message: ");
            fgets(message, sizeof(message),stdin);
        }*/
        
        //Send some data
        if( send(sock , message , sizeof(message) , 0) < 0)
        {
            puts("Send failed");
            exit(1);
        }

        //Receive a reply from the server
//         if( recv(sock , server_reply , 2000 , 0) < 0)
//         {
//             puts("recv failed");
//             break;
//         }
        while(recv(sock , server_reply , 1024, 0)>0)
        {
            if(strcmp(server_reply,"end\n")==0)
            {
                printf("am intrat in iful cu end\n");
                break;
            }
            printf("Server Reply: %s\n", server_reply);
        }
        
            
        
    }

    close(sock);
    return 0;
}

