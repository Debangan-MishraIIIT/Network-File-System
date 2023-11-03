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
		printf("RECV: %s\n", buffer);
	}
	return NULL;
}

int main()
{
	int namingServerPort = 6969;
	int sockfd = joinNamingServer("127.0.0.1", namingServerPort, "Storage Server");

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
