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
        int i;
	for(i=1; i< 1+size-4; i++){
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
        int i;
	for(i=1; i< 1+size-4; i++){
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

enum COMMUNICATION_STATUS fetchResponeStatus(unsigned char* payload, int size)
{
	return payload[1];
}

char fetchCharData(unsigned char* payload, int size)
{
	return payload[4];
}

short fetchShortData(unsigned char* payload, int size)
{
  short data = (payload[5] << 8) | payload[4];
  return data;
}

int fetchIntData(unsigned char* payload, int size)
{
  int data = (payload[7]<<24) | (payload[6]<<16) | (payload[5] << 8) | payload[4];
  return data;
}

int fetchData(unsigned char* payload, unsigned char* dst)
{
  if ((payload == NULL) || (dst==NULL)){
    printf("ERROR fetchData NULL pointer\n");
    exit(-1);
  }
  int len = ((payload[3]  << 8 ) & 0xFF00) | payload[2] ;
  if ((len < 0) || (len > MAX_BUFFER_SIZE)){
    printf("ERROR fetchData size %d error\n", len);
    exit(-1);
  }
  memcpy(dst,payload+4,len);
  return len;
}

enum COMMUNICATION_STATUS host_send_enter_communication(unsigned int* jtagid, unsigned char* revision, unsigned int* version)
{
	unsigned char buffer[300];
        host_send_nodata(ENTER_COMMUNICATION);
	int size = communication_host_read_rsp(buffer, 100);
	if ((size) && (verify_checksum(buffer,size))){
          enum COMMUNICATION_STATUS status = fetchResponeStatus( buffer, size);
          switch (status)
          {
      case COMMUNICATION_ERR_RSP_TIMEOUT:
      {
         printf("host_send_enter_communication timeout\n");
         exit(-1);
      }
      break;
            case COMMUNICATION_RET_SUCCESS:
              printf("host_send_enter_communication SUCCESS\n");
              //printf("%X,%X,%X,%X\n",  buffer[11],  buffer[12],  buffer[13],  buffer[14]);
              *jtagid = (buffer[8] << 24) | (buffer[7] << 16) | (buffer[6] << 8) | buffer[5];
              *revision = buffer[9];
              *version = (buffer[12] << 16) | (buffer[11] << 8) | buffer[10];
            break;
            default:
              printf("host_send_enter_communication: status %d\n", status);
            break;
          }
          return status;
	}else{
            if (size == 0){
            //return COMMUNICATION_ERR_RSP_TIMEOUT;
		printf("host_send_enter_communication timeout\n");
              exit(-1);
            }else{
		printf("check sum error host_send_enter_communication\n");
                exit(-1);
            }
	}
}
void host_send_exit_communication(void)
{
  host_send_nodata(EXIT_COMMUNICATION);
}

void  slave_rsp_enter_communication(enum COMMUNICATION_STATUS status , unsigned int jtagid, unsigned char revision, unsigned int version)
{
  unsigned char payload[] = {PAYLOAD_START,status, 8,0 
  , status 
  , jtagid & 0x000000FF
  , (jtagid & 0x0000FF00) >> 8
  , (jtagid & 0x00FF0000) >> 16 
  , (jtagid & 0xFF000000) >> 24 
  , revision 
  , version & 0x000000FF
  , (version & 0x0000FF00) >> 8
  , (version & 0x00FF0000) >> 16 
  ,0,0, PAYLOAD_STOP};
  set_checksum(payload, sizeof(payload));
  communication_slave_send_rsp(payload, sizeof(payload));
}


enum COMMUNICATION_STATUS host_get_fsm(unsigned char *fsm)
{
	unsigned char buffer[300];
        host_send_nodata(GET_FSM);
	int size = communication_host_read_rsp(buffer, 100);
	if ((size) && (verify_checksum(buffer,size))){
          enum COMMUNICATION_STATUS status = fetchResponeStatus( buffer, size);
          switch (status)
          {
            case COMMUNICATION_RET_SUCCESS:
            {
              if (fetchDataLength(buffer, size) == 1){  
                printf("host_get_fsm SUCCESS\n");
                *fsm = fetchCharData(buffer, size);
              }else{
                printf("host_get_fsm data length error\n");
                exit(-1);
              }
            }
            break;
            default:
              printf("host_send_enter_communication: status %d\n", status);
            break;
          }
          return status;

	}else{
           if (size == 0){ 
            //return COMMUNICATION_ERR_RSP_TIMEOUT;
              printf("host_get_fsm timeout\n");
              exit(-1);
            }else{
                printf("check sum or size error host_get_fsm\n");
                exit(-1);
            }   

	}
  
}

enum COMMUNICATION_STATUS host_set_fsm(unsigned char fsm)
{
        unsigned char buffer[300];
        host_send_char_data(SET_FSM, fsm);
        int size = communication_host_read_rsp(buffer, 100);
        if ((size) && (verify_checksum(buffer,size))){
          enum COMMUNICATION_STATUS status = fetchResponeStatus( buffer, size);
          switch (status)
          {   
      case COMMUNICATION_ERR_RSP_TIMEOUT:
      {
         printf("host_set_fsm timeout\n");
         exit(-1);
      }
      break;
            case COMMUNICATION_RET_SUCCESS:
            {   
              printf("host_set_fsm to %d pass\n", fsm);
            }   
            break;
            default:
              printf("host_send_enter_communication: status %d\n", status);
            break;
          }   
          return status;

        }else{
            if (size == 0){
            //return COMMUNICATION_ERR_RSP_TIMEOUT;
              printf("host_set_fsm timeout\n");
              exit(-1);
            }else{
                printf("check sum or size error host_set_fsm\n");
                exit(-1);
            }
        }   

}


enum COMMUNICATION_STATUS host_send_string(char* data, int size)
{
  unsigned char buffer[300];
  host_send_data(SEND_STRING , data, size);
  size = communication_host_read_rsp(buffer, 100);
  if ((size) && (verify_checksum(buffer,size))){
    enum COMMUNICATION_STATUS status = fetchResponeStatus( buffer, size);
    switch (status)
    {
      case COMMUNICATION_RET_SUCCESS:
      {
         printf("host_send_string pass\n");
      }
      break;
      default:
        printf("host_send_string: status %d\n", status);
        break;
    }
    return status;
  }
  printf("host_send_string timeout\n");
  exit(-1);
   
}

enum COMMUNICATION_STATUS host_send_binary_size(int size)
{
  unsigned char buffer[300];
  //SEND_BINARY_SIZE
  host_send_int_data(SEND_BINARY_SIZE,size);
  size = communication_host_read_rsp(buffer, 100);
  if ((size) && (verify_checksum(buffer,size))){
    enum COMMUNICATION_STATUS status = fetchResponeStatus( buffer, size);
    switch (status)
    {   
      case COMMUNICATION_RET_SUCCESS:
      {   
         printf("host_send_binary_size pass\n");
      }   
      break;
      default:
        printf("host_send_binary_size: status %d\n", status);
        break;
    }  
    return status;
  }
  //return COMMUNICATION_ERR_RSP_TIMEOUT;
  printf("host_send_binary_size timeout\n");
  exit(-1);
}

enum COMMUNICATION_STATUS host_send_binary_crc(short crc)
{
  unsigned char buffer[300];
  host_send_short_data(SEND_BINARY_DATA_CHECKSUM,crc);
  int size = communication_host_read_rsp(buffer, 100);
  if ((size) && (verify_checksum(buffer,size))){
    enum COMMUNICATION_STATUS status = fetchResponeStatus( buffer, size);
    switch (status)
    {
      case COMMUNICATION_RET_SUCCESS:
      {
         printf("host_send_binary_crc pass\n");
      }
      break;
      default:
        printf("host_send_binary_crc: status %d\n", status);
        break;
    }    
    return status;
  }
  //return COMMUNICATION_ERR_RSP_TIMEOUT;
  printf("host_send_binary_crc timeout\n");
  exit(-1);
}

enum COMMUNICATION_STATUS host_send_binary_data(char* data,int size)
{
  unsigned char buffer[300];
  host_send_data(SEND_BINARY_DATA , data, size);
  size = communication_host_read_rsp(buffer, 100);
  if ((size) && (verify_checksum(buffer,size))){
    enum COMMUNICATION_STATUS status = fetchResponeStatus( buffer, size);
    switch (status)
    {
      case COMMUNICATION_RET_SUCCESS:
      {
         printf("host_send_binary_data pass\n");
      }
      break;
      default:
        printf("host_send_binary_data: status %d\n", status);
        break;
    }
    return status;
  }
  //return COMMUNICATION_ERR_RSP_TIMEOUT;
  printf("host_send_binary_data timeout\n");
  exit(-1);
}
 

//=================================================================
void host_send_nodata(enum COMMUNICATION_COMMAND_TYPE cmd)
{
  unsigned char payload[] = {PAYLOAD_START,cmd, 0,0 ,0,0, PAYLOAD_STOP};
  set_checksum(payload, sizeof(payload));
  communication_host_send(payload, sizeof(payload));
}


void host_send_char_data(enum COMMUNICATION_COMMAND_TYPE cmd, char data)
{
  unsigned char payload[] = {PAYLOAD_START,cmd, 1,0
  , (unsigned char) data 
  ,0,0, PAYLOAD_STOP};
  set_checksum(payload, sizeof(payload));
  communication_host_send(payload, sizeof(payload));
}

void host_send_short_data(enum COMMUNICATION_COMMAND_TYPE cmd, short data)
{
  unsigned char payload[] = {PAYLOAD_START,cmd, 2,0
  , (unsigned char) (data & 0xFF) 
  , (unsigned char) ((data & 0xFF00) >> 8)
  ,0,0, PAYLOAD_STOP};
  set_checksum(payload, sizeof(payload));
  communication_host_send(payload, sizeof(payload));
}

void host_send_int_data(enum COMMUNICATION_COMMAND_TYPE cmd, int data)
{
  unsigned char payload[] = {PAYLOAD_START,cmd, 4,0
  , (unsigned char) (data & 0xFF) 
  , (unsigned char) ((data & 0xFF00) >> 8)
  , (unsigned char) ((data & 0xFF0000) >> 16)
  , (unsigned char) ((data & 0xFF000000) >> 24)
  ,0,0, PAYLOAD_STOP};
  set_checksum(payload, sizeof(payload));
  communication_host_send(payload, sizeof(payload));
}


void host_send_data(enum COMMUNICATION_COMMAND_TYPE cmd, unsigned char* data, int size)
{
  unsigned char payload[300];
  payload[0] = PAYLOAD_START;
  payload[1] = cmd;
  payload[2] = size & 0xFF;
  payload[3] = (size & 0xFF00) >> 8;
  memcpy(payload+4,data, size);
  payload[size+7] = PAYLOAD_STOP;
  set_checksum(payload, size+7);
  communication_host_send(payload, size+7);
}

void slave_rsp_nodata(enum COMMUNICATION_STATUS status)
{
  unsigned char payload[] = {PAYLOAD_START,status, 0,0 ,0,0, PAYLOAD_STOP};
  set_checksum(payload, sizeof(payload));
  communication_slave_send_rsp(payload, sizeof(payload));
}
void slave_rsp_char_data(enum COMMUNICATION_STATUS status, char data)
{
  unsigned char payload[] = {PAYLOAD_START,status, 1,0 
  , (unsigned char) (data) 
  ,0,0, PAYLOAD_STOP};
  set_checksum(payload, sizeof(payload));
  communication_slave_send_rsp(payload, sizeof(payload));
}
void slave_rsp_short_data(enum COMMUNICATION_STATUS status, short data)
{
  unsigned char payload[] = {PAYLOAD_START,status, 2,0 
  , (unsigned char) (data & 0xFF) 
  , (unsigned char) ((data & 0xFF00) >> 8)
  ,0,0, PAYLOAD_STOP};
  set_checksum(payload, sizeof(payload));
  communication_slave_send_rsp(payload, sizeof(payload));
}
void slave_rsp_int_data(enum COMMUNICATION_STATUS status, int data)
{
  unsigned char payload[] = {PAYLOAD_START,status, 4,0 
  , (unsigned char) (data & 0xFF) 
  , (unsigned char) ((data & 0xFF00) >> 8)
  , (unsigned char) ((data & 0xFF0000) >> 16)
  , (unsigned char) ((data & 0xFF000000) >> 24)
  ,0,0, PAYLOAD_STOP};
  set_checksum(payload, sizeof(payload));
  communication_slave_send_rsp(payload, sizeof(payload));
}
void slave_rsp_data(enum COMMUNICATION_STATUS status, unsigned char* data, int size)
{
  unsigned char payload[300];
  payload[0] = PAYLOAD_START;
  payload[1] = status;
  payload[2] = size & 0xFF;
  payload[3] = (size & 0xFF00) >> 8;
  memcpy(payload+4,data, size);
  payload[size+7] = PAYLOAD_STOP;
  set_checksum(payload, size+7);
  communication_slave_send_rsp(payload, size+7);
}
