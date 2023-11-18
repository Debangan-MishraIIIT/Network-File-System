#include "headers.h"
// there should be an inifinite thread in the NS side that also checks and updates the paths
int nmSock1;
void convertPermissions(mode_t st_mode, char *perms)
{
	perms[0] = (S_ISDIR(st_mode)) ? 'd' : '-';
	perms[1] = (st_mode & S_IRUSR) ? 'r' : '-';
	perms[2] = (st_mode & S_IWUSR) ? 'w' : '-';
	perms[3] = (st_mode & S_IXUSR) ? 'x' : '-';
	perms[4] = (st_mode & S_IRGRP) ? 'r' : '-';
	perms[5] = (st_mode & S_IWGRP) ? 'w' : '-';
	perms[6] = (st_mode & S_IXGRP) ? 'x' : '-';
	perms[7] = (st_mode & S_IROTH) ? 'r' : '-';
	perms[8] = (st_mode & S_IWOTH) ? 'w' : '-';
	perms[9] = (st_mode & S_IXOTH) ? 'x' : '-';
	perms[10] = '\0'; // Null-terminate the string
}

void sendPathToNS(char *path, char perms[11], size_t size, int nmSock, time_t lastModifiedTime, time_t lastAccessTime)
{
	struct fileDetails *det = (struct fileDetails *)malloc(sizeof(struct fileDetails));
	strcpy(det->path, path);
	strcpy(det->perms, perms);
	det->size = size;
	det->isDir = perms[0] == '-' ? false : true;
	det->lastAccessTime = lastAccessTime;
	det->lastModifiedTime = lastModifiedTime;
	strcpy(det->fileName, basename(path));

	int bytesSent = send(nmSock, det, sizeof(struct fileDetails), 0);
	if (bytesSent == -1)
	{
		handleNetworkErrors("send");
	}
	printf(BLUE_COLOR "Added %s as an accessible path\n" RESET_COLOR, path);
}

void *takeInputsDynamically(void *args)
{
	int nmSock = *(int *)(args);
	while (1)
	{
		// char *path;
		// path = malloc(sizeof(char) * 4096);
		// scanf("%s", path);
		char input[4096];
		fgets(input, 4096, stdin);
		char *token = strtok(input, " \t\n");
		if (strcmp(token, "ADD"))
		{
			printf(RED_COLOR "Invalid Syntax!\n" RESET_COLOR);
			continue;
		}
		char *path = strtok(NULL, " \n");
		if (check_path_exists(path))
		{
			// send given file
			struct stat dirStat;
			int r = stat(path, &dirStat);
			if (r == -1)
			{
				fprintf(stderr, "File error\n");
				exit(1);
			}
			char perms[11];
			convertPermissions(dirStat.st_mode, perms);
			size_t size = dirStat.st_size;
			sendPathToNS(path, perms, size, nmSock, dirStat.st_mtime, dirStat.st_atime);
		}
		else
		{
			printf("path does not exist\n");
		}
	}
	return NULL;
}

void *serveNM_Requests(void *args)
{
	int nmfd = *((int *)args);
	// printf("nmfd in ss: %d\n", nmfd);
	while (1)
	{
		char buffer[4096];
		int bytesRecv = recv(nmfd, buffer, sizeof(buffer), 0);
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
			// perror("recv");
			break;
		}
		if (bytesRecv == 0)
		{
			printf(RED_COLOR "[-] Naming Server has disconnected!\n" RESET_COLOR);
			exit(0);
			break;
		}

		printf(YELLOW_COLOR "Recieved command from NM: %s\n" RESET_COLOR, buffer);

		char *request_command = strtok(buffer, " \t\n");
		char *path = strtok(NULL, " \t\n");

		int status = -1;
		if (strcmp(request_command, "RMFILE") == 0)
		{
			// RMFILE path
			status = removeFile(path);
		}

		char statusMsg[1000];
		bzero(statusMsg, sizeof(statusMsg));
		if (status == -1)
		{
			printf(YELLOW_COLOR "Command failed\n" RESET_COLOR);

			strcpy(statusMsg, "ERROR IN EXECUTION\0");
		}
		else
		{
			printf(YELLOW_COLOR "Command executed\n" RESET_COLOR);
			strcpy(statusMsg, "SUCCESS IN EXECUTION\0");
		}
		// printf("sending: %s\n", statusMsg);
		int bytesSend = send(nmfd, statusMsg, strlen(statusMsg), 0);

		if (bytesSend == -1)
		{
			handle_errors("send");
		}
	}
	return NULL;
}

void *serveClient_Request(void *args)
{
	int connfd = *((int *)args);
	// printf("connfd in ss: %d\n", connfd);

	char buffer[4096];
	int bytesRecv = recv(connfd, buffer, sizeof(buffer), 0);
	if (bytesRecv == -1)
	{
		// perror("recv");
		handleNetworkErrors("recv");
	}

	printf("Recieved from client: %s\n", buffer);
	char *arg_arr[3];
	parse_input(arg_arr, buffer);
	char *request_command = arg_arr[0];

	if (strcmp(arg_arr[0], "WRITE") == 0)
	{
		struct stat fileStat;
		int r = stat(arg_arr[1], &fileStat);
		if (r == -1)
		{
			fprintf(stderr, "File error\n");
			exit(1);
		}
		char perms[11];
		convertPermissions(fileStat.st_mode, perms);

		// send the perms first
		bytesRecv = send(connfd, perms, sizeof(perms), 0);
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
		}

		// send the file to client
		if (!sendFile(arg_arr[1], connfd))
			printf("SS: sent the file to client\n");
		else
			printf(RED "File not sent\n" reset);
	}
	else if (strcmp(arg_arr[0], "READ") == 0)
	{
		struct stat fileStat;
		int r = stat(arg_arr[1], &fileStat);
		if (r == -1)
		{
			fprintf(stderr, "File error\n");
			exit(1);
		}
		char perms[11];
		convertPermissions(fileStat.st_mode, perms);

		// send the perms first
		bytesRecv = send(connfd, perms, sizeof(perms), 0);
		if (bytesRecv == -1)
		{
			handleNetworkErrors("recv");
		}

		// send the file to client
		if (!sendFile(arg_arr[1], connfd))
			printf("SS: sent the file to client\n");
		else
			printf(RED "File not sent\n" reset);
	}
	else if (strcmp(arg_arr[0], "FILEINFO") == 0)
	{
		if (!check_path_exists(arg_arr[1]))
		{
			printf("file not found"); // error
			return NULL;
		}
		else
		{
			if (isDirectory(arg_arr[1]))
			{
				printf("directory path given instead of file"); // error
				return NULL;
			}
			else
			{
				struct stat dirStat;
				int r = stat(arg_arr[1], &dirStat);
				if (r == -1)
				{
					fprintf(stderr, "File error\n");
					exit(1);
				}
				char perms[11];
				convertPermissions(dirStat.st_mode, perms);
				size_t size = dirStat.st_size;
				// since it is connfd, this sends the file details to client
				sendPathToNS(arg_arr[1], perms, size, connfd, dirStat.st_mtime, dirStat.st_atime);
				printf("SS: sent the file details to client\n");
			}
		}
		return NULL;
	}
	return NULL;
}

void *acceptClients(void *args)
{
	int sockfd = *((int *)args);
	struct sockaddr_in cli;
	socklen_t len = sizeof(cli);
	while (1)
	{
		int connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
		if (connfd < 0)
		{
			// perror("accept");
			handleNetworkErrors("accept");
			exit(0);
		}
		pthread_t serveClientThread;
		pthread_create(&serveClientThread, NULL, serveClient_Request, &connfd);
	}
	return NULL;
}

int initializeNMConnection(char *ip, int port, int nmPort, int cliPort)
{
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;

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

	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr.s_addr = inet_addr(ip);
	cliaddr.sin_port = htons(nmPort);

	// to avoid bind error
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if ((bind(sockfd, (SA *)&cliaddr, sizeof(cliaddr))) != 0)
	{
		handleNetworkErrors("bind");
		// perror("bind");
		exit(0);
	}

	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		handleNetworkErrors("connect");
		exit(0);
	}

	char joinMsg[4096] = "JOIN_AS Storage Server";
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

	return sockfd;
}

int initializeNMConnectionForRecords(char *ip, int port)
{
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		handleNetworkErrors("socket");
		exit(0);
	}
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);

	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		handleNetworkErrors("connect");
		exit(0);
	}

	return sockfd;
}

int initialzeClientsConnection(int port)
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
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	// to avoid bind error
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
	{
		// perror("bind");
		handleNetworkErrors("bind");
		exit(0);
	}
	if (listen(sockfd, 12) != 0)
	{
		handleNetworkErrors("listen");
	}

	return sockfd;
}

int main(int argc, char *argv[])
{
	// int nmPort = 6970;
	// int cliPort = 6971;
	if (argc != 4)
	{
		printf(RED_COLOR "[-] Invalid Arguments!\n" RESET_COLOR);
		exit(0);
	}
	int mainPort = atoi(argv[1]);
	int nmPort = atoi(argv[2]);
	int cliPort = atoi(argv[3]);
	// printf("NMPORT: %d, CLIPORT:%d\n", nmPort, cliPort);
	int cliSock = initialzeClientsConnection(cliPort);
	printf(GREEN_COLOR "[+] Storage Server Initialized\n" RESET_COLOR);

	nmSock1 = initializeNMConnection("127.0.0.1", mainPort, nmPort, cliPort); // for feedback transfer
	int nmSock2 = initializeNMConnectionForRecords("127.0.0.1", mainPort);	  // for records

	printf(GREEN_COLOR "[+] Connected to Naming Server\n" RESET_COLOR);
	printf(YELLOW_COLOR "Port For NM Communication: %d\nPort for Client Communication: %d\n" RESET_COLOR, nmPort, cliPort);

	pthread_t nmThread, clientsThread, inputThread;
	pthread_create(&nmThread, NULL, serveNM_Requests, (void *)&nmSock1);
	pthread_create(&clientsThread, NULL, acceptClients, (void *)&cliSock);
	pthread_create(&inputThread, NULL, takeInputsDynamically, (void *)&nmSock2);

	pthread_join(inputThread, NULL);
	pthread_join(nmThread, NULL);
	pthread_join(clientsThread, NULL);

	return 0;
}
