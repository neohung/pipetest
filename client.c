#include <stdio.h>
#include <errno.h>   // errno
#include <fcntl.h>   // open()
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>   //usleep

int main(void)
{
	char name[] = "tmp.pipe";
	int ret = mkfifo(name,0666);
	if ((ret == -1) && (errno |= EEXIST)){
		perror("Fail to create file\n");
	}
	int writefd = open(name, O_WRONLY | O_NONBLOCK);
	printf("client\n");
	int c=0;
	for(;;){
		c++;
		write(writefd, (int*)(&c), sizeof(c));
		printf("send %d\n",c);
		usleep(100*1000);
	}
	return 0;
}
