#include "headers.h"

void sendRequest(char *request, int sockfd)
{
	// send request
	int bytesSent = send(sockfd, request, sizeof(request), 0);
	if (bytesSent == -1)
	{
		perror("send");
	}
	// recieve the storage server details
	struct hostDetails ss;
	int bytesRecv = recv(sockfd, &ss, sizeof(ss), 0);
	if (bytesRecv == -1)
	{
		perror("recv");
	}
	printf("SS details: %s:%d\n", ss.ip, ss.port);
}

int main()
{
	int namingServerPort = 6969;
	int sockfd = joinNamingServer("127.0.0.1", namingServerPort, "Client");
	while (1)
	{
		char *input = malloc(sizeof(char) * 4096);
		if (fgets(input, 4096, stdin) == NULL)
		{
			break;
		}
		// printf("%s", input);
		input[strlen(input) - 1] = '\0'; // removing the \n char
		sendRequest(input, sockfd);
		free(input);
	}

	return 0;
}

// int main()
// {
// 	int sockfd= client_establish_connection(8080, "127.0.0.1");
// 	send_file("file.txt", sockfd);
// 	close(sockfd);
// }
