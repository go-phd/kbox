
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#ifndef __WIN32__
#include <netinet/in.h>		 /*for host / network byte order conversions	*/
#endif
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include <mqueue.h>
#include <sys/stat.h>

#include <time.h>

#include <pthread.h>


#include <sys/select.h>
#include <sys/time.h>


// 保留内存容量，至少2MB
// 剩余最后 1KB 防止越界
#define KBOX_RESERVERED_MEMORY_LEN	(0x6F0000 - 0x1000)

int kboxfd;


void read_to_file(void) 
{
	int rlen = 0;
	
	char *revdbuf = malloc(KBOX_RESERVERED_MEMORY_LEN);
	if (revdbuf == NULL) {
		printf("mallco revdbuf fail.\n");
	}
		
	rlen = read(kboxfd, revdbuf, KBOX_RESERVERED_MEMORY_LEN);
	printf("rlen = %d\n", rlen);
	if (rlen > 0) {
		int fd = open ("./kbox.log", O_RDWR | O_CREAT);//"wb"
		if (fd) {
			write(fd, revdbuf, KBOX_RESERVERED_MEMORY_LEN);
			close(fd);
		}
	}

	free(revdbuf);
}


int main (int argc, char *argv[])
{
	kboxfd = open ("/dev/kbox", O_RDWR);//"wb"
	printf("open(/dev/kbox) kboxfd %d\n", kboxfd);
	if (kboxfd < 0)
	{
		printf("open(/dev/kbox) fail\n");
		return -1;
	}

	read_to_file();

	close(kboxfd);
	
	return 0;	
}


