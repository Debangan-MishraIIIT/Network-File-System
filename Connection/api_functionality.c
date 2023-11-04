#include "headers.h"

int make_directory(char *dirname)
{
    int check;
    check = mkdir(dirname, 0777);

    // check if directory is created or not
    if (!check)
        printf("Directory created\n");
    else
    {
        printf("Unable to create directory\n");
        return -1;
    }
}

int make_file(char* dirname)
{
    int check;
    check = creat(dirname, 0777);

    // check if file is created or not
    //if file is already existing, it is cleared, just like a normal write
    if(check==-1)
    {
        printf("Unable to create file\n");
        return -1;
    }else{
        printf("File created\n");
    }
}

int isDirectory(const char *path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

int recursive_directory_deletion(char *curr_address)
{
    DIR *dir = opendir(curr_address);
    if (dir == NULL)
    {
        perror("opendir");
        return -1;
    }

    struct dirent *dir_read;
    while ((dir_read = readdir(dir)) != 0)
    {
        char buffer[maxlen];
        snprintf(buffer, sizeof(buffer), "%s/%s", curr_address, dir_read->d_name);
        if (isDirectory(buffer) == 1 && strcmp(dir_read->d_name, ".") != 0 && strcmp(dir_read->d_name, "..") != 0)
        {
            recursive_directory_deletion(buffer);
        }else if(isDirectory(buffer) == 0){
            remove(buffer);
        }
    }

    remove(curr_address);
    closedir(dir);
    return 0;
}

int remove_files_and_directory(char* path){
    
	if (!check_path_exists(path))
	{
		return -1;
	}
	else
	{
		if(isDirectory(path)){
            recursive_directory_deletion(path);
        }else{
            remove(path);
        }
	}
}
