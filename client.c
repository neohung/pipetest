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
        unsigned int jtagid, version;
        unsigned char revision;
        int ret  = host_send_enter_communication(&jtagid, &revision, &version);
        printf("jtagid=%X, r=%X, V=%X\n", jtagid, revision, version);
	sleep(1);
        char fsm;
        ret  = host_get_fsm(&fsm);
        printf("fsm=%d\n",fsm);
        sleep(1);
        ret  = host_set_fsm(SAVE_BINARY_FILE);
        char name[] = "ABCEFG";
        ret  = host_send_string(name, sizeof(name));
        ret  = host_send_binary_size(6);
        int a = '1';
        ret  = host_send_binary_data((char*)&a,1);
        a = '2';
        ret  = host_send_binary_data((char*)&a,1);
        a = '3';
        ret  = host_send_binary_data((char*)&a,1);
        a = '4';
        ret  = host_send_binary_data((char*)&a,1);
        a = '5';
        ret  = host_send_binary_data((char*)&a,1);
        ret  = host_send_binary_crc(6);
	sleep(1);
        ret  = host_set_fsm(COMMUNICATION_IDLE);
	sleep(1);
        host_send_exit_communication();
        return 0;
}
