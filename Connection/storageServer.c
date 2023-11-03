#include "headers.h"

void *serveNM_Requests(void *args)
{
	int nmfd = *((int *)args);
	while (1)
	{
		char buffer[4096];
		int bytesRecv = recv(nmfd, buffer, sizeof(buffer), 0);
		if (bytesRecv == -1)
		{
			perror("recv");
		}
		printf("Recieved from NM: %s\n", buffer);
	}
	return NULL;
}

void *serveClient_Requests(void *args)
{
	return NULL;
}

int initializeNMConnection(char *ip, int port, int nmPort, int cliPort)
{
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		perror("socket");
		exit(0);
	}
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);

	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr.s_addr = inet_addr(ip);
	cliaddr.sin_port = htons(nmPort);

	if ((bind(sockfd, (SA *)&cliaddr, sizeof(cliaddr))) != 0)
	{
		perror("bind");
		exit(0);
	}

	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		perror("connect");
		exit(0);
	}

	char joinMsg[4096] = "JOIN_AS Storage Server";
	int bytesSent = send(sockfd, joinMsg, sizeof(joinMsg), 0);
	if (bytesSent == -1)
	{
		perror("send");
	}
	printf("SS connected to naming server.\n");

	// initialize detials
	struct ssDetails details;

	socklen_t addrlen = sizeof(cliaddr);
	if (getpeername(sockfd, (struct sockaddr *)&cliaddr, &addrlen) == 0)
	{
		inet_ntop(AF_INET, &cliaddr.sin_addr, details.ip, INET_ADDRSTRLEN);
	}
	details.nmPort = nmPort;
	details.cliPort = cliPort;

	bytesSent = send(sockfd, &details, sizeof(details), 0);
	if (bytesSent == -1)
	{
		perror("send");
	}
	printf("SS detials sent to NM.\n");

	return sockfd;
}

int initialzeClientsConnection(int port)
{
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1)
	{
		perror("socket");
		exit(0);
	}
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
	{
		perror("bind");
		exit(0);
	}
	return sockfd;
}

int main()
{
	int nmPort = 6970;
	int cliPort = 6971;
	initialzeClientsConnection(cliPort);
	int sockfd = initializeNMConnection("127.0.0.1", 6969, nmPort, cliPort);
	pthread_t nmThread;
	pthread_create(&nmThread, NULL, serveNM_Requests, (void *)&sockfd);
	pthread_join(nmThread, NULL);
	return 0;
}

// int main()
// {
// 	int sockfd= client_establish_connection(8080, "127.0.0.1");
// 	send_file("file.txt", sockfd);
// 	close(sockfd);
// }
