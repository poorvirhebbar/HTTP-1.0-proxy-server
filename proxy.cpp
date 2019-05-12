#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <netdb.h>

#include "proxy_parse.h"
#define MAX 2000000
using namespace std;

char* convert_Request_to_string(struct ParsedRequest *req)
{

	/* Set headers */
	ParsedHeader_set(req, "Host", req -> host);
	ParsedHeader_set(req, "Connection", "close");

	int iHeadersLen = ParsedHeader_headersLen(req);

	char *headersBuf;

	headersBuf = (char*) malloc(iHeadersLen + 1);

	if (headersBuf == NULL) {
		fprintf (stderr," Error in memory allocation  of headersBuffer ! \n");
		exit (1);
	}

	ParsedRequest_unparse_headers(req, headersBuf, iHeadersLen);
	headersBuf[iHeadersLen] = '\0';

	int request_size = strlen(req->method) + strlen(req->path) + strlen(req->version) + iHeadersLen + 4;
	
	char *serverReq;

	serverReq = (char *) malloc(request_size + 1);

	if(serverReq == NULL){
		fprintf (stderr," Error in memory allocation for serverrequest ! \n");
		exit (1);
	}

	serverReq[0] = '\0';
	strcpy(serverReq, req->method);
	strcat(serverReq, " ");
	strcat(serverReq, req->path);
	strcat(serverReq, " ");
	strcat(serverReq, req->version);
	strcat(serverReq, "\r\n");
	strcat(serverReq, headersBuf);

	free(headersBuf);

	return serverReq;

}

int createserverSocket(char *pcAddress, char *pcPort) {
  struct addrinfo ahints;
  struct addrinfo *paRes;

  int iSockfd;

  /* Get address information for stream socket on input port */
  memset(&ahints, 0, sizeof(ahints));
  ahints.ai_family = AF_UNSPEC;
  ahints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(pcAddress, pcPort, &ahints, &paRes) != 0) {
   		fprintf (stderr," Error in server address format ! \n");
		exit (1);
  }

  /* Create and connect */
  if ((iSockfd = socket(paRes->ai_family, paRes->ai_socktype, paRes->ai_protocol)) < 0) {
    	fprintf (stderr," Error in creating socket to server ! \n");
		exit (1);
  }
  if (connect(iSockfd, paRes->ai_addr, paRes->ai_addrlen) < 0) {
    	fprintf (stderr," Error in connecting to server ! \n");
		exit (1);
	}

  /* Free paRes, which was dynamically allocated by getaddrinfo */
  freeaddrinfo(paRes);

  return iSockfd;
}

void writeToserverSocket(const char* buff_to_server,int sockfd,int buff_length)
{
	string temp; 

	temp.append(buff_to_server);
	
	int totalsent = 0;

	int senteach;

	while (totalsent < buff_length) {
		if ((senteach = send(sockfd, (void *) (buff_to_server + totalsent), buff_length - totalsent, 0)) < 0) {
			fprintf (stderr," Error in sending to server ! \n");
				exit (1);
		}
		totalsent += senteach;
	}	

}

void writeToclientSocket(const char* buff_to_server,int sockfd,int buff_length)
{
	string temp;

	temp.append(buff_to_server);

	int totalsent = 0;

	int senteach;

	while (totalsent < buff_length) {
		if ((senteach = send(sockfd, (void *) (buff_to_server + totalsent), buff_length - totalsent, 0)) < 0) {
			fprintf (stderr," Error in sending to server ! \n");
				exit (1);
		}
		totalsent += senteach;
	}	

}

void writeToClient (int Clientfd, int Serverfd) {
	int MAX_BUF_SIZE = 5000;

	int iRecv;
	char buf[MAX_BUF_SIZE];

	while ((iRecv = recv(Serverfd, buf, MAX_BUF_SIZE, 0)) > 0) {
	      writeToclientSocket(buf, Clientfd,iRecv);         // writing to client	    
		memset(buf,0,sizeof buf);	
	}      

	if (iRecv < 0) {
		fprintf (stderr,"Error while recieving from server, try again after waiting for a few seconds! \n");
	  exit (1);
	}
}
bool isPartOf(char *a, char *b){
   if(std::strstr(b,a) != NULL){    //Strstr says does b contain a
      return true;
   } 
   return false;
}

void* datafromclient(void* sockid, char* blacklist)
{
	
	int MAX_BUFFER_SIZE = 5000;

	char buf[MAX_BUFFER_SIZE];

	int newsockfd = *((int*)sockid);

	char *request_message;  // Get message from URL

	request_message = (char *) malloc(MAX_BUFFER_SIZE); 

	if (request_message == NULL) {
		fprintf (stderr," Error in memory allocation ! \n");
		exit (1);
	}	

	request_message[0] = '\0';

	int total_recieved_bits = 0;

	while (strstr(request_message, "\r\n\r\n") == NULL) {  // determines end of request

	  int recvd = recv(newsockfd, buf, MAX_BUFFER_SIZE, 0) ;

	  if(recvd < 0 ){
	  	fprintf (stderr," Error while recieving ! \n");
		exit (1);
	  				
	  }else if(recvd == 0) {
	  		break;
	  } else {

	  	total_recieved_bits += recvd;

	  	/* if total message size greater than our string size,double the string size */

	  	buf[recvd] = '\0';
	  	if (total_recieved_bits > MAX_BUFFER_SIZE) {
			MAX_BUFFER_SIZE *= 2;
			request_message = (char *) realloc(request_message, MAX_BUFFER_SIZE);
			if (request_message == NULL) {
				fprintf (stderr," Error in memory re-allocation ! \n");
				exit (1);
			}
		}


	  }

	  strcat(request_message, buf);

	}

	struct ParsedRequest *req;    // contains parsed request

	req = ParsedRequest_create();

	if (ParsedRequest_parse(req, request_message, strlen(request_message)) < 0) {		
		fprintf (stderr,"Error:400: Bad request:Only http/1.0 and get with headers are allowed, so please recheck by using telnet seperately! \n");
		exit(0);
	}

	if (req->port == NULL){             // if port is not mentioned in URL, we take default as 80 
		 req->port = (char *) "80";
		}
		char yo[100];
		strcpy(yo,"http://");
		strcat(yo,req->host);

		 if (isPartOf(yo, blacklist))
         {
            	printf("%s\n","blacklisted, redirecting to warning.or.kr" );
            	req->host = (char *) "warning.or.kr";
         }
         // else printf("%s\n","the page, you are seeking is fine, enjoy!" );  
		
	char*  browser_req  = convert_Request_to_string(req);	
	
		
	int iServerfd;

	iServerfd = createserverSocket(req->host, req->port);

	writeToserverSocket(browser_req, iServerfd, total_recieved_bits);
	writeToClient(newsockfd, iServerfd);


	ParsedRequest_destroy(req);
		
	close(newsockfd);   // close the sockets
	close(iServerfd);


	int y = 3;
	int *p = &y;
	return p;

}

int main (int argc, char *argv[]) 
{	
	char str[MAX];
	// bool black_or_not;
	char blacklist[MAX];
	strcpy(blacklist,"");
	// int i =0;
	if ((fseek(stdin, 0, SEEK_END), ftell(stdin)) > 0)
	{
  		rewind(stdin);
  		fgets(str, sizeof str, stdin); 
        strcat(blacklist,str);       
    }
 
	int sockfd,newsockfd;

	struct sockaddr_in serv_addr; 
	struct sockaddr cli_addr;


	if (argc<2) 
  	{
  		fprintf (stderr,"Usage: ./proxy <port_number>\n");
  		return 1;
  	}
  	
  	sockfd = socket(AF_INET, SOCK_STREAM, 0);   // create a socket

  	if (sockfd<0) {
  		fprintf (stderr,"SORRY! Cannot create a socket, Try again ! \n");
  		return 1;
  	}

  	memset(&serv_addr,0,sizeof serv_addr);

  	int  portno = atoi(argv[1]);           // argument through terminal 
 	serv_addr.sin_family = AF_INET;     // ip4 family 
  	serv_addr.sin_addr.s_addr = INADDR_ANY;  // represents for localhost i.e 127.0.0.1 
 	serv_addr.sin_port = htons(portno); 


 	int binded = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

 	if (binded <0 ) {
 		fprintf (stderr,"Error on binding! Try with a different port_number \n");
  		return 1;
 	}
  	
  	listen(sockfd, 100);  // can have maximum of 100 browser requests

  	int clilen = sizeof(struct sockaddr);


  	while(1) {

  		newsockfd = accept(sockfd,&cli_addr, (socklen_t*) &clilen); 

  		if (newsockfd <0){
  			fprintf(stderr, "ERROR! On Accepting Request ! i.e requests limit crossed \n");
 		}

 		int pid = fork();

 		if(pid == 0){

 			 datafromclient((void*)&newsockfd, blacklist);
 			close(newsockfd);
 			_exit(0);
 		}else{
 			close(newsockfd); 
 		}
 	}
 	close(sockfd);


	return 0;
}