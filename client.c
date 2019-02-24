#include <stdio.h>
#include <stdlib.h>
#include <errno.h>   // errno
#include <fcntl.h>   // open()
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>   //usleep
#include <string.h>  //memcpy
#include "communication.h"

int main(void)
{
	unsigned char buffer[300];
	communication_host_start("host_to_slave.pipe", "respone.pipe");
	unsigned char data[] = {1,2,3,4};
    communication_host_send(data, 4);
    int size = communication_host_read_rsp(buffer, 500);
    if (size){
      	printf("rsp szie=%d: \n",size);
      	for(int index=0;index<size;index++){
      		printf("0x%X ", buffer[index]);
      	}
        printf("\n");
    }else{
    	printf("error communication_host_read_rsp timeout\n");
    	exit(-1);
    }
	return 0;
}
