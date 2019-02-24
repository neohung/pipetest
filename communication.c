#include "communication.h"

#define SINGLETON(t)  \
     static struct t* Singleton_instance_##t; \
     static struct t* getSingleton_##t() { \
     	if (!Singleton_instance_##t){ \
     		 Singleton_instance_##t = malloc(sizeof(struct t)); \
     	} \
     	return Singleton_instance_##t; \
     } \

SINGLETON(COMMUNICATION_HOST);
SINGLETON(COMMUNICATION_SLAVE);

static int read_from_fd(void* fd, unsigned char* data, int timeout_ms);
static void write_to_fd(void* fd, unsigned char* data, int size);

void communication_slave_start(char* pipe_name, char* rsp_name)
{

	getSingleton_COMMUNICATION_SLAVE()->readdata = &read_from_fd;
	getSingleton_COMMUNICATION_SLAVE()->writedata = &write_to_fd;
	int ret = mkfifo(pipe_name,0666);
	if ((ret == -1) && (errno |= EEXIST)){
		perror("Fail to create file\n");
	}
	ret = mkfifo(rsp_name,0666);
	if ((ret == -1) && (errno |= EEXIST)){
		perror("Fail to create file\n");
	}
	getSingleton_COMMUNICATION_SLAVE()->receive_id = open(pipe_name, O_RDONLY | O_NONBLOCK);
	if (getSingleton_COMMUNICATION_SLAVE()->receive_id == -1){
		printf("ERROR to open %s\n", pipe_name);
		exit(-1);
	}
	getSingleton_COMMUNICATION_SLAVE()->rsp_send_id = open(rsp_name, O_WRONLY | O_NONBLOCK);
	while(getSingleton_COMMUNICATION_SLAVE()->rsp_send_id == -1){
		getSingleton_COMMUNICATION_SLAVE()->rsp_send_id = open(rsp_name, O_WRONLY | O_NONBLOCK);
		//printf("Wait to open %s\n", rsp_name);
		usleep(1000*10);
	}
	printf("Ready to communication_slave_start()\n");
}

void communication_host_start(char* pipe_name, char* rsp_name)
{

	getSingleton_COMMUNICATION_HOST()->readdata = &read_from_fd;
	getSingleton_COMMUNICATION_HOST()->writedata = &write_to_fd;
	int ret = mkfifo(pipe_name,0666);
	if ((ret == -1) && (errno |= EEXIST)){
		perror("Fail to create file\n");
	}
	ret = mkfifo(rsp_name,0666);
	if ((ret == -1) && (errno |= EEXIST)){
		perror("Fail to create file\n");
	}
	getSingleton_COMMUNICATION_HOST()->send_id = open(pipe_name, O_WRONLY | O_NONBLOCK);
	while(getSingleton_COMMUNICATION_HOST()->send_id  == -1){
		getSingleton_COMMUNICATION_HOST()->send_id = open(pipe_name, O_WRONLY | O_NONBLOCK);
		//printf("Wait to open %s\n", pipe_name);
		usleep(1000*10);
	}
	getSingleton_COMMUNICATION_HOST()->rsp_receive_id = open(rsp_name, O_RDONLY | O_NONBLOCK);
	if (getSingleton_COMMUNICATION_HOST()->rsp_receive_id == -1){
		printf("ERROR to open %s\n", rsp_name);
		exit(-1);
	}
    printf("Ready to communication_host_start()\n");
}

void communication_host_send(unsigned char* data, int size)
{
	getSingleton_COMMUNICATION_HOST()->writedata(&(getSingleton_COMMUNICATION_HOST()->send_id), data, size);
}

int communication_host_read_rsp(unsigned char* data, int timeout_ms)
{
	int size = getSingleton_COMMUNICATION_HOST()->readdata(&(getSingleton_COMMUNICATION_HOST()->rsp_receive_id), getSingleton_COMMUNICATION_HOST()->receive_buffer, timeout_ms);
    if ((data) && (size) && (size < MAX_BUFFER_SIZE)) 
    	memcpy(data, getSingleton_COMMUNICATION_HOST()->receive_buffer, size);
    return size;
}

int communication_slave_read(unsigned char* data, int timeout_ms)
{
	int size = getSingleton_COMMUNICATION_SLAVE()->readdata(&(getSingleton_COMMUNICATION_SLAVE()->receive_id), getSingleton_COMMUNICATION_SLAVE()->receive_buffer, timeout_ms);
    if ((data) && (size) && (size < MAX_BUFFER_SIZE)) 
    	memcpy(data, getSingleton_COMMUNICATION_SLAVE()->receive_buffer, size);
    return size;
}

void communication_slave_send_rsp(unsigned char* data, int size)
{
	getSingleton_COMMUNICATION_SLAVE()->writedata(&(getSingleton_COMMUNICATION_SLAVE()->rsp_send_id), data, size);
}

static void write_to_fd(void* fd, unsigned char* data, int size)
{
	int writefd = (*(int*)fd);
	write(writefd, data, size);
}

static int read_from_fd(void* fd, unsigned char* data, int timeout_ms)
{
	unsigned char readbuffer[MAX_BUFFER_SIZE];
	int readfd = (*(int*)fd);
	fd_set read_set;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000*timeout_ms; //timeout_ms ms
	int err;
	int size = 0;
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
	}
		//usleep(500*1000);
	return size;
}

void set_checksum(unsigned char* payload, int size)
{
	int sum = 0;
	//6
	for(int i=1; i< 1+size-4; i++){
		sum += payload[i];
	}
	int checksum = 0xFFFF - sum;
	payload[size-3] = (char)((checksum & 0xFF00 )>> 8);
	payload[size-2] = (char)(checksum & 0x00FF);
}

bool verify_checksum(unsigned char* payload, int size)
{
	int sum = 0;
	//6
	for(int i=1; i< 1+size-4; i++){
		sum += payload[i];
	}
	int checksum = ((payload[size-3]  << 8 ) & 0xFF00) | payload[size-2] ;
	sum += checksum;
	if (sum == 0xFFFF){
		return true;
	}else{
		return false;
	}
}

char fetchCommandType(unsigned char* payload, int size)
{
	return payload[1];
}

int fetchDataLength(unsigned char* payload, int size)
{
	int len = ((payload[3]  << 8 ) & 0xFF00) | payload[2] ;
	return len;
}

void host_send_enter_communication()
{
	unsigned char buffer[300];
	unsigned char payload[] = {PAYLOAD_START,ENTER_COMMUNICATION, 0,0,0,0, PAYLOAD_STOP};
	set_checksum(payload, sizeof(payload));
	communication_host_send(payload, sizeof(payload));
	int size = communication_host_read_rsp(buffer, 100);
	if (size){
		printf("Got %d\n",size);
	}else{
		printf("error host_send_enter_communication\n");
		exit(-1);
	}
}
