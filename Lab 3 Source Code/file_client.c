/* A simple echo client using TCP */
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>



#define SERVER_TCP_PORT 3000	/* well-known port */
#define BUFLEN		256	/* buffer length */

int main(int argc, char **argv)
{
	int 	n, i, bytes_to_read, size, sizeTemp;
	int 	sd, port;
	struct	hostent		*hp;
	struct	sockaddr_in server;
	char	*host, *bp, rbuf[BUFLEN], sbuf[BUFLEN], *fileName, errorCheck[BUFLEN];

	switch(argc){
	case 2:
		host = argv[1];
		port = SERVER_TCP_PORT;
		break;
	case 3:
		host = argv[1];
		port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
		exit(1);
	}

	/* Create a stream socket	*/	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (hp = gethostbyname(host)) 
	  bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	else if ( inet_aton(host, (struct in_addr *) &server.sin_addr) ){
	  fprintf(stderr, "Can't get server's address\n");
	  exit(1);
	}

	/* Connecting to the server */
	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
	  fprintf(stderr, "Can't connect \n");
	  exit(1);
	}

	printf("Enter file name: \n");
	while(n=read(0, sbuf, BUFLEN)){	/* get user message */
		fileName = sbuf; //Store file name in sbuf
        write(sd, sbuf, n);		/* send it out */
		read(sd, errorCheck, BUFLEN);//Check if there was any error with opening the file from server

		if(strcmp(errorCheck, "ERROR: FILE NOT FOUND") == 0){ //If the file is not found then print error
			printf("ERROR: FILE NOT FOUND\n");					//and close socket
			close(sd);
			return(0);
	  }

		read(sd, &size, sizeof(int)); //If there is no error then read what the
		bp = rbuf;					//size of the file is
		bytes_to_read = size;
      
		FILE *file;
		file = fopen(sbuf, "wb"); // Create a file with the file name

	while(i = read(sd,rbuf, BUFLEN)){ //Read contents of file
		fwrite(rbuf, 1, i, file); // Write contents of file in the new file
	}

	fclose(file); //Close the file
	close(sd); //Close the socket
	return(0);
	}
	
	close(sd);
	return(0);
}
