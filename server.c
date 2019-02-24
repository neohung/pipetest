#include <stdio.h>
#include <errno.h>   // errno
#include <fcntl.h>   // open()
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>   //usleep
#include "communication.h"
pthread_t threadid;

void enter_communication(void)
{
	printf("ENTER_COMMUNICATION!!\n");
	//
	 //response
    unsigned char data[] = {1,2,3,4};
    communication_slave_send_rsp(data, 4);
}

int main(void)
{	
    communication_slave_start("host_to_slave.pipe", "respone.pipe");
    unsigned char buffer[300];
    for(;;){
      int size = communication_slave_read(buffer,100);
      //printf("size %d\n",size);
      if (size){
      	if (verify_checksum(buffer, size)){
      		//printf("CRC OK\n");
      		switch (fetchCommandType(buffer, size)){
      			case ENTER_COMMUNICATION:
      			{
      			  enter_communication();
      			}
      			break;
      			default:
      			  printf("UNKNOW CMD\n");
      			break;
      	    }
      	}else{
      		printf("CRC ERROR\n");
      		exit(-1);
      	}
      	//switch (fetchCommandType(buffer, size)){

      	//}
      	/*
      	printf("szie=%d: \n",size);
      	for(int index=0;index<size;index++){
      		printf("0x%X ", buffer[index]);
      	}
        printf("\n");
        unsigned char data[] = {5,6,7,8};
        communication_slave_send_rsp( data, 4);
        */
      }
    }
	return 0;
}
