#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <time.h>
#include <arpa/inet.h>
# include <fcntl.h>

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
        int split_size = size/20;
        int mod = size%20;
        int index = 0;
        char buf[split_size],buf2[(split_size+1)];
        time_t timep;
        struct tm *p;
        while(!feof(fp)) {
            if(mod > 0) {
                bytes_num = fread(buf2,sizeof(char),sizeof(buf2),fp);
                bytes_num = write(newsockfd,buf2,bytes_num);
                mod--;
            }else{
                bytes_num = fread(buf,sizeof(char),sizeof(buf),fp);
                bytes_num = write(newsockfd,buf,bytes_num);
            }
            // get local time
            time(&timep);
            p = localtime(&timep);
            printf("%d%% %d/%d/%d %d:%d:%d\n",index*5,(1900+p->tm_year),(1+p->tm_mon),(p->tm_mday),p->tm_hour,p->tm_min,p->tm_sec);
            index++;
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
            if(bytes_num == 0) {
                break;
            }
            bytes_num = fwrite(buf,sizeof(char),bytes_num,fp);
        }
        
        fclose(fp);
        close(sockfd);

    }
    else if(!strcmp(argv[1],"udp") && !strcmp(argv[2],"send")) {
        struct sockaddr_in serv_addr;
        struct sockaddr_in clie_addr;
        char recv_buf[1024];
        int sock_id,recv_len,clie_addr_len;
        FILE *fp;

        // create aocket
        if((sock_id = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
            error("Create socket failed.\n");
        }
        
        memset(&serv_addr,0,sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(atoi(argv[4]));
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        // bind 
        if(bind(sock_id,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
            error("bind socket failed.\n");
        }

        // file_size
        int size = file_size(argv[5]);
        printf("The size of file is %d\n",size);

        clie_addr_len = sizeof(clie_addr);
        bzero(recv_buf,1024);
        if(recv_len = recvfrom(sock_id,recv_buf,1024,0,(struct sockaddr *)&clie_addr,&clie_addr_len)) {
            if(recv_len < 0) {
                printf("Recieve data from client failed.\n");
            }
            printf("client : %s\n",recv_buf);
            bzero(recv_buf,1024);

            // file open
            if((fp = fopen(argv[5],"rb")) == NULL) {
                error("file open failed.\n");
            }
            // send file
            int read_len,send_len;
            int split_size = size/20;
            int mod = size%20;
            int index = 1;
            char buf[split_size],buf2[(split_size+1)];
            time_t timep;
            struct tm *p;
 
            bzero(buf,sizeof(split_size));
            bzero(buf2,sizeof(split_size+1));

            printf("buf size : %d %d\n",(int)sizeof(buf),(int)sizeof(buf2));
            while(mod>0 && (read_len = fread(buf2,sizeof(char),(split_size+1),fp)) > 0) {
                send_len = sendto(sock_id,buf2,read_len,0,(struct sockaddr *)&clie_addr,clie_addr_len);
                mod--;
                bzero(buf2,sizeof(split_size+1));
                time(&timep);
                p = localtime(&timep);
                printf("%d%% %d/%d/%d %d:%d:%d\n",index*5,(1900+p->tm_year),(1+p->tm_mon),(p->tm_mday),p->tm_hour,p->tm_min,p->tm_sec);
                index++;
            }
            while((read_len = fread(buf,sizeof(char),split_size,fp)) > 0) {
                send_len = sendto(sock_id,buf,read_len,0,(struct sockaddr *)&clie_addr,clie_addr_len);
                bzero(buf,sizeof(split_size));
                time(&timep);
                p = localtime(&timep);
                printf("%d%% %d/%d/%d %d:%d:%d\n",index*5,(1900+p->tm_year),(1+p->tm_mon),(p->tm_mday),p->tm_hour,p->tm_min,p->tm_sec);
                index++;
            }
            // close file
            fclose(fp);
        }
        close(sock_id);

    }
    else if(!strcmp(argv[1],"udp") && !strcmp(argv[2],"recv")) {
        FILE *fp;
        struct sockaddr_in serv_addr;
        char send_buf[1024];
        int sock_id,send_len,serv_addr_len,i_ret;

        // create socket
        if((sock_id = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
            error("Create socket failed.\n");
        }

        memset(&serv_addr,0,sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(atoi(argv[4]));
        inet_pton(AF_INET,argv[3],&serv_addr.sin_addr);
        serv_addr_len = sizeof(serv_addr);

        i_ret = connect(sock_id,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr));
        if(i_ret == -1) {
            error("Connect socket failed.\n");
        }
        
        // send
        strcpy(send_buf,"give me files!");
        if(sendto(sock_id,send_buf,sizeof(send_buf),0,(struct sockaddr*)&serv_addr,serv_addr_len) == -1) {
            error("send to server error!\n");
        }

        // file open
        if((fp = fopen("udp_file","wb")) == NULL) {
            error("file open failed.\n");
        }
        // recv
        int recv_len;
        char buf[1000000];
        int index = 20;
        while(index>0 && (recv_len = recvfrom(sock_id,buf,sizeof(buf),0,(struct sockaddr *)&serv_addr,&serv_addr_len)) > 0) {
            int write_length = fwrite(buf,sizeof(char),recv_len,fp);
            bzero(buf,sizeof(buf));
            index--;
        }
        fclose(fp);
        close(sock_id);
    }
 
    return 0;
}
