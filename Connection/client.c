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

	if (strcmp(request_command, "MKDIR") == 0 || strcmp(request_command, "MKFILE") == 0 || strcmp(request_command, "RMFILE") == 0 || strcmp(request_command, "RMDIR") == 0 || strcmp(request_command, "COPYDIR") == 0 || strcmp(request_command, "COPYFILE") == 0)
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

		// ack
		char buffer[1000];
		int bytesRecv = recv(sockfd, buffer, sizeof(buffer), 0);

		if (bytesRecv == -1)
		{
			handle_errors("recv");
		}
		// printf("Recieved from NM\n%s\n\n", buffer);
		if (strcmp(buffer, "SUCCESS IN EXECUTION") == 0)
		{
			printf(YELLOW_COLOR "File %s removed\n" RESET_COLOR, arg_arr[1]);
			return 0;
		}
		else
		{
			printf(ORANGE_COLOR "File %s not removed\n" RESET_COLOR, arg_arr[1]);
			return -2;
		}
	}
	else if (strcmp(request_command, "WRITE") == 0)
	{
		printf("%s %s\n", arg_arr[0], arg_arr[1]);
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

		char ssRequest[1000];
		strcpy(ssRequest, input);

		// recieve the storage server details
		struct ssDetails ss;
		int bytesRecv = recv(sockfd, &ss, sizeof(ss), 0);
		if ((ss.id <= 0) || bytesRecv == 0 || (bytesRecv == -1 && errno == ECONNRESET))
		{
			// nm has disconnected
			return -1;
		}
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
			return -2;
		}

		printf("Recieved from NM - SS %s:%d\n", ss.ip, ss.cliPort);

		int connfd = joinSS(ss);
		bytesSent = send(connfd, ssRequest, sizeof(ssRequest), 0);
		if (bytesSent == -1)
		{
			handleNetworkErrors("recv");
			return -2;
		}
		printf("\'%s\' sent to SS %s:%d\n", ssRequest, ss.ip, ss.cliPort);

		// receive the perms first
		char perms[11];
		bytesRecv = recv(connfd, perms, sizeof(perms), 0);
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
			return -2;
		}

		printf("permissions: %s\n", perms);
		if (perms[0] == 'd')
		{
			printf(RED "Requested File is directory\n" reset);
			return -2;
		}

		if (!receiveFile("temp.txt", connfd))
		{
			printf("Client: received the file from ss\n");
		}
		else
		{
			printf(RED "File not received\n" reset);
			return -2;
		}

		mode_t mode = reversePermissions(perms);

		if (chmod("temp.txt", mode) == 0)
		{
			printf("Permissions set successfully.\n");
		}
		else
		{
			perror("chmod");
			return -2;
		}

		char editor[1000];
		while (1)
		{
			printf("Choose Editor that you want:\n");
			printf("1. gedit\n");
			printf("2. vim\n");
			printf("3. nano\n");
			printf("4. none(user's personal editor)\n");
			scanf("%s", editor);
			if (!strcmp(editor, "gedit") || !strcmp(editor, "vim") || !strcmp(editor, "nano") || !strcmp(editor, "none"))
			{
				break;
			}
		}

		if (!strcmp(editor, "none"))
		{
			printf("User chose custome editor\n");
			return 0;
		}
		else
		{
			if (writeFile("temp.txt", editor) == -1)
			{
				printf("Invalid file saved in user directory\n");
				return -2;
			}
		}
		removeFile("temp.txt");
		return 0;
	}
	else if (strcmp(request_command, "READ") == 0)
	{
		printf("%s %s\n", arg_arr[0], arg_arr[1]);
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

		char ssRequest[1000];
		strcpy(ssRequest, input);

		// recieve the storage server details
		struct ssDetails ss;
		int bytesRecv = recv(sockfd, &ss, sizeof(ss), 0);
		if ((ss.id <= 0) || bytesRecv == 0 || (bytesRecv == -1 && errno == ECONNRESET))
		{
			// nm has disconnected
			return -1;
		}
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
			return -2;
		}

		printf("Recieved from NM - SS %s:%d\n", ss.ip, ss.cliPort);

		int connfd = joinSS(ss);
		bytesSent = send(connfd, ssRequest, sizeof(ssRequest), 0);
		if (bytesSent == -1)
		{
			handleNetworkErrors("recv");
			return -2;
		}
		printf("\'%s\' sent to SS %s:%d\n", ssRequest, ss.ip, ss.cliPort);

		// receive the perms first
		char perms[11];
		bytesRecv = recv(connfd, perms, sizeof(perms), 0);
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
			return -2;
		}

		printf("permissions: %s\n", perms);
		if (perms[0] == 'd')
		{
			printf(RED "Requested File is directory\n" reset);
			return -2;
		}

		// disabling write permissions while reading
		perms[2] = '-';

		if (!receiveFile("temp.txt", connfd))
		{
			printf("Client: received the file from ss\n");
		}
		else
		{
			printf(RED "File not received\n" reset);
			return -2;
		}

		mode_t mode = reversePermissions(perms);

		if (chmod("temp.txt", mode) == 0)
		{
			printf("Permissions set successfully.\n");
		}
		else
		{
			perror("chmod");
			return -2;
		}

		char editor[1000];
		while (1)
		{
			printf("Choose Editor that you want:\n");
			printf("1. gedit\n");
			printf("2. vim\n");
			printf("3. nano\n");
			printf("4. none(user's personal editor)\n");
			scanf("%s", editor);
			if (!strcmp(editor, "gedit") || !strcmp(editor, "vim") || !strcmp(editor, "nano") || !strcmp(editor, "none"))
			{
				break;
			}
		}

		if (!strcmp(editor, "none"))
		{
			printf("User chose custom editor\n");
			return 0;
		}
		else
		{
			if (readFile("temp.txt", editor) == -1)
			{
				printf("Invalid file saved in user directory\n");
				return -2;
			}
		}
		removeFile("temp.txt");
		return 0;
	}
	else if (strcmp(request_command, "FILEINFO") == 0)
	{
		printf("%s %s\n", arg_arr[0], arg_arr[1]);
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

		char ssRequest[1000];
		strcpy(ssRequest, input);

		// recieve the storage server details
		struct ssDetails ss;
		int bytesRecv = recv(sockfd, &ss, sizeof(ss), 0);
		if ((ss.id <= 0) || bytesRecv == 0 || (bytesRecv == -1 && errno == ECONNRESET))
		{
			// nm has disconnected
			return -1;
		}
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
			return -2;
		}

		printf("Recieved from NM - SS %s:%d\n", ss.ip, ss.cliPort);

		int connfd = joinSS(ss);
		bytesSent = send(connfd, ssRequest, sizeof(ssRequest), 0);
		if (bytesSent == -1)
		{
			handleNetworkErrors("recv");
			return -2;
		}
		printf("\'%s\' sent to SS %s:%d\n", ssRequest, ss.ip, ss.cliPort);

		struct fileDetails det;
		bzero(&det, sizeof(det));
		bytesRecv = recv(connfd, &det, sizeof(det), 0);
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
			return -2;
			// break;
		}
		printf("File Name: %s\n", det.fileName);
		printf("File path: %s\n", det.path);
		printf("File permissions: %s\n", det.perms);
		printf("File size: %d\n", det.size);
		printf("File last modified time: %s\n", ctime(&det.lastModifiedTime));
		printf("File last access time: %s\n", ctime(&det.lastAccessTime));
		return 0;
	}
	else
	{
		printf(RED_COLOR "Invalid Input!\n" RESET_COLOR);
		return 0;
	}
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

		if (strlen(input) == 1)
			continue;

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
		if (status == 0)
		{
			printf("Request Over\n");
		}
		if (status == -2)
		{
			printf("Request Failed\n");
		}

		free(input);
	}
	pthread_join(disconnectionThread, NULL);
	return 0;
}
