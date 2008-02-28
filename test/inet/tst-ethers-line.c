#include <netinet/ether.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define ETHER_LINE_LEN 256

int main(void)
{
	struct ether_addr addr;
	char hostname[ETHER_LINE_LEN];
	int fd, i;
	const char *ethers;
	struct stat statb;
	
	if ((fd = open(ETHER_FILE_NAME, O_RDONLY)) == -1) {
		perror ("Cannot open file");
		exit(1);
	}

	if (fstat(fd, &statb)) {
		perror("Stat failed");
		exit(1);
	}
	ethers = mmap(NULL, statb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	
	if (ethers == MAP_FAILED) {
		perror("File mapping failed");
		exit(1);
	}		

	ether_line(ethers, &addr, hostname);
	
	for (i = 0; i < 6; i++) {
		printf("%02x", addr.ether_addr_octet[i]);
		if (i < 5)
			printf(":");
	}
	printf(" %s\n", hostname);

	return 0;
}
