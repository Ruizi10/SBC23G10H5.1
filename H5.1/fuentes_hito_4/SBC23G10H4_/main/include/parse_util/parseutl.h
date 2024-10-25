#ifndef __PARSEUTL__
#define __PARSEUTL__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>



#ifdef __cplusplus
extern "C"
{
#endif

ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp);
ssize_t getline(char **buf, size_t *bufsiz, FILE *fp);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* !__PARSEUTL__ */