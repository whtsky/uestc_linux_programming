#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define DEFAULT_SOURCE_FOLDER "."
#define DEFAULT_TARGET_FOLDER "/tmp/wuhaotian"

char path[PATH_MAX];
char target[PATH_MAX];

int main(int argc, char *argv[]) {
  if (argc != 3 && argc != 1) {
    fprintf(stderr, "Usage: %s <source> <target>", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (argc == 1) {
    getcwd(path, PATH_MAX);
    strcpy(target, DEFAULT_TARGET_FOLDER);
  } else {
    strcpy(path, argv[1]);
    strcpy(target, argv[2]);
  }

  // don't care rv
  mkdir(target, 0770);

  int basepath_length = strlen(path);
  if (path[basepath_length - 1] != '/') {
    path[basepath_length++] = '/';
  }
  char *pathStart = path + basepath_length;

  int target_length = strlen(target);
  if (target[target_length - 1] != '/') {
    target[target_length++] = '/';
  }
  char *targetStart = target + target_length;

  DIR *current_dir = opendir(path);
  if (current_dir == NULL) {
    fprintf(stderr, "Can't open %s.\n", path);
    exit(EXIT_FAILURE);
  }
  struct dirent *dp;
  char *filepath = malloc(MAXPATHLEN);
  while ((dp = readdir(current_dir)) != NULL) {
    int namelen = strlen(dp->d_name);
    if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
      continue;
    }
    // concat filepath
    strncpy(pathStart, dp->d_name, namelen);
    *(pathStart + namelen) = '\0';
    struct stat buf;
    stat(path, &buf);
    int pid;
    pid = fork();
    if (pid < 0) {
      perror("Error forking");
      exit(EXIT_FAILURE);
    }
    if (pid == 0) {
      // child
      if ((buf.st_mode & S_IFMT) == S_IFDIR) {
        printf("folder %s, make child cpr\n", path);
        int namelen = strlen(dp->d_name);
        strncpy(targetStart, dp->d_name, namelen);
        *(targetStart + namelen) = '\0';
        execlp("./cpr", "cpr", path, target, NULL);
        exit(EXIT_SUCCESS);
      } else {
        // copy
        printf("Copying %s to %s\n", path, target);
        execlp("./mycp", "mycp", path, target, NULL);
        exit(EXIT_SUCCESS);
      }
    }
    // block waiting child
    wait(NULL);
  }
  closedir(current_dir);

  return EXIT_SUCCESS;
}