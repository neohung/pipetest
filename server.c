#include <stdio.h>
#include <errno.h>   // errno
#include <fcntl.h>   // open()
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>   //usleep
#include "communication.h"
pthread_t threadid;

int main(void)
{
	
    communication_slave_start("host_to_slave.pipe", "respone.pipe");
    unsigned char buffer[300];
    for(;;){
      int size = communication_slave_read(buffer,100);
      if (size){
      	printf("szie=%d: \n",size);
      	for(int index=0;index<size;index++){
      		printf("0x%X ", buffer[index]);
      	}
        printf("\n");
        //
        unsigned char data[] = {5,6,7,8};
        communication_slave_send_rsp( data, 4);
      }
    }
	return 0;
}
