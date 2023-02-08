#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#define MAX 8000
#define BUFFER_SIZE 8000

struct args {
	const char *document_root;
	int connfd;
};

void serveClient(int connfd, const char *fileLocation){

	FILE *filePtr = fopen(fileLocation,"r");
	if (NULL == filePtr){
		perror("Failed: ");
		return;
	}
	int fd = fileno(filePtr);
	printf("server succesfully opened requested file:%d\n",fd);

	struct stat st;
	fstat(fd,&st);
	off_t file_size = st.st_size;

	char file_buff[BUFFER_SIZE];

	int n;
	do{
		bzero(file_buff, BUFFER_SIZE);
		n = fread(file_buff,1,BUFFER_SIZE,filePtr);
		if (n>0){
			write(connfd, file_buff, n);
		}
	}while(n>0);
	fclose(filePtr);
}

void *handleRequestThread(void *thread_args){

	//parsing supplied arguments
	struct args *passed_args;
	passed_args = (struct args*) thread_args;
	const char *document_root = passed_args->document_root;
	int connfd = passed_args->connfd;


	//first parse the http header sent by the client
	char requestBuffer[BUFFER_SIZE];
	bzero(requestBuffer,BUFFER_SIZE);
	read(connfd, requestBuffer, sizeof(requestBuffer));

	//Log the client request on the server's console log
	printf("Server recieved client request\n");
	printf("Client Request Header:\n");
	printf("%s\n",requestBuffer);

	//get the time from the server
	char timestr[200];
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(timestr, 200, "%a, %d %b %Y %H:%M:%S %Z", &tm);
	printf("Server time: %s\n", timestr);

	char *httpMethod = strtok(requestBuffer, " ");	
	char *requestedFile = strtok(NULL, " ");

	//check for any bad requests
	if(strncmp(requestedFile,"//",2)==0){
		char responseHeader[1000] = "HTTP/1.0 400 Bad Request\nContent-Type: text/html;charset=UTF-8\nContent-Length: 100000000\r\n";

		strcat(responseHeader,"Date: ");
		strcat(responseHeader,timestr);
		strcat(responseHeader,"\r\n\r\n");
		write(connfd, responseHeader, strlen(responseHeader));
		char formattedFile[BUFFER_SIZE];
		strcat(formattedFile,document_root);
		strcat(formattedFile,"/http400.html");
		serveClient(connfd,formattedFile);
		close(connfd);
		pthread_exit(NULL);
		return NULL;

	}

	//first handle the default / loading page
	if(strcmp(requestedFile,"/")==0){
		char responseHeader[1000] = "HTTP/1.0 200 OK\nContent-Type: text/html;charset=UTF-8\nContent-Length: 100000000\r\n";

		strcat(responseHeader,"Date: ");
		strcat(responseHeader,timestr);
		strcat(responseHeader,"\r\n\r\n");
		write(connfd, responseHeader, strlen(responseHeader));
		char formattedFile[BUFFER_SIZE];
		strcat(formattedFile,document_root);
		strcat(formattedFile,"/index.html");
		serveClient(connfd,formattedFile);
		close(connfd);
		pthread_exit(NULL);
		return NULL;
	}

	//check the requested file
	char formattedFile[BUFFER_SIZE];
	strcat(formattedFile,document_root);
	strcat(formattedFile,requestedFile);
	printf("requested file is: %s\n", formattedFile);
	char statusCode[40];
	FILE *filePtr;
	filePtr = fopen(formattedFile,"r");
	if (NULL == filePtr){
		perror("unable to open file\n");
		if (errno == 2){
			strcpy(statusCode, "404 Not Found");
		}
		else if(errno == 13){
			strcpy(statusCode, "403 Forbidden");
		}
	}
	else{
		strcpy(statusCode, "200 OK");
	}
	
	char responseHeader[1000] = "HTTP/1.0 ";
	strcat(responseHeader, statusCode);
	strcat(responseHeader, "\nContent-Type: ");
	char *contentLength = "\nContent-Length: 100000000\r\n";

	//handle error codes 404 and 403
	if(strcmp(statusCode,"404 Not Found")==0){
		printf("Server Error, file requested resulted in: %s\n", statusCode);
		char responseHeader[1000] = "HTTP/1.0 404 Not Found\nContent-Type: text/html;charset=UTF-8\nContent-Length: 100000000\r\n";

		strcat(responseHeader,"Date: ");
		strcat(responseHeader,timestr);
		strcat(responseHeader,"\r\n\r\n");
		write(connfd, responseHeader, strlen(responseHeader));
		char formattedFile[BUFFER_SIZE];
		strcat(formattedFile,document_root);
		strcat(formattedFile,"/http404.html");
		serveClient(connfd,formattedFile);
		close(connfd);
		pthread_exit(NULL);
		return NULL;


	}else if(strcmp(statusCode,"403 Forbidden")==0){
		printf("Server Error, file requested resulted in: %s\n", statusCode);
		char responseHeader[1000] = "HTTP/1.0 403 Forbidden\nContent-Type: text/html;charset=UTF-8\nContent-Length: 100000000\r\n";

		strcat(responseHeader,"Date: ");
		strcat(responseHeader,timestr);
		strcat(responseHeader,"\r\n\r\n");
		write(connfd, responseHeader, strlen(responseHeader));
		char formattedFile[BUFFER_SIZE];
		strcat(formattedFile,document_root);
		strcat(formattedFile,"/http403.html");
		serveClient(connfd,formattedFile);
		close(connfd);
		pthread_exit(NULL);
		return NULL;

	}

	//content type parsing
	char *contentType = strrchr(requestedFile,'.');
	printf("content type detected is: %s\n", contentType);
	if (NULL == contentType){
		printf("file is not supported\n");
		//invalid file type, sending 400
		char responseHeader[1000] = "HTTP/1.0 400 Bad Request\nContent-Type: text/html;charset=UTF-8\nContent-Length: 100000000\r\n";
	
		strcat(responseHeader,"Date: ");
		strcat(responseHeader,timestr);
		strcat(responseHeader,"\r\n\r\n");
		write(connfd, responseHeader, strlen(responseHeader));
		char formattedFile[BUFFER_SIZE];
		strcat(formattedFile,document_root);
		strcat(formattedFile,"/http400.html");
		serveClient(connfd,formattedFile);
		close(connfd);
		pthread_exit(NULL);
		return NULL;

		
	}
	if (strncmp(contentType, ".html", 5)==0){
		strcat(responseHeader, "text/html;charset=UTF-8");
	}
	else if (strncmp(contentType, ".txt", 4)==0){
		strcat(responseHeader, "text/txt");
	}
	else if (strncmp(contentType, ".js", 3)==0){
		strcat(responseHeader, "text/js");
	}
	else if (strncmp(contentType, ".css", 4)==0){
		strcat(responseHeader, "text/css");
	}
	else if((strncmp(contentType, ".jpg", 4)==0) || (strncmp(contentType, ".ico", 4)==0)){
		strcat(responseHeader, "image/jpg");
	}
	else if (strncmp(contentType, ".gif", 4)==0){
		strcat(responseHeader, "image/gif");
	}
	else if (strncmp(contentType, ".png", 4)==0){
		strcat(responseHeader, "image/png");
	}else{
		//invalid file type, sending 400
		char responseHeader[1000] = "HTTP/1.0 400 Bad Request\nContent-Type: text/html;charset=UTF-8\nContent-Length: 100000000\r\n";

		strcat(responseHeader,"Date: ");
		strcat(responseHeader,timestr);
		strcat(responseHeader,"\r\n\r\n");
		write(connfd, responseHeader, strlen(responseHeader));
		char formattedFile[BUFFER_SIZE];
		strcat(formattedFile,document_root);
		strcat(formattedFile,"/http400.html");
		serveClient(connfd,formattedFile);
		close(connfd);
		pthread_exit(NULL);
		return NULL;
	}
	
	//content length addition
	strcat(responseHeader,"\nContent-Length: 100000000\r\n"); 
	strcat(responseHeader,"Date: ");
	strcat(responseHeader,timestr);
	strcat(responseHeader,"\r\n\r\n");
	write(connfd, responseHeader, strlen(responseHeader));

	//serve the file
	serveClient(connfd,formattedFile);
	close(connfd);
	pthread_exit(NULL);

	return NULL;
}

int startServerAtPort(int port){
	int sockfd;

	//open a tcp socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0){
		perror("unable to open a tcp socket\n");
		exit(1);
	}
	printf("server tcp socket successfully opened\n");

	//allow for immediate use of the port
	int sockoptval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));
	
	//bind the tcp socket to a port
	struct sockaddr_in serverAddr;
	memset((char *)&serverAddr, 0, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(port);

	if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr))<0){
		perror("binding of tcp socket failed\n");	
		exit(1);
	}
	printf("server tcp socket sucessfully binded to port: %d\n",port);

	//accept the connection with a queue backlog of 50
	if (listen(sockfd,50)<0){
		perror("listening failed\n");
		exit(1);
	}
	printf("server now listening to tcp socket %d\n",sockfd);

	return sockfd;

}


int main(int argc, char *argv[]){

	if (argc != 5){
		printf("incorrect number of arguments given\n");
		printf("number of arguments given: %d\n", argc);
		printf("failed to start the server\n");
		return -1;	
	}

	int port = atoi(argv[4]);	
	const char *document_root = argv[2];

	int sockfd = startServerAtPort(port);
	
	//client connection information
	int connfd;
	struct sockaddr_in clientAddr;
	socklen_t alen;
	
	while(1){

		if ((connfd = accept(sockfd, (struct sockaddr *)&clientAddr, &alen))<0){
			perror("unable to accept client\n");
			exit(1);
		}
		printf("Server accepted client %d\n",connfd);

		struct args * thread_args = (struct args *)malloc(sizeof(struct args));
		thread_args->document_root = (char *)document_root;
		thread_args->connfd = connfd;

		pthread_t t;
		int *pclient = malloc(sizeof(int));
		*pclient = connfd;

		pthread_create(&t, NULL,handleRequestThread,(void *)thread_args); 
	}

	return 0;
	
}