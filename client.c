/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "7806" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	char * win = "You Win!";
	char * lose = "You Lose!";

	//fprintf(stderr, "argc = %d", argc);
	//fprintf(stderr, "argv = %s", argv[1]);
	if (argc != 3) {
	    fprintf(stderr,"usage: client hostname port\n");
	    exit(1);
	}

	char * port = argv[2];
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("Client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	while(1) {
        printf("Client: Enter Your Guess!: ");
        fgets(buf,100-1,stdin);
        //send guess to server
        if ((send(sockfd,buf, strlen(buf),0))== -1) { 
                fprintf(stderr, "Failure Sending Message\n");
                close(sockfd);
                exit(1);
        }
        else {
                //if not sending, recieve answer from server
                numbytes = recv(sockfd, buf, sizeof(buf),0);
                if ( numbytes <= 0 )
                {
                        printf("Either Connection Closed or Error\n");
                        exit(1);
                }
                buf[numbytes] = '\0';
                //if you lose, print result and end connection
                if ((strncmp(buf, lose, 8) == 0)) {
                	printf("%s\n", buf);
                	break;
                }        
                //if you win, print result and end connection
                if ((strcmp(buf, win) == 0)) { 
                	printf("%s\n", buf);
                	break;         
                }
                //otherwise print out the hint from server
                printf("Client: Hint: %s\n",buf);
                
           }
    }

    return 0;
}

