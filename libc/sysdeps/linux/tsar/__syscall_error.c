/* wrapper for setting errno */

#include <errno.h>
#include <features.h>

int __syscall_error(int err_no) attribute_hidden;
int __syscall_error(int err_no)
{
	/* with TSAR, whenever there is an error, it's a negative value.
	 * transform it into a positive value when setting errno */
	__set_errno(-err_no);
	return -1;
}
