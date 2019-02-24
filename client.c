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
    host_send_enter_communication();

	return 0;
}
