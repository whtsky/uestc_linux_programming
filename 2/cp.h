#include <sys/stat.h>

#define CREATE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
void cp(char *source, char *target);