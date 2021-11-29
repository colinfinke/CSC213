#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void list_directory(const char* directory_name);

int main(int argc, char** argv) {
  // Process arguments
  if (argc == 1) {
    list_directory(".");
  } else if (argc == 2) {
    list_directory(argv[1]);
  } else {
    fprintf(stderr, "Please specify one or zero command line arguments.\n");
  }

  return 0;
}

void list_directory(const char* directory_name) {
  // set up directory pointer and directory struct
  DIR* dir = opendir(directory_name);
  struct dirent* dirStruct;

  // check to make sure there is a directory
  if (dir != NULL) {
    // loop through each file/directory in the directory
    while ((dirStruct = readdir(dir)) != NULL) {
      // makes a struct for the fileInfo
      struct stat fileInfo;

      // sets pathFull to the entire path to dirStruct->d_name
      char* pathFull = realpath(directory_name, NULL);
      if (pathFull == NULL) exit(0);
      strcat(pathFull, "/");
      strcat(pathFull, dirStruct->d_name);

      // sets up fileInfo for the path
      if (stat(pathFull, &fileInfo) == 0) {
        // prints permissions in drwxrwxrwx format
        printf(S_ISDIR(fileInfo.st_mode) ? "d" : "-");
        printf((fileInfo.st_mode & S_IRUSR) ? "r" : "-");
        printf((fileInfo.st_mode & S_IWUSR) ? "w" : "-");
        printf((fileInfo.st_mode & S_IXUSR) ? "x" : "-");
        printf((fileInfo.st_mode & S_IRGRP) ? "r" : "-");
        printf((fileInfo.st_mode & S_IWGRP) ? "w" : "-");
        printf((fileInfo.st_mode & S_IXGRP) ? "x" : "-");
        printf((fileInfo.st_mode & S_IROTH) ? "r" : "-");
        printf((fileInfo.st_mode & S_IWOTH) ? "w" : "-");
        printf((fileInfo.st_mode & S_IXOTH) ? "x" : "-");

        // prints user's name, group, file size, and filename
        printf(" %s %s %lu %s\n", getpwuid(fileInfo.st_uid)->pw_name,
               getgrgid(fileInfo.st_uid)->gr_name, fileInfo.st_size, dirStruct->d_name);
      }
    }

    // closes dir
    closedir(dir);
  }
}
