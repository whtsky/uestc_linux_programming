#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include "cp.h"

#define BUFFER_SIZE 1024

bool isdir(int fd) {
  struct stat buf;
  if (fstat(fd, &buf) != 0) {
    perror("fstat");
    exit(EXIT_FAILURE);
  }
  return S_ISDIR(buf.st_mode);
}

int openSource(char *path) {
  int fd = open(path, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "Error opening %s: %s\n", path, strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (isdir(fd)) {
    fprintf(stderr, "Error: %s is a directory (not copied).", path);
    exit(EXIT_FAILURE);
  }
  return fd;
}

int openTarget(char *path, char *sourceFilename) {
  int fd = open(path, O_CREAT | O_EXCL | O_WRONLY, CREATE_MODE);
  if (fd != -1) {
    return fd;
  }
  if (errno == EEXIST) {
    fd = open(path, O_RDONLY);
    if (isdir(fd)) {
appendpath:
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
        return openTarget(newPath, NULL);
      }
    }
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
  } else if (errno == EISDIR) {
    mkdir(path, CREATE_MODE | S_IXUSR);
    goto appendpath;
  }
  fprintf(stderr, "Error opening %s: %s\n", path, strerror(errno));
  exit(EXIT_FAILURE);
}

void cp(char *source, char *target) {
  int sourceFd = openSource(source);
  char *sourceDup = strdup(source);
  char *sourceFilename = basename(source);
  int targetFd = openTarget(target, sourceFilename);
  char buffer[BUFFER_SIZE];
  ssize_t n = 0;
  do {
    n = read(sourceFd, buffer, BUFFER_SIZE);
    write(targetFd, buffer, n);
  } while (n == BUFFER_SIZE);
  close(sourceFd);
  close(targetFd);
  free(sourceDup);
}
