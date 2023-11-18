#ifndef _API_H_
#define _API_H_

int send_file(char *filename, int sockfd);
int receive_file(char *filename, int sockfd);
int receive_directory(int connfd);
int check_path_exists(const char *directoryPath);
int make_directory(char *dirname);
int make_file(char *dirname);
int isDirectory(const char *path);
int recursive_directory_deletion(char *curr_address);
int recursive_directory_sending(char *curr_address, int nmfd);
int remove_files_and_directory(char *path);
void get_request(char request[], char actual_request[]);
void get_path(char request[], char path[]);
int handle_naming_server_commands(char *command, char *inputS, int nmfd);
void parse_input(char *array[], char *inputS);
void file_separator(char *array[], char *inputS);
int writeFile(char *path, char *editor);
int readFile(char *path, char *editor);
int sendFile(char *path, int sockfd);
int receiveFile(char *path, int sockfd);
mode_t reversePermissions(char *perms);

int removeFile(char *path);

#endif