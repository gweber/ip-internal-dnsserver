#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


int debug = 0;


int do_request(char *req_server, char *request) {
	int pos, labelpos, i, req_length = 0;

	int sockfd, bytesReceived;
	struct sockaddr_in serverAddr, clientAddr;
	struct hostent *hp;
	socklen_t len;

	unsigned char *buffer;
	u_int16_t rid = (u_int16_t) rand();

	if (debug) {
		printf("server: %s :: request %s\n", req_server, request);
	}

	req_length = strlen(request);
	// 12 header + 6 (1b label + %00 + 2b class +2b type) + strlen
	buffer = (unsigned char *) calloc( 18 + req_length, sizeof(char));
	// 0 .. 1 are the id
	memcpy(&buffer[0], &rid, 2);
	// 2 .. 3 are the flags
	//buffer[2] = 0;
	// rd 1
	buffer[2] |= 1;
	// 4 .. 5  qdcount - 1 question
	buffer[5] = 1;
	// 6 .. 7  ancount
	// 8 .. 9  nscount
	// 10..11  arcount
	// init first label
	labelpos = 12;
	buffer[labelpos] = 1;
	// 12 is first label, so copy the request to 13
	memcpy(&buffer[13], request, req_length);
	for (pos = 13; pos < 13 + req_length + 1; pos++) {
		if (buffer[pos] == 46 || buffer[pos] == 0 ) {
			buffer[labelpos] = pos - labelpos - 1;
			labelpos = pos;
		}
	}
	// type, 1 = A
	buffer[pos + 1] = 1;
	// class, 1 = INternet
	pos = pos + 2;
	buffer[pos + 1] = 1;
	pos = pos + 2 ;

	if (debug) {
		for (i = 0; i < pos; i++) {
			printf("\\x%02x", buffer[i]);
			if ( !((i + 1) % 16)) {
				printf("\n");
			} else if ( !((i + 1) % 4)) {
				printf(" ");
			}
		}
	}

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	/* client side */
	bzero((char *) &clientAddr, sizeof(clientAddr));
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	clientAddr.sin_port = htons(0); // random port

	if (bind(sockfd, (struct sockaddr *) &clientAddr, sizeof(clientAddr)) < 0)//check UDP socket is bind correctly
	{
		perror("Cannot bind ");
		close(sockfd);
		return -4;
	}

	bzero((char *) &serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(req_server);
	serverAddr.sin_port = htons(53);

	int n;
	char recv_buffer[1024];
	ssize_t send_result;

	send_result = sendto(sockfd, (const char *)buffer, pos, 0, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	n = recvfrom(sockfd, recv_buffer, 1024, 0, (struct sockaddr *) &clientAddr, &len);
	recv_buffer[n] = '\0';

	if (debug) {
		for (i = 0; i < n; i++) {
			printf("\\x%02x", recv_buffer[i]);
			if ( !((i + 1) % 16)) {
				printf("\n");
			} else if ( !((i + 1) % 4)) {
				printf(" ");
			}
		}
	}

	//printf("%s\n", req_server);
	//

	printf("%s %lu\n", request, strlen(request));

	free(buffer);
	close(sockfd);

	return 0;
}




int main (int argc, char **argv) {

	char *cvalue = NULL;
	char *server = NULL;
	char *request = NULL;
	int index;
	int c;
	int req_length = 0;
	int i = 0;
	int pos, labelpos = 0;
	int loop = 0;


	opterr = 0;
	if (argc < 2){
		fprintf (stderr, "usage: %s request\n", argv[0]);
		abort ();
	}

	while ((c = getopt (argc, argv, "lds:")) != -1)
		switch (c) {
		case 's':
			server = optarg;
			break;
		case 'd':
			debug = 1;
			break;
		case 'l':
			loop = atoi(optarg);
			break;
		case '?':
			if (optopt == 's')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr,
				         "Unknown option character `\\x%x'.\n",
				         optopt);
			return 1;
		default:
			fprintf (stderr, "usage: %s request\n", argv[0]);
			abort ();
		}

	for (index = optind; index < argc; index++)
		request = argv[index];

	//request = "10.10.10.10.ip.example.com";
	//request = "www.google.com";
	if (debug) {
		printf("size of server %lu\n", strlen(server));
	}

	// TODO: check the server set
	if (strlen(server) < 1) {
		if (debug) {
			printf("problem with server, setting localhost\n");
		    server = "127.0.0.1";
		}
	}
	//server = "10.24.193.3";

	printf ("server: %s :: request: %s %lu\n", server, request, strlen(request));

	for (i = 0; i < loop; i++) {
		do_request(server, request);
		// Creating socket file descriptor

		//printf("request send %u\n", (int) send_result);

		//printf("response recv %u\n", (int) n);

		// printf("server sent : %d %s\n",n, recv_buffer);



	}


	return 0;
}
