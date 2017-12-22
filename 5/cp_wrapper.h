#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef CP_WRAPPER_H
#define CP_WRAPPER_H

typedef struct {
    char source[PATH_MAX];
    char target[PATH_MAX];
} cp_params;

void cp_thread(cp_params *params);
#endif