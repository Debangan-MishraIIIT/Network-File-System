#include "headers.h"

int main()
{
	int sockfd= client_establish_connection(8080, "127.0.0.1");
	send_file("README.md", sockfd);
	close(sockfd);
}
