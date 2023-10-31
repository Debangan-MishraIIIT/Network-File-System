#include "headers.h"

int main() 
{ 
	struct pair p= server_establish_connection(8080);
	int sockfd= p.first;
	int connfd= p.second;

	receive_file(connfd); 
	close(sockfd); 
}
