#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#define PORT 8080


typedef struct {
	char * ip;
	char *message;
	}client;
	

client clients[1000];
int n=0;


int getClientIndex(client newClient)
{
	for (int i=0;i<n;i++){
		if(strcmp(clients[i].ip,newClient.ip)==0){
			return i;
		}
	}
	return -1;
}

int main(int argc, char const *argv[])
{

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";
    
   	 // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT); 
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    for (;;)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        } 

        if (fork()==0)  /* procesul fiu */
        { 
	    close(server_fd);
	    //procesare cerere
	    while((valread = read(new_socket, buffer, 1024))>0)
	    {
			client newClient;
			newClient.ip=inet_ntoa(address.sin_addr);
			strcpy(newClient.message,buffer);
			int pos=getClientIndex(newClient);
			
			if(pos==-1)	//the client is new to the server
			{
				clients[n++]=newClient;
			}
			else
			{
				strcpy(clients[pos].message,newClient.message);
			}
			
			printf("IP address is: %s %s\n",newClient.ip, newClient.message);
			
			
			send(new_socket, (char*)buffer, sizeof(buffer), 0);
			
	    }
	    close (new_socket);
	    exit(0);
        }
        //close (new_socket);  /* parinte */
     }
   
}
