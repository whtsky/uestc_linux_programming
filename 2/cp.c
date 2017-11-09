#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#define BUFFER_SIZE 1024

int CREATE_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

char *getFilename(char *path) {
  char *token = strtok(path, "/");
  char *lastToken = NULL;
  while (token) {
    lastToken = token;
    token = strtok(NULL, "/");
  }
  return lastToken;
}

int openSource(char *path) {
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "Error opening %s: %s", path, strerror(errno));
    exit(EXIT_FAILURE);
  }
  return fd;
}

int openTarget(char *path, char *sourceFilename) {
  int fd = open(path, O_CREAT | O_EXCL | O_WRONLY, CREATE_MODE);
  if (fd >= 0) {
    return fd;
  }
  if (errno == EEXIST) {
    DIR *dir = opendir(path);
    if (dir != NULL) {
      closedir(dir);
      // target is folder
      if (sourceFilename != NULL) {
        // append filename
        int pathLength = strlen(path);
        int filenameLength = strlen(sourceFilename);
        int newPathLength = pathLength + filenameLength + 1;
        bool appendSeperator = path[pathLength - 2] != '/';
        if (appendSeperator) {
          newPathLength++;
        }
        char newPath[newPathLength];
        strncpy(newPath, path, pathLength);
        if (appendSeperator) {
          newPath[pathLength++] = '/';
        }
        strncpy(newPath + pathLength, sourceFilename, filenameLength);
        newPath[newPathLength - 1] = '\0';
        fd = openTarget(newPath, NULL);
        return fd;
      }
    } else {
      while (1) {
        printf("%s already exist. Do you want to\n", path);
        printf("[R]eplace [A]ppend: ");
        char choice;
        scanf(" %c", &choice);
        if (choice == 'R' || choice == 'r') {
          return open(path, O_WRONLY | O_TRUNC);
        } else if (choice == 'A' || choice == 'a') {
          return open(path, O_WRONLY | O_APPEND);
        }
        printf("Invalid input: %c\n", choice);
      }
    }
  }
  fprintf(stderr, "Error opening %s: %s", path, strerror(errno));
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <source> <target>", argv[0]);
    exit(EXIT_FAILURE);
  }
  int sourceFd = openSource(argv[1]);
  char *sourceFilename = getFilename(argv[1]);
  int targetFd = openTarget(argv[2], sourceFilename);
  char buffer[BUFFER_SIZE];
  size_t n = 0;
  do {
    n = read(sourceFd, buffer, BUFFER_SIZE);
    write(targetFd, buffer, BUFFER_SIZE);
  } while (n == BUFFER_SIZE);
  close(sourceFd);
  close(targetFd);
  return 0;
}
