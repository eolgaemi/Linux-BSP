#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(void)

{	
	int ret = -1;

	ret = mknod("/dev/test",S_IRWXU | S_IRWXG | S_IRWXO | S_IFCHR, (240<<8)|1);

	if(ret < 0)
	{
		perror("mknod");
		return ENXIO;
	}
	return 0;
}
