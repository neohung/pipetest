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

#define PAYLOAD_START 0xAA
#define PAYLOAD_STOP 0x55


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
	COMMUNICATION_ERR_RSP_TIMEOUT=0,  // 
	COMMUNICATION_ERR_CMD,      // command is not recognized
	COMMUNICATION_ERR_DATA,     // data form error
	COMMUNICATION_ERR_LENGTH,   // data length is out of range
	COMMUNICATION_ERR_INVAILD_CMD,  //cant do this now 
	COMMUNICATION_ERR_UNK,       // unknow error
	COMMUNICATION_RET_SUCCESS
};

enum FSM{
  NORMAL = 5, 
  COMMUNICATION_IDLE = 7,
  SAVE_BINARY_FILE = 10
};

enum COMMUNICATION_COMMAND_TYPE{
  ENTER_COMMUNICATION,
  EXIT_COMMUNICATION,  //reset device
  SYNC_COMMUNICATION,  //clean buffer and reset flag
  SET_FSM,  //finite-state machine
  GET_FSM,
  SEND_STRING,
  SEND_BINARY_SIZE,
  SEND_BINARY_DATA,
  SEND_BINARY_DATA_CHECKSUM,
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

enum COMMUNICATION_STATUS host_send_enter_communication(unsigned int* jtagid, unsigned char* revision, unsigned int* version);

void  slave_rsp_enter_communication(enum COMMUNICATION_STATUS status , unsigned int jtagid, unsigned char revision, unsigned int version);

void host_send_exit_communication(void);

enum COMMUNICATION_STATUS host_get_fsm(unsigned char *fsm);

enum COMMUNICATION_STATUS host_set_fsm(unsigned char fsm);
enum COMMUNICATION_STATUS host_send_string(char* data, int size);
enum COMMUNICATION_STATUS host_send_binary_size(int size);
enum COMMUNICATION_STATUS host_send_binary_data(char* data,int size);
enum COMMUNICATION_STATUS host_send_binary_crc(short crc);


void slave_rsp_nodata(enum COMMUNICATION_STATUS status);
void slave_rsp_char_data(enum COMMUNICATION_STATUS status, char data);
void slave_rsp_short_data(enum COMMUNICATION_STATUS status, short data);
void slave_rsp_int_data(enum COMMUNICATION_STATUS status, int data);
void slave_rsp_data(enum COMMUNICATION_STATUS status, unsigned char* data, int size);
void host_send_data(enum COMMUNICATION_COMMAND_TYPE cmd, unsigned char* data, int size);
void host_send_int_data(enum COMMUNICATION_COMMAND_TYPE cmd, int data);
void host_send_short_data(enum COMMUNICATION_COMMAND_TYPE cmd, short data);
void host_send_char_data(enum COMMUNICATION_COMMAND_TYPE cmd, char data);
void host_send_nodata(enum COMMUNICATION_COMMAND_TYPE cmd);


int fetchData(unsigned char* payload, unsigned char* dst);
void set_checksum(unsigned char* payload, int size);
bool verify_checksum(unsigned char* payload, int size);
char fetchCommandType(unsigned char* payload, int size);
enum COMMUNICATION_STATUS fetchResponeStatus(unsigned char* payload, int size);
int fetchDataLength(unsigned char* payload, int size);
char fetchCharData(unsigned char* payload, int size);
short fetchShortData(unsigned char* payload, int size);
int fetchIntData(unsigned char* payload, int size);
#endif
