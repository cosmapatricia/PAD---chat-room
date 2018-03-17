#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write

#include<pthread.h> //for threading, compile with -pthread
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


void *connection_handler(void *);

typedef struct {
        char username[512];
	char message[512];	
	char password[512];
        int socket_fd;
	}client;
        
client clients[1000];
int n=0;
pthread_mutex_t mutex ;
int threadNumber = 0 ;

int getClientIndex(client newClient)
{
	for (int i=0;i<n;i++){
		if(clients[i].socket_fd==newClient.socket_fd){
			return i;
		}
	}
	return -1;
}

void readUsersFromFile()
{
	FILE * f=fopen("users.txt","r");
	char username[1024];
	char password[1024];
	while(1) {
      if( feof(f) ) { 
         break ;
      }
      fscanf(f,"%s",username);
   	  fscanf(f,"%s",password);
   	  printf("username: %s; password: %s\n",username,password);
   	}
   	fclose(f);
}

void addUserToFile(client * user)
{
	FILE * f=fopen("users.txt","a");
	fprintf(f,"\n%s",user->username);
	fprintf(f,"\n%s",user->password);
   	fclose(f);
}

int main(int argc , char *argv[])
{
	//readUsersFromFile();
	//client * user=(client *)malloc(sizeof(client));
	//strcpy(user->username,"newUser");
	//strcpy(user->password,"newUserPas");
	//addUserToFile(user);
    int socket_desc , new_socket , c , *new_sock;
    struct sockaddr_in server , client;
    pthread_t sniffer_thread;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);
    pthread_mutex_init(&mutex, NULL);
    
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("Bind failed");
        exit(1);
    }
    puts("Bind done");

    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
         if (new_socket<0)
        {
            perror("Accept failed");
            exit(1);
        }
        puts("Connection accepted");

        new_sock = malloc(1);
        *new_sock = new_socket;

        if( pthread_create( &sniffer_thread , &attr ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("Could not create thread");
            exit(1);
        }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
        
        pthread_mutex_lock(&mutex);
        threadNumber++;
        n++;
        printf("%d fire active \n", threadNumber);
        pthread_mutex_unlock(&mutex);        
    }


    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size, i;
    //char client_message[1024];
    client * client_received=(client *)malloc(1540);
    
    int count=0;

    //Receive a message from client
    while( (read_size = recv(sock , client_received , 1024 , 0)) > 0 )
    {
    		
        //Send the message back to client
        
        pthread_mutex_lock(&mutex);
                client newClient;
                char client_message[1024];
                char empty_buff[512];
		strcpy(newClient.message,client_received->message);
		strcpy(newClient.username,client_received->username);
		strcpy(newClient.password,client_received->password);
                newClient.socket_fd=sock;

                        
			int pos=getClientIndex(newClient);
			
			if(pos==-1)	//the client is new to the server
			{
				clients[n-1]=newClient;
			}
			else
			{
				strcpy(clients[pos].message,newClient.message);
			}
			
                        count++;
			printf("\n%s(fd= %d): %s\n", newClient.username, newClient.socket_fd, newClient.message);
                        
			//send(new_socket, (char*)buffer, sizeof(buffer), 0);
                        //printf("n: %d\n",n);
                        
                        strcpy(client_message, client_received->username);
                        strcat(client_message,": ");
                        strcat(client_message, client_received->message);
                        strcat(client_message, empty_buff);
                    
                        printf("client password: %s\n",newClient.password);
                        
                        
                        for(i=0; i<n; i++)
                        {                            
                                send(clients[i].socket_fd, (char*)client_message, 1538, 0);
                        }
         pthread_mutex_unlock(&mutex);
        
    }

    if(read_size <= 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
//     else if(read_size == -1)
//     {
//         perror("recv failed bla bla");
//     }

    //Free the socket pointer
    close(sock);
    free(socket_desc);
    
    pthread_mutex_lock(&mutex);
    threadNumber--;
    //stergem clientul care s-a deconectat
    for(i=0; i<n; i++)
    {
        if(clients[i].socket_fd==sock)
            break;
    }
    if(i!=n)
        for(int j=i; j<n-1;j++)
        {
            clients[j]=clients[j+1];
        }
    n--;
    printf("%d clienti conectati \n", n);
    printf("%d fire active \n", threadNumber);
    pthread_mutex_unlock(&mutex);
    return NULL;    
}
