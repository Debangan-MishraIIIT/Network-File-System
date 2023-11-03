#include "headers.h"
// there should be an inifinite thread in the NS side that also checks and updates the paths

int check_path_exists(const char *directoryPath)
{
	struct stat dirStat;
	if (stat(directoryPath, &dirStat) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void convertPermissions(mode_t st_mode, char *perms) {
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

void *take_inputs_dynamically(void *args)
{
	while (1)
	{
		char *path;
		path = malloc(sizeof(char) * 250);
		scanf("%s", path);
		if (check_path_exists(path))
		{
			struct stat dirStat;
			printf("path exists\n");
			int r = stat(path, &dirStat);
			if (r == -1)
			{
				fprintf(stderr, "File error\n");
				exit(1);
			}
			char perms[11];
			convertPermissions(dirStat.st_mode, perms);
			printf("Permission bits: %s\n", perms);
			// send path to NS
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
	// int sockfd = joinNamingServer("127.0.0.1", namingServerPort, "Storage Server");

	pthread_t nmThread;
	pthread_t inputThread;

	// pthread_create(&nmThread, NULL, serveNM_Requests, (void *)&sockfd);
	pthread_create(&inputThread, NULL, take_inputs_dynamically, NULL);
	pthread_join(inputThread, NULL);
	// pthread_join(nmThread, NULL);

	return 0;
}

// int main()
// {
// 	int sockfd= client_establish_connection(8080, "127.0.0.1");
// 	send_file("file.txt", sockfd);
// 	close(sockfd);
// }
