#include <stdio.h>
#include <errno.h>   // errno
#include <fcntl.h>   // open()
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>   //usleep
pthread_t threadid;
int rsp_fd;
int readfd ;
void send_response(int* fd, unsigned char* data, int size)
{
	int writefd = (*(int*)fd);
	write(writefd, data, size);
	//printf("rsp_fd %d, readfd %d\n",rsp_fd,readfd);
}
void* reader(void* fd)
{
	int readfd = (*(int*)fd);
	unsigned char readbuffer[300];
	fd_set read_set;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000*10;
	int data,err;
	int size;
	for(;;)
	{
		FD_ZERO(&read_set);
		FD_SET(readfd, &read_set);
		err = select(readfd+1, &read_set,NULL,NULL,&tv);
		if ((err > 0) && (FD_ISSET(readfd, &read_set))){
			FD_CLR(readfd, &read_set);
			size = read(readfd , readbuffer, sizeof(readbuffer));
			//printf("read size= %d\n",size);
			int r = 1234;
			send_response(&rsp_fd, (unsigned char*)&r, sizeof(r));
		}
		//usleep(500*1000);
	}
}

int main(void)
{
	char name[] = "host_to_slave.pipe";
	char rsp_name[] = "respone.pipe";
	int ret = mkfifo(name,0666);
	if ((ret == -1) && (errno |= EEXIST)){
		perror("Fail to create file\n");
	}
	ret = mkfifo(rsp_name,0666);
	if ((ret == -1) && (errno |= EEXIST)){
		perror("Fail to create file\n");
	}
	readfd = open(name, O_RDONLY | O_NONBLOCK);
	//need to read first
	sleep(5);
	rsp_fd = open(rsp_name, O_WRONLY | O_NONBLOCK);
	pthread_create(&threadid, NULL, &reader, (void*) &readfd);
	printf("server\n");
	for(;;){
		//printf("1\n");
		usleep(500*1000);
	}
	return 0;
}
