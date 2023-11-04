#ifndef _API_H_
#define _API_H_

int make_directory(char *dirname);
int make_file(char* dirname);
int isDirectory(const char *path);
int recursive_directory_deletion(char *curr_address);
int remove_files_and_directory(char* path);

#endif