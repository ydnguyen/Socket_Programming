#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string.h>

#define SERVER_PORT 5432
#define MAX_LINE 1024

int main (int argc, char *argv[]) {
	
	FILE *fp;
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host;
	char buf[MAX_LINE];
	char readBuf[MAX_LINE];
	int s,new_s;
	int len;
	
	//check the number of arguments (has to be 2)
	if (argc==2) {
		//2nd argument will be the host server we're trying to reach
		host = argv[1];
	}
	else {
		//print an error if more than 2 arguments given
		fprintf(stderr, "usage: simplex-talk host\n");
	}
	
	//resolve host name into IP address; return null pointer if error occurs
	hp = gethostbyname(host);
	if (!hp) {
		fprintf(stderr, "simplex-talk: unknown host: %s \n", host);
		exit(1);
	}
	
	//build address data structure for receiving host
	//initialize all bytes of receiving address to 0
	bzero((char *)&sin, sizeof(sin));
	//set sin_family to AF_INET to connect to the Internet
	sin.sin_family = AF_INET;
	//copy the bytes of resolved IP address (of given host) to the address data structure
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	//converts SERVER_PORT to network byte order; assign it to sin_port
	sin.sin_port = htons(SERVER_PORT);
	
	//active open 
	//create an endpoint for communication using socket() system call; will return -1 if an error occurs
	//SOCK_DGRAM used to implement UDP
	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("simplex-talk: socket");
		exit(1);
	}
	
	//initiate connection on a socket using connect() system call; will return -1 if an error occurs
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("simplex-talk: connect");
		exit(1);
	}
	
	//get size of socket
	int sin_size = sizeof(sin);
	//intialize length of received message from server, in bytes
	int reclen = 0;

	//once socket connection is established, get and send lines of text using a while loop
	while (fgets(buf, sizeof(buf), stdin)) {
		//delete last char of message, which is new line char
		buf[MAX_LINE-1] = '\0';
		//length of message sent is 1 more than actual length of message
		len = strlen(buf) + 1;
		//send string buf on socket using send() system call
		sendto(s, buf, len, 0, (struct sockaddr *)&sin, sizeof(sin));
		//receive message from server
		reclen=recvfrom(s, readBuf, sizeof(readBuf), 0, 0, 0);
		//delete last char of received message, which is new line char
		readBuf[reclen-1]='\0';
		//print the message from server
		printf("%s\n", readBuf);
		 
	}
	//close socket after communication
	close(s);

}
