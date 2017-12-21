#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int CREATE_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

bool isdir(char *path) {
  DIR *dir = opendir(path);
  bool flag = dir != NULL;
  if (flag) {
    closedir(dir);
  }
  return flag;
}

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
  if (isdir(path)) {
    fprintf(stderr, "Error: %s is a directory (not copied).", path);
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
    if (isdir(path)) {
      // target is folder
      if (sourceFilename != NULL) {
        // append filename
        size_t pathLength = strlen(path);
        size_t filenameLength = strlen(sourceFilename);
        // +1 for \0, +1 for potential appended seperator
        size_t newPathLength = pathLength + filenameLength + 2;
        bool appendSeperator = path[pathLength - 1] != '/';
        char newPath[newPathLength];
        strncpy(newPath, path, pathLength);
        if (appendSeperator) {
          newPath[pathLength++] = '/';
        }
        strncpy(newPath + pathLength, sourceFilename, filenameLength);
        newPath[pathLength + filenameLength] = '\0';
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

void cp(char *source, char *target) {
  int sourceFd = openSource(source);
  char *sourceFilename = getFilename(source);
  int targetFd = openTarget(target, sourceFilename);
  char buffer[BUFFER_SIZE];
  ssize_t n = 0;
  do {
    n = read(sourceFd, buffer, BUFFER_SIZE);
    write(targetFd, buffer, BUFFER_SIZE);
  } while (n == BUFFER_SIZE);
  close(sourceFd);
  close(targetFd);
}
