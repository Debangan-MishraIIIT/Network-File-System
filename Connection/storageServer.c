#include "headers.h"
// there should be an inifinite thread in the NS side that also checks and updates the paths

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

void sendPathToNS(char *path, char perms[11], size_t size, int nmSock)
{
	struct fileDetails *det = (struct fileDetails *)malloc(sizeof(struct fileDetails));
	if (!det)
		handleSYSandInputErrors("malloc");
	strcpy(det->path, path);
	strcpy(det->perms, perms);
	det->size = size;
	det->isDir = perms[0] == '-' ? false : true;

	int bytesSent = send(nmSock, det, sizeof(struct fileDetails), 0);
	if (bytesSent == -1)
	{
		perror("send");
	}
	printf("File detials sent to SS\n");
}

void *take_inputs_dynamically(void *args)
{
	int nmSock = *(int *)(args);
	while (1)
	{
		char *path;
		path = malloc(sizeof(char) * 250);
		if (!path)
			handleSYSandInputErrors("malloc");
		scanf("%s", path);
		if (check_path_exists(path))
		{
			struct stat dirStat;
			printf("path exists\n");
			int r = stat(path, &dirStat);
			if (r == -1)
			{
				handleFileOperationError("stat");
				return NULL;
			}
			char perms[11];
			convertPermissions(dirStat.st_mode, perms);
			size_t size = dirStat.st_size;
			printf("path: %s\n", path);
			printf("Permission bits: %s\n", perms);
			printf("Size: %zu bytes\n", size);
			sendPathToNS(path, perms, size, nmSock);
			// send path to NS
		}
		else
		{
			handleFileOperationError("no_path");
		}
	}
	return NULL;
}

void *serveNM_Requests(void *args)
{
	int nmfd = *((int *)args);
	char err_message[1000];
	while (1)
	{
		char buffer[4096];
		bzero(buffer, sizeof(buffer));

		int bytesRecv = recv(nmfd, buffer, sizeof(buffer), 0);
		if (bytesRecv == -1)
		{
			handle_errors("recv");
		}
		if (bytesRecv == 0)
		{
			break;
		}

		printf("Recieved from NM: %s\n", buffer);
		char *argument_array[2];
		parse_input(argument_array, buffer);
		char *request_command = argument_array[0];
		char *request_path = argument_array[1];

		printf("command:%s\npath:%s\n", request_command, request_path);

		int resp;

		if (strcmp(request_command, "MKDIR") == 0 || strcmp(request_command, "MKFILE") == 0 || strcmp(request_command, "RMFILE") == 0 || strcmp(request_command, "RMDIR") == 0 || strcmp(request_command, "COPYFILE") == 0 || strcmp(request_command, "COPYDIR") == 0)
		{
			resp = handle_naming_server_commands(request_command, request_path, nmfd);
		}
		else if (strcmp(request_command, "RECDIR") == 0)
		{
			char main_cwd[1000];
			if (getcwd(main_cwd, sizeof(main_cwd)) == NULL)
			{
				handleFileOperationError("getcwd");
			}
			if (chdir(request_path) == -1)
			{
				handleFileOperationError("chdir");
			}
			resp = receive_directory(nmfd);
			if (chdir(main_cwd) == -1)
			{
				handleFileOperationError("chdir");
			}
		}

		if (resp == -1)
		{
			strcpy(err_message, "ERROR IN EXECUTION");
		}
		else
		{
			strcpy(err_message, "SUCCESS IN EXECUTION");
		}
		printf("sending: %s\n", err_message);
		int bytesSend = send(nmfd, err_message, strlen(err_message), 0);
		if (bytesSend == -1)
		{
			handle_errors("send");
		}
		printf("sent\n");
		bzero(err_message, sizeof(err_message));
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
		perror("recv");
	}

	printf("Recieved from client: %s\n", buffer);
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
			perror("accept");
			exit(EXIT_FAILURE);
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

	// to avoid bind error
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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
	char buffer[4096];
	int bytesRecv = recv(sockfd, buffer, sizeof(buffer), 0);
	if (bytesRecv == -1)
	{
		perror("recv");
	}
	if (strcmp(buffer, "ACCEPTED JOIN") != 0)
	{
		return -1;
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

int initializeNMConnectionForRecords(char *ip, int port)
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

	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		perror("connect");
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
		perror("socket");
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
		perror("bind");
		exit(0);
	}
	if (listen(sockfd, 12) != 0)
		perror("listen");

	return sockfd;
}

int main(int argc, char *argv[])
{
	// int nmPort = 6970;
	// int cliPort = 6971;
	if (argc != 3)
	{
		handleSYSandInputErrors("invalid_input");
		exit(0);
	}
	int nmPort = atoi(argv[1]);
	int cliPort = atoi(argv[2]);
	printf("NMPORT: %d, CLIPORT:%d\n", nmPort, cliPort);
	int cliSock = initialzeClientsConnection(cliPort);
	int nmSock1 = initializeNMConnection("127.0.0.1", 6969, nmPort, cliPort); // for feedback transfer
	int nmSock2 = initializeNMConnectionForRecords("127.0.0.1", 6969);		  // for records

	pthread_t nmThread, clientsThread, inputThread;
	pthread_create(&nmThread, NULL, serveNM_Requests, (void *)&nmSock1);
	pthread_create(&clientsThread, NULL, acceptClients, (void *)&cliSock);
	pthread_create(&inputThread, NULL, take_inputs_dynamically, (void *)&nmSock2);

	pthread_join(inputThread, NULL);
	pthread_join(nmThread, NULL);
	pthread_join(clientsThread, NULL);

	return 0;
}
