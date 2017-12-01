#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

struct stat buf;
char path[PATH_MAX];

static struct {
  // argv[0]
  char *program_name;
  /*
    -a
    Include directory entries whose names begin with a dot (.).
    -A
    List all entries except for . and ..
  */
  bool seedot;  // show all files, include hidding
  bool listdot; // show . and ..
  /*
    -L
    Follow all symbolic links to final target and list the file or directory
    the link references rather than the link itself.
  */
  bool follow_link;
  /*

  */

} flags = {.program_name = NULL,
           .seedot = false,
           .listdot = false,
           .follow_link = false};

void print_permissions(mode_t mode) {
  putchar((mode & S_IRUSR) ? 'r' : '-');
  putchar((mode & S_IWUSR) ? 'w' : '-');
  putchar((mode & S_IXUSR) ? 'x' : '-');
  putchar((mode & S_IRGRP) ? 'r' : '-');
  putchar((mode & S_IWGRP) ? 'w' : '-');
  putchar((mode & S_IXGRP) ? 'x' : '-');
  putchar((mode & S_IROTH) ? 'r' : '-');
  putchar((mode & S_IWOTH) ? 'w' : '-');
  putchar((mode & S_IXOTH) ? 'x' : '-');
}

void print_filetype(mode_t mode) {
  switch (mode & S_IFMT) {
  case S_IFREG:
    putchar('-');
    break;
  case S_IFDIR:
    putchar('d');
    break;
  case S_IFLNK:
    putchar('l');
    break;
  case S_IFCHR:
    putchar('c');
    break;
  case S_IFBLK:
    putchar('b');
    break;
  case S_IFSOCK:
    putchar('s');
    break;
  case S_IFIFO:
    putchar('f');
    break;
  }
}

void print_file(char *path, struct dirent *dp) {
  if (flags.follow_link) {
    stat(path, &buf);
  } else {
    lstat(path, &buf);
  }
  print_filetype(buf.st_mode);
  print_permissions(buf.st_mode);
  putchar(' ');
  printf("%d\t", buf.st_nlink);
  struct passwd *user = getpwuid(buf.st_uid);
  if (user == NULL) {
    perror("Error reading username");
    exit(EXIT_FAILURE);
  }
  printf("%s\t", user->pw_name);
  struct group *group = getgrgid(buf.st_gid);
  if (group == NULL) {
    perror("Error reading groupname");
    exit(EXIT_FAILURE);
  }
  printf("%s\t%lld\t", group->gr_name, buf.st_size);
  char buffer[26];
  struct tm *tm_info = localtime(&(buf.st_mtime));

  strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  printf("%s\t%s\n", buffer, dp->d_name);
}

void print_usage() {
  printf("%s: List directory contents.\n\n", flags.program_name);
  printf("%s [-AaLh] [directory]\n", flags.program_name);
  puts("\t -a: Include directory entries whose names begin with a dot (.)");
  puts("\t -A: List all entries except for . and ..");
  puts("\t -L: Follow all symbolic links to final target and list the file or "
       "directory the link references rather than the link itself.");
  puts("\t -h: Show this page");
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  flags.program_name = argv[0];
  char ch;
  while ((ch = getopt(argc, argv, "AaLh")) != -1) {
    switch (ch) {
    case 'a':
      flags.listdot = true;
    /* FALLTHROUGH */
    case 'A':
      flags.seedot = true;
      break;
    case 'L':
      flags.follow_link = true;
      break;
    case 'h':
    case '?':
    default:
      print_usage();
    }
  }
  argc -= optind;
  if (argc > 2) {
    print_usage();
  }
  if (argc == 2) {
    argv += optind;    
    strncpy(path, argv[1], strlen(argv[1]));
  } else {
    getcwd(path, PATH_MAX);
  }
  int basepath_length = strlen(path);
  if (path[basepath_length - 1] != '/') {
    path[basepath_length++] = '/';
  }
  char *pathStart = path + basepath_length;

  DIR *current_dir = opendir(path);
  if (current_dir == NULL) {
    fprintf(stderr, "Can't open %s.\n", path);
    exit(EXIT_FAILURE);
  }
  struct dirent *dp;
  int total = 0;
  char *filepath = malloc(MAXPATHLEN);
  while ((dp = readdir(current_dir)) != NULL) {
    int namelen = strlen(dp->d_name);
    if (namelen > 0 && dp->d_name[0] == '.') {
      if (!flags.seedot) {
        continue;
      }
      if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
        if (!flags.listdot) {
          continue;
        }
      }
    }
    // concat filepath
    strncpy(pathStart, dp->d_name, namelen);
    *(pathStart + namelen) = '\0';
    print_file(path, dp);
    total++;
  }
  closedir(current_dir);
  printf("total %d\n", total);
  return 0;
}
