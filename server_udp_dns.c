#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PORT 5432
#define MAX_PENDING 5
#define MAX_LINE 1024

int main() {
	struct sockaddr_in sin;
	char buf[MAX_LINE];
	int len;
	int s;
	//dns lookup table
	const char *dns[10][2] = {
		{"localhost", 		"127.0.0.1"},
		{"mail.foo.com", 	"192.168.210.4"},
		{"dns.foo.com", 	"192.168.210.2"},
		{"router.foo.com", 	"192.168.210.1"},
		{"pc1.foo.com", 	"192.168.210.5"},
		{"pc2.foo.com", 	"192.168.210.6"},
		{"mail2.foo.com", 	"192.168.210.3"},
		{"router2.foo.com", 	"192.168.210.7"}
	};

	//build address data structure with it as the receiving host
	//initialize all bytes of receiving address to 0
	bzero((char *)&sin, sizeof(sin));
	//set sin_family to AF_INET to connect to the Internet
	sin.sin_family = AF_INET;
	//accept connection from any incoming address
	sin.sin_addr.s_addr = INADDR_ANY;
	//set sin_port to its own port (5432)
	sin.sin_port = htons(SERVER_PORT);
	
	//setup passive open (for server)
	//create an endpoint for communication using socket() system call; will return -1 if an error occurs
	//SOCK_DGRAM used to implement UDP
	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("simplex-talk: socket");
		exit(1);
	}
	//assign a name to a socket using bind() system call; return 0 if successful, and return -1 for error
	if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
		perror("simplex-talk: bind");
		exit(1);
	}
	
	//flag var for dns lookup
	int found = 0;
	//result from dns lookup
	char result[64];
	for(;;) {
		struct sockaddr_in client; //client's IP address
		socklen_t client_len; //length of client's address
		memset( &client, 0, sizeof(client)); //initialize client's IP address
		client_len = sizeof(client);
		len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&client, &client_len); 
		//remove new line char from client's message for easier string comparison
		buf[strlen(buf)-1]='\0';
		int i = 0;
		int j = 0;
		for (; i<8; i++) {
			for (; j<2; j++) {
				//compare string given by client with dns table; 
				//if matches first column, return 2nd column, and vice versa;
				if (!strcmp(buf,dns[i][j]) & j==0) {
					//clear string of previous output
					strcpy(result,"");
					snprintf(result, 64, "%s\n", dns[i][1]);
					//set flag var to 1, and break out of loop
					found = 1; break;}
				if (!strcmp(buf,dns[i][j]) & j==1) {
					//clear string of previous output
					strcpy(result,"");
					snprintf(result, 64, "%s\n", dns[i][0]);
					found = 1; break;}
			}
			//reset column var for next row
			j=0;
			//break out of 2nd loop if flag var is 1
			if (found) {break;}
		}
		//if not found, flag var is 0, send message to client that entry is not found
		if (found == 0) {snprintf(result,64, "No such entry: %s\n", buf);}
		//reset flag var, ready for next lookup request
		else {found = 0;}
		//send response to client
		sendto(s, result, strlen(result), 0, (struct sockaddr *)&client, sizeof(client));
		//print message sent to client for debugging
		printf("Response sent to client: %s", result);
						
	}
}
