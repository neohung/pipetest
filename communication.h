#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__
#include <stdio.h>
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

//static struct COMMUNICATION_HOST* Singleton_instance_COMMUNICATION_HOST;
//static struct COMMUNICATION_SLAVE* Singleton_instance_COMMUNICATION_SLAVE;

void communication_host_start(char* pipe_name, char* rsp_name);
void communication_slave_start(char* pipe_name, char* rsp_name);
void communication_host_send(unsigned char* data, int size);
int communication_slave_read(unsigned char* data, int timeout_ms);
void communication_slave_send_rsp(unsigned char* data, int size);
int communication_host_read_rsp(unsigned char* data, int timeout_ms);
//struct COMMUNICATION* getCommunicationSingleton();

#endif