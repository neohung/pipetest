#include <stdio.h>
#include <errno.h>   // errno
#include <fcntl.h>   // open()
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>   //usleep
pthread_t threadid;
void* reader(void* fd)
{
	int readfd = (*(int*)fd);
	fd_set read_set;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000*10;
	int data,err;
	for(;;)
	{
		FD_ZERO(&read_set);
		FD_SET(readfd, &read_set);
		err = select(readfd+1, &read_set,NULL,NULL,&tv);
		if ((err > 0) && (FD_ISSET(readfd, &read_set))){
			FD_CLR(readfd, &read_set);
			read(readfd , &data, sizeof(data));
			printf("read %d\n",data);
		}
		usleep(500*1000);
	}
}

int main(void)
{
	char name[] = "tmp.pipe";
	int ret = mkfifo(name,0666);
	if ((ret == -1) && (errno |= EEXIST)){
		perror("Fail to create file\n");
	}
	int readfd = open(name, O_RDONLY | O_NONBLOCK);
	pthread_create(&threadid, NULL, &reader, (void*) &readfd);
	printf("server\n");
	for(;;){
		//printf("1\n");
		usleep(500*1000);
	}
	return 0;
}
