#ifndef HAVE__COPY_H
#define HAVE__COPY_H 1

#include <unistd.h>

ssize_t copyFds(int infd, int outfd);

#endif //HAVE__COPY_H
