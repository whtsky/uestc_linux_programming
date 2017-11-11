#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>

static struct {
  // argv[0]
  char *program_name;
  /* 
    -a
    Include directory entries whose names begin with a dot (.).
    -A
    List all entries except for . and ..
  */
  bool seedot; // show all files, include hidding  
  bool listdot; // show . and ..
  /*
    -L
    Follow all symbolic links to final target and list the file or directory
    the link references rather than the link itself.
  */
  bool follow_link;
  /*

  */

} flags = {
  .program_name=NULL,
  .seedot=false,
  .listdot=false,
  .follow_link=false
};

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

void print_usage() {
  printf("%s: List directory contents.\n\n", flags.program_name);
  printf("%s [-AaLh] [directory]\n", flags.program_name);
  puts("\t -a: Include directory entries whose names begin with a dot (.)");
  puts("\t -A: List all entries except for . and ..");
  puts("\t -L: Follow all symbolic links to final target and list the file or directory the link references rather than the link itself.");
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
		case 'h':
		case '?':
    default:
			print_usage();
		}
	}
  argc -= optind;
	argv += optind;
  int basepath_length;
  char *path;
  if (argc > 1) {
    // todo: add args support
    basepath_length = strlen(argv[argc - 1]);
    path = argv[argc - 1];
  } else {
    path = getcwd(NULL, 0);
    if (!path) {
      fprintf(stderr, "%s: can't get current dir.\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  DIR *current_dir = opendir(path);
  if (current_dir == NULL) {
    fprintf(stderr, "Can't open %s.\n", path);
    exit(EXIT_FAILURE);
  }
  struct dirent *dp;
  int total = 0;
  char *filepath = malloc(MAXPATHLEN);  
  while ((dp = readdir(current_dir)) != NULL) {
    if (dp->d_namlen > 0 && dp->d_name[0] == '.') {
      if (!flags.seedot) {
        continue;
      }
      if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
        if (!flags.listdot) {
          continue;
        }
      }
    }
    // todo: loop, concat filepath
    // print_file(dp);
    total++;
  }
  printf("total %d\n", total);

  closedir(current_dir);
  free(path);
  return 0;
}
