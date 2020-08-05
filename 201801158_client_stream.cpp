
#include<bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h> 
#include<arpa/inet.h>	
#include<pthread.h>

void *connection_handler(void *);
void KillHandler(int) ;

using namespace std;

int *new_sock = new int;


int main(int argc, char *argv[])
{

	printf("\n*** Instructions ***\n\n->  Initialy You can send message to only server itself.\n->  For Changing Reciver for ONE TO ONE Mode as you are Sender, \n      - Type SET and hit ENTER, it will ask for socket ID,\n      - enter socket Id to connect and again hit enter.\n->  Now you can send message to socket with entered ID.\n->  Similarly for BROADCAST Mode type \"411\" after typing SET and hitting ENTER\n->  Type Q and hit Enter To quit conversation\n\n");
	
	
    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    signal( SIGINT, KillHandler );	

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) 
        perror("ERROR opening socket");
    
    server = gethostbyname(argv[1]);
    
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    
    
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        perror("ERROR connecting");
    
    
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    
    if (n < 0) 
    {
	perror("ERROR reading from socket");
	exit(0);
    }
    
    printf("%s\n",buffer);
    
    pthread_t sniffer_thread;
    *new_sock = sockfd;
    
    if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
	{
		perror("Could not create thread");
		return 1;
	}
	
	cout<<"Connecting..."<<"\n\n";
	
	sleep(1);
    
    while(1)
    {    
	    printf("Please enter the message: ");
	    bzero(buffer,256);
	    fgets(buffer,255,stdin);	    
	    
	    if(strcmp("\n",buffer)==0) 
	    {
	    	cout<<"     - Enter Something !!!\n";	
	    	continue;
	    }
	    n = write(sockfd,buffer,strlen(buffer));
	    
	    
	    if (n < 0) perror("ERROR writing to socket");
	  
	    
	    if(strcmp(buffer,"Q")==10) break;
	    
	    if(strcmp(buffer,"SET\n")==0) 
	    {
	    	cout<<"ENTER SOCKET ID TO COMMUNICATE: ";
	    	bzero(buffer,256);
	    	fgets(buffer,255,stdin);	    
	   	n = write(sockfd,buffer,strlen(buffer));
	   	
	   	if (n < 0) perror("ERROR in Setting socket");
	    	
	    }
	   
    }
    
    pthread_join(sniffer_thread,NULL);
   
    return 0;
}




void *connection_handler(void *socket_desc)
{
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	
	char buffer[256];
	
	bzero(buffer,256);
	int n = read(sock,buffer,255);    
	if (n < 0) perror("ERROR reading from socket");
	printf("%s\n",buffer);
	
	printf("-------------------------------------------------------------------------------\n");
	
	while(1)
	{
		bzero(buffer,256);
	    	int n = read(sock,buffer,255);
	    
	    	if (n <= 0) 
	    	{
	    		cout<<"SERVER CLOSED !!!⚠️\n";	
	    		KillHandler(1);
	    		perror("ERROR reading from socket");
	    	}
	    	
	    	if(strcmp(buffer,"\n")==0) exit(0);
	    	
	    	if(strcmp(buffer,"Q\n")==0)
	    	{
	    		cout<<"SERVER CLOSED !!!⚠️\n";
	    		exit(0);	
	    	}
		 
	    	printf("\n\n**NEW MESSAGE : %s\n",buffer);
	    
    	}
    	
	//Free the socket pointer
	free(socket_desc);
	return 0;
}


void KillHandler(int sig) 
{
	string term ="Q\n";
	int n = write(*new_sock,&term[0],term.length());
	if (n < 0) perror("ERROR in Reading Terminating Command socket");
	exit(0);
}

