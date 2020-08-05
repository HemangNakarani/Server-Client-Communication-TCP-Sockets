
#include<bits/stdc++.h>
#include<stdio.h>
#include<string.h>	
#include<stdlib.h>	
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>

void *connection_handler(void *);
void KillHandler(int) ;

using namespace std;

map<int,int> id;

int main(int argc , char *argv[])
{

	int socket_desc , new_socket , portno, c , *new_sock;
	struct sockaddr_in server , client;
	string message;
	
	signal( SIGINT, KillHandler );
	signal( SIGTERM, KillHandler );
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);

	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	
	//Prepare the sockaddr_in structure
        bzero((char *) &server, sizeof(server));
        portno = atoi(argv[1]);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( portno );
	
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("Bind failed");
		return 1;
	}
		
	//Listen
	listen(socket_desc , 1000);
	
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	
	
	//Accept and incoming connection
	while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		printf("->  Connection accepted for socket id: %d \n\n",new_socket);
		message = "Hello Client , I have received your connection. And now I will assign a handler for you\n";
		string serverinit = "Current Connected Sockets To Server: ";
		
		for(auto dd:id)
		{
			if(dd.second!=0)
			{
					serverinit+=  to_string(dd.first) + ", ";
					string newcon="Socket ID: ";
					newcon+= to_string(new_socket) + " is Connected with Server !!";
					write(dd.first , &newcon[0] , newcon.length());
					
			}
		}

		if(id.size()==0) serverinit="Congrats !!😀️ You are first to connected with server.";
		
		write(new_socket , &message[0] , message.length());
		write(new_socket , &serverinit[0] , serverinit.length());
		
		
		pthread_t sniffer_thread;
		new_sock = new int;
		*new_sock = new_socket;
		
		id[new_socket]=1;
		
		if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
		{
			perror("Could not create thread");
			return 1;
		}

	}
	
	if (new_socket<0)
	{
		perror("Accept failed");
		return 1;
	}
	
	return 0;
}

/*  This will handle connection for each client */
void *connection_handler(void *socket_desc)
{

	int sock = *(int*)socket_desc;
	
	string message;
	char buffer[256];
	
	message = "Greetings! I am your connection handler And we are connected :)\n";
	write(sock , &message[0] , message.length());
	
	while(1)
	{
		bzero(buffer,256);
	    	int n = read(sock,buffer,255);
	    	if (n <= 0) 
	    	{	
	    		cout<<"FREE SOCKET\n";
			id[sock]=0;
			free(socket_desc);
	    		pthread_exit(0); 
	    	}
	    	
	    	printf("Msg Transfer: %d to %d : %s \n",sock,id[sock],buffer);
	    	
	    	if(strcmp(buffer,"Q\n")==0) break;
	    	
	    	if(strcmp(buffer,"SET\n")==0)
	    	{
	    		cout<<"    ->  READING SOCKET ID FOR CHANGING RECEIVER OF ID "<<sock<<" ....\n";
	    		bzero(buffer,256);
	    		int n = read(sock,buffer,255);
	    		
	    		if (n < 0) perror("ERROR reading from socket");
	    		
	    		id[sock] = atoi(buffer);
	    		cout<<"Sender Id: "<<sock<<" set to Reciever Id: "<< id[sock]<<"\n\n";
	    		string initmsg ="Socket " + to_string(sock) + " connected to you";
	    		write( id[sock] , &initmsg[0] , initmsg.length());
	    		continue;	
	    	}
	    	
	    	if(id[sock]==411)
	    	{
	    		for(auto x:id)
	    		{
	    			if(x.first!=sock && x.second!=0)
	    			{
	    				write( x.first , buffer , strlen(buffer));	
	    			}	
	    		}
	    	}
	    	else if(id[id[sock]]!=0)
	    	{
	    		write(id[sock] , buffer , strlen(buffer));	
	    	}
	    	
    	}
	
	cout<<"FREE SOCKET\n";
	id[sock]=0;
	free(socket_desc);
	return 0;
}


void KillHandler(int sig) 
{
	string term ="Q\n";
	for(auto dd:id)
	{
		cout<<dd.first<<" "<<dd.second<<"\n";
		if(dd.second!=0)
		{
			int n = write(dd.first,&term[0],term.length());
			if (n < 0) perror("ERROR in Reading Terminating Command socket");
		}
	}
	
	exit(0);
}

