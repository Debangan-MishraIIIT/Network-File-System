#include "headers.h"

int main()
{
	int namingServerPort = 6969;
	int sockfd = joinNamingServer("127.0.0.1", namingServerPort, "Client");
	return 0;
}

// int main()
// {
// 	int sockfd= client_establish_connection(8080, "127.0.0.1");
// 	send_file("file.txt", sockfd);
// 	close(sockfd);
// }
