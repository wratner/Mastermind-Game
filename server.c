/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define PORT "7806"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

char * mastermind(char * sec, char * guess, char * reply)
{

	int i = 0;
	int slot_counter = 0;
	int color_counter = 0;
	int seen [4] = {-1,-1,-1,-1};
	
	//check first color
	if (((guess[0] == sec[1]) || (guess[0] == sec[2]) || (guess[0] == sec[3]))) {
		for(i = 0; i < 4; i++) {
			if ((guess[0] == sec[i]) && (seen[i] == -1)) {
				seen[i] = 1;
				break;
			}
		}
	}
	//check second color
	if (((guess[1] == sec[0]) || (guess[1] == sec[2]) || (guess[1] == sec[3]))) {
		if ((guess[1] == sec[0]) && (seen[0] == -1)) {
			seen[0] = 1;
		}
		else if ((guess[1] == sec[1]) && (seen[0] == -1)) {
			seen[1] = 1;
		}
		else {
			for(i = 2; i < 4; i++) {
				if ((guess[1] == sec[i]) && (seen[i] == -1)) {
					seen[i] = 1;
					break;
				}
			}
		}
	}
	//check third color
	if (((guess[2] == sec[0]) || (guess[2] == sec[1]) || (guess[2] == sec[3]))) {
		if ((guess[2] == sec[0]) && (seen[0] == -1)) {
			seen[0] = 1;
		}
		else if((guess[2] == sec[1]) && (seen[1] == -1)) {
			seen[1] = 1;
		}
		else if((guess[2] == sec[2]) && (seen[2] == -1)) {
			seen[2] = 1;
		}
		else if((guess[2] == sec[3]) && (seen[3] == -1)) {
			seen[3] = 1;
		}
	}
	//check fourth color
	if (((guess[3] == sec[0]) || (guess[3] == sec[1]) || (guess[3] == sec[2]))) {
		if ((guess[3] == sec[0]) && (seen[0] == -1)) {
			seen[0] = 1;
		}
		else if((guess[3] == sec[1]) && (seen[1] == -1)) {
			seen[1] = 1;
		}
		else if((guess[3] == sec[2]) && (seen[2] == -1)) {
			seen[2] = 1;
		}
		else if((guess[3] == sec[3]) && (seen[3] == -1)) {
			seen[3] = 1;
		}
	}
	//check for correct color and slot
	for (i= 0; i < 4; i++) {
		if (sec[i] == guess[i]) {
			slot_counter++;
			seen[i] = -1;
		}
	}
	//tally up number of correct colors
	for (i = 0; i < 4; i++) {
		if (seen[i] != -1) {
			color_counter++;
		}
	}
	sprintf(reply, "%d correct color+slot, %d correct colors", slot_counter, color_counter);
	if (slot_counter == 4) {
		reply = "You Win!";
	}
	return reply;
}

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

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
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	char choice[5];
	char secret[5];
	int num_guess = 0;
	int end_game = 0;


	int i = 0;

	if (argc != 2) {
		fprintf(stderr,"usage: server port\n");
		exit(1);
	}

	char * port = argv[1];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	printf("Enter <random> or <four digit number>: ");
	scanf("%s", choice);

	if (strcmp(choice, "random") == 0) {
		srand(time(NULL));
		for (i=0; i <4; ++i)
			secret[i] = (char)(((int)'0')+rand() % 6);
		secret[4] = '\0';
		//printf("%s", secret);
	}
	else {
		for (i = 0; i < 4; i++) 
			secret[i] = choice[i];
		secret[4] = '\0';
	}
	

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
			fprintf(stderr, "server: failed to bind\n");
			return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	int numbytes = 0;
	char buf[100];
	char temp[100];
	char * msg;
	printf("server: waiting for connections...\n");

	
	while(1) {

		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			printf("problem here");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);
		if (!fork()) {
			close(sockfd);
			while(end_game == 0) {
				//recieve the guesses
				if ((numbytes = recv(new_fd, buf, 1024,0))== -1) {
					perror("recv");
					exit(1);
				}
				//check if client shut down before end of game
				else if (numbytes == 0) {
					printf("Connection closed\n");
					break;
				}
				buf[numbytes] = '\0';
				msg = mastermind(secret, buf, temp); //perform game logic
				num_guess++;
				if ((strcmp(msg, "You Win!") == 0)) //if win, send appropriate message and close
				{
					end_game = 1;
					send(new_fd, msg, strlen(msg), 0);
					close(new_fd);
					return 0;
				}
				if (num_guess == 8) { //if lose, send appropriate message and close
					sprintf(msg, "You Lose! The correct answer was: %s", secret);
					end_game = 1;
					send(new_fd, msg, strlen(msg), 0);
					close(new_fd);
					return 0;
				}
				//if neither win or lose, send hint
				if ((send(new_fd,msg, strlen(msg),0))== -1) 
				{
					fprintf(stderr, "Failure Sending Message\n");
					close(new_fd);
					break;
				}
        	} 

    	}
        close(new_fd);
    } 

    return 0;
}

