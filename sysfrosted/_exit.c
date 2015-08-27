
/*
 * Frosted version of exit.
 */

#include "frosted_api.h"

void _exit(int rc)
{
    sys_exit(rc);
}
