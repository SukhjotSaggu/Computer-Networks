/* A simple echo server using TCP */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#define SERVER_TCP_PORT 3000	/* well-known port */
#define BUFLEN		256	/* buffer length */

int echod(int);
void reaper(int);

int main(int argc, char **argv)
{
	int 	sd, new_sd, client_len, port;
	struct	sockaddr_in server, client;

	switch(argc){
	case 1:
		port = SERVER_TCP_PORT;
		break;
	case 2:
		port = atoi(argv[1]);
		break;
	default:
		fprintf(stderr, "Usage: %d [port]\n", argv[0]);
		exit(1);
	}

	/* Create a stream socket	*/	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	/* Bind an address to the socket	*/
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	/* queue up to 5 connect requests  */
	listen(sd, 5);

	(void) signal(SIGCHLD, reaper);

	while(1) {
	  client_len = sizeof(client);
	  new_sd = accept(sd, (struct sockaddr *)&client, &client_len);
	  if(new_sd < 0){
	    fprintf(stderr, "Can't accept client \n");
	    exit(1);
	  }
	  switch (fork()){
	  case 0:		/* child */
		(void) close(sd);
		exit(echod(new_sd));
	  default:		/* parent */
		(void) close(new_sd);
		break;
	  case -1:
		fprintf(stderr, "fork: error\n");
	  }
	}
}

/*	echod program	*/
int echod(int sd)
{
	char	*bp, buf[BUFLEN], fbuf[BUFLEN], ch, ebuf[BUFLEN] = "ERROR: FILE NOT FOUND", ackbuf[BUFLEN] = "FILE FOUND";
	int 	n, bytes_to_read, size, i;
	FILE	*file;

		read(sd, buf, BUFLEN); //Read file name fron client
		buf[strcspn(buf, "\n")] = 0; //Remove '\n'

		file = fopen(buf,"r"); //Open the requested file
		if (file != NULL){ // If the file exists then proceed 
			write(sd, ackbuf, BUFLEN); // Let client know the file has been forund
			fseek(file, 0L, SEEK_END);
			size = ftell(file); // Find the size of the file
			write(sd, &size, sizeof(int)); //Send the size of the file to client
			rewind(file);

			while ((i = (fread(fbuf, 1, 100, file)))!=0){ //Send 100 byte packets to the client
				write(sd, fbuf, i);
			}

			fclose(file); //Close file
			close(sd); //Close socket
			return 0;

		} else{	// If the file does not exist then send error to client
			write(sd, ebuf,BUFLEN);
			printf("file not found \n"); // Print error to terminal
			close(sd); //Close socket
			return 0;	
		}

	close(sd);
	return(0);
}

/*	reaper		*/
void	reaper(int sig)
{
	int	status;
	while(wait3(&status, WNOHANG, (struct rusage *)0) >= 0);
	exit(1);
}