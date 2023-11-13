#ifndef _API_H_
#define _API_H_

int check_path_exists(const char *directoryPath);
int make_directory(char *dirname);
int make_file(char* dirname);
int isDirectory(const char *path);
int recursive_directory_deletion(char *curr_address);
int remove_files_and_directory(char* path);
void get_request(char request[], char actual_request[]);
void get_path(char request[], char path[]);
int handle_naming_server_commands(char* command, char* inputS);
void parse_input(char* array[], char* inputS);

#endif