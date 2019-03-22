#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>

void error (const char *msg) {
    perror(msg);
    exit;
}

int file_size(char* filename) {
    struct stat statbuf;
    stat(filename,&statbuf);
    int size = statbuf.st_size;
    return size;
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

        // get file size
        int size = file_size(argv[5]);
        printf("The size of file is %d\n",size);

        // file open
        FILE *fp;
        fp = fopen(argv[5],"rb");
        if(fp < 0) {
            error("ERROR opening the file");
        }

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
        // send file name
        n = write(newsockfd,argv[5],sizeof(argv[5]));
        if(n < 0) {
            error("ERROR writing to socket");
        }
        bzero(buffer,256);
        // read from client
        n = read(newsockfd,buffer,255);
        if(n < 0) {
            error("ERROR reading from socket");
        }
        printf("client : %s\n",buffer);
       
        // send file
        int bytes_num;
        int split_size = 1000;
        char buf[split_size];
        while(!feof(fp)) {
            bytes_num = fread(buf,sizeof(char),sizeof(buf),fp);
            printf("READ %d bytes\n",bytes_num);
            bytes_num = write(newsockfd,buf,bytes_num);
            printf("SEND %d bytes\n",bytes_num);
        }

        // close socket
        fclose(fp);
        close(newsockfd);
        close(sockfd);

    }
    
    else if(!strcmp(argv[1],"tcp") && !strcmp(argv[2],"recv")) {
        
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
        // send
        n = write(sockfd,buffer,strlen(buffer));
        if(n < 0) {
            error("ERROR writing to socket");
        }
        bzero(buffer,256);
        // recv
        n = read(sockfd,buffer,255);
        if(n < 0) {
            error("ERROR reading from socket");
        }
        
        //file open
        FILE *fp;
        int bytes_num;
        char buf[1000000];
        char file_name[50] = "client/";
        strcat(file_name,buffer);
        
        if ((fp = fopen(file_name,"wb")) == NULL ) {
            error("ERROR opening the file");
        }
        bzero(buffer,256);
        strcpy(buffer,"Start to send file to me!");
        // send
        n = write(sockfd,buffer,strlen(buffer));
        if(n < 0) {
            error("ERROR writing to socket");
        }

        while(1) {
            bytes_num = read(sockfd,buf,sizeof(buf));
            printf("READ %d bytes\n",bytes_num);
            if(bytes_num == 0) {
                break;
            }
            bytes_num = fwrite(buf,sizeof(char),bytes_num,fp);
            printf("WRITE %d bytes\n",bytes_num);
        }
        
        fclose(fp);
        close(sockfd);

    }
    
    return 0;
}
