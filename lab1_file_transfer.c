#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error (const char *msg) {
    perror(msg);
    exit;
}

int main (int argc,char *argv[]) {
    
    if(!strcmp(argv[1],"tcp") && !strcmp(argv[2],"send")) {
        int sockfd,newsockfd,portno;
    	socklen_t clilen;
    	char buffer[256];
    	struct sockaddr_in serv_addr,cli_addr;
    	int n;

    	// create socket
    	sockfd = socket(AF_INET,SOCK_STREAM,0);
    	if(sockfd < 0) {
        	error("ERROR opening socket");
    	}
    	bzero((char*) &serv_addr,sizeof(serv_addr));
    	// get port number
   	    portno = atoi(argv[4]);
    	serv_addr.sin_family = AF_INET;
    	serv_addr.sin_addr.s_addr = INADDR_ANY;
    	serv_addr.sin_port = htons(portno);
    	// bind
    	if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0) {
            error("EROOR on binding");
    	}
        // listen socket <= 5
        listen(sockfd,5);
        // accept
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);
        if(newsockfd < 0) {
            error("ERROR on accept");
        }
        // clear buffer
        bzero(buffer,256);
        // read from client
        n = read(newsockfd,buffer,255);
        if(n < 0) {
            error("ERROR reading from socket");
        }
        printf("Here is the message : %s\n",buffer);
        n = write(newsockfd,"I got your message",18);
        if(n < 0) {
            error("ERROR writing to socket");
        }
        // close socket
        close(newsockfd);
        close(sockfd);

    }else if(!strcmp(argv[1],"tcp") && !strcmp(argv[2],"recv")) {
        
        int sockfd,portno,n;
        struct sockaddr_in serv_addr;
        struct hostent *server;

        char buffer[256];
        // get port number
        portno = atoi(argv[4]);
        // create socket
        sockfd = socket(AF_INET,SOCK_STREAM,0);
        if(sockfd < 0) {
            error("ERROR opening socket");
        }
        server = gethostbyname(argv[3]);
        if(server == NULL) {
            fprintf(stderr,"ERROR,no such host\n");
            exit(0);
        }
        
        bzero ((char*)&serv_addr,sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char*)server->h_addr,(char*)&serv_addr.sin_addr.s_addr,server->h_length);
        serv_addr.sin_port = htons(portno);
        // connect
        if(connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
            error("ERROR connecting");
        }
        // clear buffer
        bzero(buffer,256);
        strcpy(buffer,"client asks for files!");
        n = write(sockfd,buffer,strlen(buffer));
        if(n < 0) {
            error("ERROR writing to socket");
        }
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if(n < 0) {
            error("ERROR reading from socket");
        }
        printf("%s\n",buffer);
        close(sockfd);

    }
    
    
    return 0;
}
