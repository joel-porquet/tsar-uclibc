#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int seteuid(uid_t uid)
{
    return setreuid(-1, uid);
}
