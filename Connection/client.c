#include "headers.h"

int joinSS(struct ssDetails ss)
{
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		handleNetworkErrors("socket");
		exit(0);
	}
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ss.ip);
	servaddr.sin_port = htons(ss.cliPort);

	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		handleNetworkErrors("connect");
		exit(0);
	}
	return sockfd;
}

int sendRequest(char *input, int sockfd)
{
	// send request
	char *arg_arr[3];
	parse_input(arg_arr, input);
	char *request_command = arg_arr[0];

	// privileged
	if (strcmp(request_command, "COPYDIR") == 0 || strcmp(request_command, "COPYFILE") == 0 ||
		strcmp(request_command, "MKDIR") == 0 || strcmp(request_command, "MKFILE") == 0 || strcmp(request_command, "RMFILE") == 0 || strcmp(request_command, "RMDIR") == 0)
	{
		int bytesSent = send(sockfd, input, strlen(input), 0);
		if (bytesSent == -1)
		{
			handleNetworkErrors("send");
		}
		if (bytesSent == 0)
		{
			// nm has disconnected
			return -1;
		}

		// recieve ack
		char ackMsg[4096];
		bzero(ackMsg, sizeof(ackMsg));
		int bytesRecv = recv(sockfd, ackMsg, sizeof(ackMsg), 0);
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
		}

		// only for output formatting
		if (strcmp(request_command, "MKDIR") == 0)
		{
			if (strcmp(ackMsg, "SUCCESS") == 0)
			{
				printf(GREEN_COLOR "Directory %s successfully created\n" RESET_COLOR, arg_arr[1]);
				return 0;
			}
			else
			{
				handleAllErrors(ackMsg);
				printf(RED_COLOR "Directory %s could not be created\n" RESET_COLOR, arg_arr[1]);
				return -2;
			}
		}
		else if (strcmp(request_command, "MKFILE") == 0)
		{
			if (strcmp(ackMsg, "SUCCESS") == 0)
			{
				printf(GREEN_COLOR "File %s successfully created\n" RESET_COLOR, arg_arr[1]);
				return 0;
			}
			else
			{
				handleAllErrors(ackMsg);
				printf(RED_COLOR "File %s could not be created\n" RESET_COLOR, arg_arr[1]);
				return -2;
			}
		}
		else if (strcmp(request_command, "RMDIR") == 0)
		{
			if (strcmp(ackMsg, "SUCCESS") == 0)
			{
				printf(GREEN_COLOR "Directory %s successfully removed\n" RESET_COLOR, arg_arr[1]);
				return 0;
			}
			else
			{
				handleAllErrors(ackMsg);
				printf(RED_COLOR "Directory %s could not be removed\n" RESET_COLOR, arg_arr[1]);
				return -2;
			}
		}
		else if (strcmp(request_command, "RMFILE") == 0)
		{
			if (strcmp(ackMsg, "SUCCESS") == 0)
			{
				printf(GREEN_COLOR "File %s successfully removed\n" RESET_COLOR, arg_arr[1]);
				return 0;
			}
			else
			{
				handleAllErrors(ackMsg);
				printf(RED_COLOR "File %s could not be removed\n" RESET_COLOR, arg_arr[1]);
				return -2;
			}
		}else if (strcmp(request_command, "COPYDIR") == 0)
		{
			if (strcmp(ackMsg, "SUCCESS") == 0)
			{
				printf(GREEN_COLOR "Directory %s successfully copied\n" RESET_COLOR, arg_arr[1]);
				return 0;
			}
			else
			{
				handleAllErrors(ackMsg);
				printf(RED_COLOR "Directory %s could not be copied\n" RESET_COLOR, arg_arr[1]);
				return -2;
			}
		}
	}
	else
	{
		printf(RED_COLOR "Invalid Input!\n" RESET_COLOR);
		return 0;
	}

	// // recieve the storage server details
	// struct ssDetails ss;
	// int bytesRecv = recv(sockfd, &ss, sizeof(ss), 0);
	// if ((ss.id <= 0) || bytesRecv == 0 || (bytesRecv == -1 && errno == ECONNRESET))
	// {
	// 	// nm has disconnected
	// 	return -1;
	// }
	// if (bytesRecv == -1)
	// {
	// 	handleNetworkErrors("recv");
	// 	return 0;
	// }

	// printf("Recieved from NM - SS %s:%d\n", ss.ip, ss.cliPort);

	// int connfd = joinSS(ss);
	// bytesSent = send(connfd, request, sizeof(request), 0);
	// if (bytesSent == -1)
	// {
	// 	handleNetworkErrors("recv");
	// }
	// printf("\'%s\' sent to SS %s:%d\n", request, ss.ip, ss.cliPort);
	return 0;
}

int joinNamingServerAsClient(char *ip, int port)
{
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		// perror("socket");
		handleNetworkErrors("socket");
		exit(0);
	}
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);

	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		// perror("connect");
		handleNetworkErrors("connect");
		exit(0);
	}

	char joinMsg[4096] = "JOIN_AS Client";
	int bytesSent = send(sockfd, joinMsg, sizeof(joinMsg), 0);
	if (bytesSent == -1)
	{
		handleNetworkErrors("send");
	}

	char buffer[4096];
	int bytesRecv = recv(sockfd, buffer, sizeof(buffer), 0);
	if (bytesRecv == -1)
	{
		handleNetworkErrors("recv");
	}
	if (strcmp(buffer, "ACCEPTED JOIN") != 0)
	{
		return -1;
	}
	// printf("Client connected to naming server.\n");

	return sockfd;
}

void *isNMConnected(void *args)
{
	int sockfd = *(int *)(args);
	while (1)
	{
		sleep(1);
		int error;
		socklen_t len = sizeof(error);
		int ret = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);

		if (ret == 0 && error == 0)
		{
			sleep(1);
			continue;
		}
		else
		{
			printf(RED_COLOR "[-] Naming Server has disconnected!\n" RESET_COLOR);
			exit(0);
		}
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf(RED_COLOR "[-] Invalid Arguments!\n" RESET_COLOR);
		exit(0);
	}

	int namingServerPort = atoi(argv[1]);
	// int namingServerPort = 6969;
	int sockfd = joinNamingServerAsClient("127.0.0.1", namingServerPort);
	printf(GREEN_COLOR "[+] Client connected to Naming Server\n" RESET_COLOR);

	pthread_t disconnectionThread;
	pthread_create(&disconnectionThread, NULL, isNMConnected, &sockfd);

	while (1)
	{
		// char input[4096];
		char *input = malloc(sizeof(char) * 4096);
		if (fgets(input, 4096, stdin) == NULL)
		{
			break;
		}

		if (input[strlen(input) - 1] == '\n')
		{
			input[strlen(input) - 1] = '\0'; // removing the \n char
		}

		// printf(".%ld.%s.\n", strlen(input), input);
		int status = sendRequest(input, sockfd);
		if (status == -1)
		{
			printf(RED_COLOR "[-] Naming Server has disconnected!\n" RESET_COLOR);
			exit(0);
			break;
		}

		free(input);
	}
	pthread_join(disconnectionThread, NULL);
	return 0;
}
