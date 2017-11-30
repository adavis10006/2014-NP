#include "socket_header.h"

int main()
{
	int fd = open("/tmp/1_to_1_fifo", O_RDWR);
	char c;
	while(read(fd, &c, sizeof(c)) != 0)
		printf("%c", c);
	return 0;
}