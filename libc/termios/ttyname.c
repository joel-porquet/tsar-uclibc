#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

char *ttyname(fd)
int fd;
{
	static char dev[] = "/dev";
	struct stat st, dst;
	DIR *fp;
	struct dirent *d;
	static char name[NAME_MAX];
	int noerr = errno;

	if (fstat(fd, &st) < 0)
		return 0;
	if (!isatty(fd)) {
		errno = ENOTTY;
		return 0;
	}

	fp = opendir(dev);
	if (fp == 0)
		return 0;
	strcpy(name, dev);
	strcat(name, "/");

	while ((d = readdir(fp)) != 0) {
		strcpy(name + sizeof(dev), d->d_name);
		if (stat(name, &dst) == 0
			&& st.st_dev == dst.st_dev && st.st_ino == dst.st_ino) {
			closedir(fp);
			errno = noerr;
			return name;
		}
	}
	closedir(fp);
	errno = noerr;
	return 0;
}
