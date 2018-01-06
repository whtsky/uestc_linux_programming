#include "cp_wrapper.h"
#include <dirent.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
  char source[PATH_MAX];
  char target[PATH_MAX];
} pathcp_params;

bool recursion = false;
char *program_name;

void print_usage(void) {
  printf("%s [-r] source target\n\n", program_name);
  printf("\t -r: If source_file designates a directory, %s copies the "
         "directory and the entire subtree connected at that point.\n",
         program_name);
  puts("\t -h: Show this page");
  exit(EXIT_SUCCESS);
}

bool is_dir(mode_t mode) { return (mode & S_IFMT) == S_IFDIR; }

typedef struct pid_linked_list {
  pthread_t pid;
  struct pid_linked_list *next;
} * pid_node;

pid_node create_pid_node(void) {
  return malloc(sizeof(struct pid_linked_list));
}

char *normalize_path(char *buf, char *source, bool appendSeperator) {
  // copy path, return char pointer for writing filename
  // todo: better naming
  size_t basepath_length;
  if (source[0] != '/') {
    getcwd(buf, PATH_MAX);
    size_t baselen = strlen(buf);

    if (buf[baselen - 1] != '/') {
      buf[baselen++] = '/';
    }
    basepath_length = baselen + strlen(source);
    strcpy(buf + baselen, source);
  } else {
    strcpy(buf, source);
    basepath_length = strlen(source);
  }
  if (appendSeperator && buf[basepath_length - 1] != '/') {
    buf[basepath_length++] = '/';
  }
  buf[basepath_length] = '\0';
  return buf + basepath_length;
}

void pathcp(pathcp_params *params) {
  DIR *current_dir = opendir(params->source);
  if (current_dir == NULL) {
    fprintf(stderr, "Can't open %s.\n", params->source);
    return;
  }
  char path[PATH_MAX];
  char *pathStart = normalize_path(path, params->source, true);
  struct dirent *dp;
  struct stat buf;
  pid_node pid = NULL;
  while ((dp = readdir(current_dir)) != NULL) {
    size_t namelen = strlen(dp->d_name);
    if (namelen > 0 && dp->d_name[0] == '.') {
      continue;
    }
    // concat filepath
    strncpy(pathStart, dp->d_name, namelen);
    *(pathStart + namelen) = '\0';

    lstat(path, &buf);
    if (is_dir(buf.st_mode)) {
      if (!recursion) {
        continue;
      }
      // concat path, run child pathcp
      pathcp_params child_param;
      normalize_path(child_param.source, path, true);
      char *targetStart =
          normalize_path(child_param.target, params->target, true);
      strncpy(targetStart, dp->d_name, namelen);
      *(targetStart + namelen) = '\0';
      pid_node child = create_pid_node();
      child->next = pid;
      pid = child;
      pthread_create(&child->pid, NULL, (void *(*)(void *))pathcp,
                     &child_param);
      pthread_join(child->pid, NULL);
    } else {
      // run cp
      cp_params child_param;
      normalize_path(child_param.source, path, false);
      /* my cp supports target as folder & auto concat filename
       * so no need to concat filename here
       */
      normalize_path(child_param.target, params->target, true);
      pid_node child = create_pid_node();
      child->next = pid;
      pid = child;
      pthread_create(&child->pid, NULL, (void *(*)(void *))cp_thread,
                     &child_param);
    }
  }
  closedir(current_dir);

  // wait all threads to complete
  while (pid != NULL) {
    pthread_join(pid->pid, NULL);
    pid_node next = pid->next;
    free(pid);
    pid = next;
  }
}

int main(int argc, char **argv) {
  int ch;
  program_name = argv[0];
  while ((ch = getopt(argc, argv, "rh")) != -1) {
    switch (ch) {
    case 'r':
      recursion = true;
      break;
    case 'h':
      /* FALLTHROUGH */
    default:
      print_usage();
    }
  }
  argc -= optind;
  if (argc != 2) {
    print_usage();
  }
  argv += optind;
  pathcp_params params;
  normalize_path(params.source, argv[0], false);
  normalize_path(params.target, argv[1], false);
  pthread_t tid;
  pthread_create(&tid, NULL, (void *(*)(void *))pathcp, &params);
  pthread_join(tid, NULL);
  return 0;
}
