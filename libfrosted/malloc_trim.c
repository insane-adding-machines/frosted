#include <sys/types.h>
#include <sys/reent.h>

int _malloc_trim(struct _reent *r, size_t pad)
{
    return 0;
}

