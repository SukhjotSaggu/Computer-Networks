/* time_client.c - main */

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>                                                                            
#include <netinet/in.h>
#include <arpa/inet.h>                                                         
#include <netdb.h>

#define	BUFSIZE 64
#define	MSG		"Any Message \n"


/*------------------------------------------------------------------------
 * main - UDP client for TIME service that prints the resulting time
 *------------------------------------------------------------------------
 */
// PDU struct that contains the type and message
struct pdu{
	char type;
	char data[100];
};

int main(int argc, char **argv)
{
	char	*host = "localhost";
	int	port = 3000;
	char	now[100];		/* 32-bit integer to hold time	*/ 
	struct hostent	*phe;	/* pointer to host information entry	*/
	struct sockaddr_in sin;	/* an Internet endpoint address		*/
	int	s, n, type;	/* socket descriptor and socket type	*/
	
	switch (argc) {
	case 1:
		break;
	case 2:
		host = argv[1];
	case 3:
		host = argv[1];
		port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "usage: UDPtime [host [port]]\n");
		exit(1);
	}

	memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;                                                                
        sin.sin_port = htons(port);
                                                                                        
    /* Map host name to IP address, allowing for dotted decimal */
        if ( phe = gethostbyname(host) ){
                memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
        }
        else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
		fprintf(stderr, "Can't get host entry \n");
                                                                                
    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "Can't create socket \n");
	
                                                                                
    /* Connect the socket */
        if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "Can't connect to %s %s \n", host, "Time");
	
	char input, fileName[100];
	struct pdu pduOut;
	struct pdu pduIn;
	FILE *file;

    while (1){ //Loop to keep client program running until the user exits
        input = '\0'; //Clear user input varable before reading
    	printf("To download a file enter [D] or to exit enter [E]?\n");
    	scanf(" %c", &input);//Read user input
		
    	if (input == 'E')
        	break;
    	else{
			printf("Enter file name: \n");
        	pduOut.type = 'C';
			n=read(0, pduOut.data, 100);	/* get user message */
			write(s, &pduOut,sizeof(pduOut));
			//Place file name in pduOut data
    		pduOut.data[strcspn(pduOut.data,"\n")] = 0;
			//Store file name in 'fileName' variaable
    		strcpy(fileName, pduOut.data);
			//Display file name
 			printf("The file name: %s\n", fileName);
			//Reset pduOut data field
			memset(pduOut.data, '\0', 100);
			//Reset pduOut tyoe field
			pduOut.type = '\0';

			//Read the contents of the file from the file server
			read(s, &pduIn, sizeof(pduIn));
			
			//Display error if there is an error retriving the file from the server
			if (pduIn.type == 'E')
				write(1, "\nError Retriving the file from server\n", 39);
			//Otherwise read the file
			else{
				file = fopen(fileName, "w"); //Open file to write the message
				//Continuous while loop until break
				while (1){
					if (pduIn.type == 'D'){
						//Display the message
						printf("\n%s\n", pduIn.data);
						//Write message to the file
						fprintf(file, "%s", pduIn.data);
						//Reset pduIn data and type
						memset(pduIn.data, '\0', 100);
						pduIn.type = '\0';
						//Read next inbound PDU from server
						read(s, &pduIn, sizeof(pduIn));
					} 
					else if (pduIn.type == 'F'){
						printf("\n%s\n", pduIn.data);
						//Write final message to file
						fprintf(file, "%s", pduIn.data);
						//Close file
						fclose(file);
						break;
					}	
				}
			}	
    	}	
    }
	exit(0);
}