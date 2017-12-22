#include <stdio.h>
#include <stdlib.h>

#include "cp.h"
#include "cp_wrapper.h"

void cp_thread(cp_params *params) {
  printf("cp: %s -> %s\n", params->source, params->target);
  cp(params->source, params->target);
}
