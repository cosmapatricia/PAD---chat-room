#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>    
#include<pthread.h> //for threading, compile with -pthread
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NOT_VALID -1
#define NEW 1
#define VALID 0

void *connection_handler(void *);

typedef struct {
	int socket_fd;
	char message[512];
	char username[512];
	char password[512];
	} client;

        
//structura pt continutul bazei de date/fisierului de care se foloseste serverul
typedef struct {
	char username[512];
	char password[512];
	} user;
	
int usersLength=0;        
client clients[1000];
user usersFromFile[1000];
int n=0;
pthread_mutex_t mutex;
int threadNumber = 0; 

int getClientIndex(client newClient)
{
	for (int i=0;i<n;i++){
		if(strcmp(clients[i].username, newClient.username)==0){
			return i;
		}
	}
	return -1;
}

int getClientStatus(client newClient)
{
	for (int i=0;i<usersLength;i++)
	{
		if(strcmp(newClient.username,usersFromFile[i].username)==0)
		{
			if(strcmp(newClient.password,usersFromFile[i].password)==0)
			{
				return VALID;
			}
			else
			{
				return NOT_VALID;
			}
		}
	}
	return NEW;
}

void readUsersFromFile()
{
	FILE *f=fopen("users.txt","r");
	if(f==NULL)
	{
		printf("Error opening file\n");
		exit(1);
	}
	char username[512];
	char password[512];
	char stringAux[512];
	user aUser;
	while(1) 
	{
        	if( feof(f) ) 
		{ 
         		break;
     	 	}
		fscanf(f,"%s",stringAux);  
	      	if(stringAux[0] == '\0')         //checking for empty strings
		{			
			fscanf(f,"%s",stringAux);
	      	}
	      	strcpy(username,stringAux);
	      	fscanf(f,"%s",stringAux);
	      	if(stringAux[0] == '\0')
		{
			fscanf(f,"%s",stringAux);
		}
		strcpy(password,stringAux);
		
		strcpy(aUser.username,username);
		strcpy(aUser.password,password);
		usersFromFile[usersLength++]=aUser;
   	}	
   	fclose(f);
}

void addUserToFile(user aUser)
{
	FILE *f=fopen("users.txt","a");
        if(f==NULL)
	{
		printf("Error opening file\n");
		exit(2);
	}
	fprintf(f,"\n%s",aUser.username);
	fprintf(f,"\n%s",aUser.password);
   	fclose(f);
}

void printUsers()
{
	printf("USERS: ");
	for (int i=0;i<usersLength;i++)
	{
		printf("%s ",usersFromFile[i].username);
	}
	printf("\n");
}

int main(int argc , char *argv[])
{	   readUsersFromFile();
	   printUsers();
	    int socket_desc, new_socket, c, *new_sock;
	    struct sockaddr_in server, client;
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
                exit(3);
	    }

	    //Prepare the sockaddr_in structure
	    server.sin_family = AF_INET;
	    server.sin_addr.s_addr = INADDR_ANY;
	    server.sin_port = htons(8888);

	    //Bind
	    if (bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	    {
		puts("Bind failed");
		exit(4);
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
		    perror("Accept new client failed");
		    exit(5);
		}
		puts("Connection accepted");

		new_sock = malloc(1);   //malloc(4)? new_sock e pointer la int 
		*new_sock = new_socket;

		if( pthread_create( &sniffer_thread , &attr ,  connection_handler , (void*) new_sock) < 0)
		{
		    perror("Could not create thread");
		    exit(6);
		}
		puts("Handler assigned");
		
		pthread_mutex_lock(&mutex);
		threadNumber++;
		printf("%d active threads\n", threadNumber);
		pthread_mutex_unlock(&mutex);        
	    }
	    return 0;
}


void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size, i;
    char message[1025];
    client newClient;
    int wrong_pass=0;
    if( (read_size = recv(sock, message, sizeof(message), 0)) > 0 )
    {
        pthread_mutex_lock(&mutex);
	char* delimiter=":";
	strcpy(newClient.username,strtok(message,delimiter));
	strcpy(newClient.password,strtok(NULL,delimiter));
	newClient.socket_fd=sock;
        
	//strcpy(newClient.message,message);	//message=1025 si newClient->message-512 dar vad ca merge
        int status=getClientStatus(newClient);
        //printf("STATUS: %d\n",getClientStatus(newClient));
        if(status==VALID || status==NEW)
	{
		    if(status==NEW)
		    {
		    		user newUser;
		    		strcpy(newUser.username,newClient.username);
		    		strcpy(newUser.password,newClient.password);
		    		usersFromFile[usersLength++]=newUser;
                                addUserToFile(newUser);
		    		clients[n++]=newClient;
		    }
		    else if(status==VALID)        
		    {
		    		int pos=getClientIndex(newClient);  
                                if(pos==-1)	//the client has not been added to the current session
                                {
                                    clients[n++]=newClient;
                                }
				else
                                {
                                    strcpy(clients[pos].message,newClient.message);
                                }
		    }
			
		   printf("Client: %s (socket_fd = %d) has connected\n",newClient.username, newClient.socket_fd);              
		   printf("no of clients for current session: %d\n",n);

		   strcpy(message,newClient.username);
		   strcat(message," has connected.\n");
			
		   for(i=0; i<n; i++)
		   {
            		send(clients[i].socket_fd, message, sizeof(message), 0);
        	   }
	}	
	else           
	{
		char msg[1025];
                wrong_pass=1;
                printf("Wrong password for user -> %s\n",newClient.username);
		strcpy(msg,"Wrong password!");
		send(newClient.socket_fd,msg,sizeof(msg),0);
		close(sock);
	}		
     	pthread_mutex_unlock(&mutex);
    }
    else
    {
        printf("Error receiving login info\n");
        exit(7);
    }
	
    //Receive a message from client
    while( (read_size = recv(sock, message, sizeof(message), 0)) > 0 )
    {   
        pthread_mutex_lock(&mutex);
	strcpy(newClient.message,message);	//message=1025 si newClient->message-512 dar vad ca merge
        int pos=getClientIndex(newClient);
        strcpy(clients[pos].message,newClient.message);
        strcpy(message,newClient.username);
        strcat(message,": "); //dc nu pot spatiu dupa : ?
        strcat(message,newClient.message);	      
			
	for(i=0; i<n; i++)
	{
            send(clients[i].socket_fd, message, sizeof(message), 0);
        }
     	pthread_mutex_unlock(&mutex);
        
    }

    if(read_size <= 0 && !wrong_pass)
    {
        strcpy(message, newClient.username);
        strcat(message, " has disconnected.\n");
        printf("%s", message);
        for(i=0; i<n; i++)
	{
            if(clients[i].socket_fd!=sock)
                send(clients[i].socket_fd, message, sizeof(message), 0);
        }
        fflush(stdout);
    }

    //Free the socket pointer
    close(sock);
    free(socket_desc);
    pthread_mutex_lock(&mutex);
    threadNumber--;

    //stergem clientul care s-a deconectat din sesiunea curenta
    if(!wrong_pass)
    {
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
    }
    
    printf("%d connected clients \n", n);
    printf("%d active threads \n", threadNumber);
    pthread_mutex_unlock(&mutex);
    return NULL;    
}
