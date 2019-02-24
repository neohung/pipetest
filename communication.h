#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__
#include <stdio.h>
#include <stdbool.h>
//#include <string.h>
#include <stdlib.h>

#include <errno.h>   // errno
#include <fcntl.h>   // open()
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>   //usleep
#include <string.h>  //memcpy
/*
#define SINGLETON(t, inst, init) t* Singleton_##t() { \
                 static t inst = init;               \
                 return &inst;                       \
                }
*/

#define MAX_BUFFER_SIZE 300

#define PAYLOAD_START 0x01
#define PAYLOAD_STOP 0x17


struct COMMUNICATION_HOST
{
    int send_id;
    int rsp_receive_id;
    unsigned char receive_buffer[MAX_BUFFER_SIZE];
    int (*readdata)(void* fd, unsigned char* data, int timeout_ms);
    void (*writedata)(void* fd, unsigned char* data, int size);
};

struct COMMUNICATION_SLAVE
{
    int receive_id;
    int rsp_send_id;
    unsigned char receive_buffer[MAX_BUFFER_SIZE];
    int (*readdata)(void* fd, unsigned char* data, int timeout_ms);
    void (*writedata)(void* fd, unsigned char* data, int size);
};


enum COMMUNICATION_STATUS{
	COMMUNICATION_RET_SUCCESS = 0,
	COMMUNICATION_ERR_CMD,      // command is not recognized
	COMMUNICATION_ERR_DATA,     // data form error
	COMMUNICATION_ERR_LENGTH,   // data length is out of range
	COMMUNICATION_ERR_UNK       // unknow error
};

enum COMMUNICATION_COMMAND_TYPE{
	ENTER_COMMUNICATION,
    EXIT_COMMUNICATION
};

//static struct COMMUNICATION_HOST* Singleton_instance_COMMUNICATION_HOST;
//static struct COMMUNICATION_SLAVE* Singleton_instance_COMMUNICATION_SLAVE;

void communication_host_start(char* pipe_name, char* rsp_name);
void communication_slave_start(char* pipe_name, char* rsp_name);
void communication_host_send(unsigned char* data, int size);
int communication_slave_read(unsigned char* data, int timeout_ms);
void communication_slave_send_rsp(unsigned char* data, int size);
int communication_host_read_rsp(unsigned char* data, int timeout_ms);
//struct COMMUNICATION* getCommunicationSingleton();

void host_send_enter_communication();
void set_checksum(unsigned char* payload, int size);
bool verify_checksum(unsigned char* payload, int size);
char fetchCommandType(unsigned char* payload, int size);
int fetchDataLength(unsigned char* payload, int size);
#endif