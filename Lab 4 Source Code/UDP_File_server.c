/* time_server.c - main */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>


/*------------------------------------------------------------------------
 * main - Iterative UDP server for TIME service
 *------------------------------------------------------------------------
 */

// PDU struct that contains the type and message
struct pdu{
	char type;
	char data[100];
};

int
main(int argc, char *argv[])
{
	struct  sockaddr_in fsin;	/* the from address of a client	*/
	char	buf[100];		/* "input" buffer; any size > 0	*/
	char    *pts;
	int	sock;			/* server socket		*/
	time_t	now;			/* current time			*/
	int	alen;			/* from-address length		*/
	struct  sockaddr_in sin; /* an Internet endpoint address         */
        int     s, type;        /* socket descriptor and socket type    */
	int 	port=3000;
                                                                        
	switch(argc){
		case 1:
			break;
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
	}

        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port);
                                                                                                 
    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "can't creat socket\n");
                                                                                
    /* Bind the socket */
        if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "can't bind to %d port\n",port);
        listen(s, 5);	
	alen = sizeof(fsin);

	
	struct pdu pduIn;
	struct pdu pduOut;
	FILE *file;
	int msgLength, size;

	while (1) {
		if (recvfrom(s, &pduIn, sizeof(pduIn), 0,(struct sockaddr *)&fsin, &alen) < 0)
			fprintf(stderr, "recvfrom error\n");

		//Remove /n from in front of the file name
		pduIn.data[strcspn(pduIn.data, "\n")] = 0;

		//Find an open the requested file
		file = fopen(pduIn.data,"r");
		//If the file exists then continue
		if (file != NULL){
			//Clear pduIn data and type
			memset(pduIn.data, '\0', 100);
			pduIn.type = '\0';
			//GO through the file to find out the number of characters in it
			fseek(file, 0L, SEEK_END);
			//Store the size of file
			size = ftell(file);
			//GO back to the start of the file
			rewind(file);
			//Set the pdu type to 'D' since it is the data in the file
			pduOut.type = 'D';
			//Keep track of the number of bytes that have been read
			msgLength = 0;
			//Read file and set message in pduOut data
			while (fgets(pduOut.data, 101, file) != NULL){
				//Add the number of bytes sent out to client to the msgLength counter
				msgLength += strlen(pduOut.data);
				//Once the msgLength counter has reached the size of the message in file
				//then set the final PDU type to 'F'
				if (msgLength == size)
					pduOut.type = 'F';
				//Send the PDU's to the client
				(void) sendto(s, &pduOut, sizeof(pduOut), 0, (struct sockaddr *)&fsin, sizeof(fsin));
				//Reset pduOut data
				memset(pduOut.data, '\0', 100);
			}
			//Close the file
			fclose(file);	
		}
		else{	
			//Set pduOut type to 'E' for error
			pduOut.type = 'E';
			//Set pduOut data to error message
			strcpy(pduOut.data, "File not found");
			//Send error PDU to client
			(void) sendto(s, &pduOut, sizeof(pduOut), 0, (struct sockaddr *)&fsin, sizeof(fsin));
			//Clear PDU data
			memset(pduOut.data, '\0', 100);
		}
	}
	return 0;
}