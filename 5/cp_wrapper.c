#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cp.h"
#include "cp_wrapper.h"

#define CREATE_DIR_MODE (CREATE_MODE | S_IXUSR)

static void _mkdir(const char *dir) {
  char tmp[256];
  char *p = NULL;
  size_t len;

  snprintf(tmp, sizeof(tmp), "%s", dir);
  len = strlen(tmp);
  if (tmp[len - 1] == '/')
    tmp[len - 1] = 0;
  for (p = tmp + 1; *p; p++)
    if (*p == '/') {
      *p = 0;
      mkdir(tmp, CREATE_DIR_MODE);
      *p = '/';
    }
  mkdir(tmp, CREATE_DIR_MODE);
}

void cp_thread(cp_params *params) {
  printf("cp: %s -> %s\n", params->source, params->target);
  // check if target folder exist, if not, create it
  _mkdir(params->target);
  cp(params->source, params->target);
}
