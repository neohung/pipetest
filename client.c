#include <stdio.h>
#include <errno.h>   // errno
#include <fcntl.h>   // open()
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>   //usleep
#include <string.h>  //memcpy
int readdata(void* fd, unsigned char* data)
{
	unsigned char readbuffer[300];
	int readfd = (*(int*)fd);
	fd_set read_set;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000*10; //10ms
	int err;
	int size = 0;
	int i;
	for(i=0;i<5;i++)
	{
		FD_ZERO(&read_set);
		FD_SET(readfd, &read_set);
		err = select(readfd+1, &read_set,NULL,NULL,&tv);
		if ((err > 0) && (FD_ISSET(readfd, &read_set))){
			FD_CLR(readfd, &read_set);
			size = read(readfd , &readbuffer, sizeof(readbuffer));
			if ((data) && (size)) {
				//printf("got size %d,%d,%d,%d,\n",readbuffer[0],readbuffer[1],readbuffer[2],readbuffer[3]);
				memcpy(data,readbuffer,size);
			}
			//printf("read %d\n",size);
			break;
		}
		//usleep(500*1000);
	}
	return size;
}

int main(void)
{
	unsigned char buf[300];
	int size;
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
	//need to read first
	int writefd = open(name, O_WRONLY | O_NONBLOCK);
	int readfd = open(rsp_name, O_RDONLY | O_NONBLOCK);
	printf("client\n");
	int c=0;
	for(;;){
		c++;
		write(writefd, (int*)(&c), sizeof(c));
		printf("send %d\n",c);
		size = readdata(&readfd , buf);
		int resp  = 0;
		if (size == 4){
            //error --> resp =  (buf[0]) | (buf[1]<<8) | (buf[2]<<16) |  (buf[3]<<24);
            //error -->  resp = buf[0] + (buf[1]*256) + (buf[2]*65536) + (buf[3]*16777216);
            resp =  ((buf[3]<<24) & 0xFF000000)| ((buf[2]<<16)& 0xFF0000)| ((buf[1]<<8)& 0xFF00) | ((buf[0])& 0xFF);
            //printf("buf size %d,%d,%d,%d,\n",buf[0],buf[1],buf[2],buf[3]);
		   printf("resp=%d\n",resp);
		}
		
		usleep(100*1000);
	}
	return 0;
}
