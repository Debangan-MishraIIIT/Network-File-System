#include "headers.h"

int main()
{
	int sockfd= client_establish_connection();
	send_file("file.txt", sockfd);
	close(sockfd);
}
