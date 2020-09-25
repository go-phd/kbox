
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
#define SECTION_USER_LEN (KBOX_RESERVERED_MEMORY_LEN - 0x100000)
#define SECTION_USER_MMAP_LEN SECTION_USER_LEN//((SECTION_USER_LEN / 2) & 0xff100000)

int lddfd;

void *mmap_address = NULL;
void *mmap_address1 = NULL;

void write_mmap(void) 
{
	//char *value = "1234567890";
	if (mmap_address)
	{
		//*(volatile unsigned char *)mmap_address = *argv[2];
		//printf("bvalue = %s\n", (char *)mmap_address);
		int i = 0;
		char *pmmap = (char *)mmap_address;
		int cnt = SECTION_USER_MMAP_LEN;
		printf("cnt = 0x%x\n", cnt);
		for (i = 0; i < cnt; i++) {
			//pmmap[i] = '8';
			*(volatile unsigned char *)pmmap = '5';
			pmmap++;
		}
		//memcpy(mmap_address, value, strlen(value));
	}

	if (mmap_address1)
	{
		//*(volatile unsigned char *)mmap_address = *argv[2];
		//printf("bvalue = %s\n", (char *)mmap_address);
		int i = 0;
		int cnt = 8 * 1024 * 1024;
		printf("cnt = 0x%x\n", cnt);
		char *pmmap = (char *)mmap_address1;
		
		for (i = 0; i < cnt; i++) {
			*(volatile unsigned char *)pmmap = '9';
			pmmap++;
		}
		//memcpy(mmap_address, value, strlen(value));
	}
}

void read_mmap(void) 
{
	//char bbb[256] = {0};
	char *p = (char *)mmap_address;
	//static int i = 0;
	printf("mmap_address = %p\n", mmap_address);
	if (mmap_address)
	{
		int i;
		//int cnt = 0;
		
		for(i = 0; i < SECTION_USER_MMAP_LEN; i++) {
			if (i % 1000 == 0) {
				printf("i = %d, 0x%x\n", i, p[i]);
			}
		}
		
		
		//value = *(volatile unsigned long *)mmap_address;
		//printf("avalue = %s\n", bbb);//*(char *)mmap_address);

		//*(volatile unsigned char *)mmap_address = 'w';
		//printf("bvalue = %s\n", mmap_address);
	}
}

void read_to_file(void) 
{
	int rlen = 0;
	
	char *revdbuf = malloc(KBOX_RESERVERED_MEMORY_LEN);
	if (revdbuf == NULL) {
		printf("mallco revdbuf fail.\n");
	}
		
	rlen = read(lddfd, revdbuf, KBOX_RESERVERED_MEMORY_LEN);
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
	lddfd = open ("/dev/kbox", O_RDWR);//"wb"
	printf("open(/dev/kbox) lddfd %d\n", lddfd);
	if (lddfd < 0)
	{
		printf("open(/dev/kbox) fail\n");
		return -1;
	}

	mmap_address = mmap(0, SECTION_USER_MMAP_LEN, PROT_READ | PROT_WRITE, /*MAP_FILE  | *//*MAP_PRIVATE*/MAP_SHARED, lddfd, 0);
	//mmap_address1 = mmap(0, SECTION_USER_MMAP_LEN, PROT_READ | PROT_WRITE, /*MAP_FILE  | *//*MAP_PRIVATE*/MAP_SHARED, lddfd, SECTION_USER_MMAP_LEN);
	printf("--SIMPLE_DBG--, SECTION_USER_LEN = %d, SECTION_USER_MMAP_LEN = %d\n", SECTION_USER_LEN, SECTION_USER_MMAP_LEN);
	printf("mmap_address = %p, mmap_address1 = %p\n", mmap_address, mmap_address1);

	write_mmap();
	read_mmap();
	read_to_file();

	if (mmap_address)
	{
		munmap(mmap_address, SECTION_USER_MMAP_LEN);
		mmap_address = 0;
		printf("mmap_address = %p\n", mmap_address);
	}

	if (mmap_address1)
	{
		munmap(mmap_address1, SECTION_USER_MMAP_LEN);
		mmap_address1 = 0;
		printf("mmap_address1 = %p\n", mmap_address1);
	}

	close(lddfd);
	
	return 0;	
}


