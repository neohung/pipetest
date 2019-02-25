#include <stdio.h>
#include <errno.h>   // errno
#include <fcntl.h>   // open()
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>   //usleep
#include <sys/time.h> //timersub
#include "communication.h"

#define JTAGID 0x01030507
#define REVISION 1
#define MAJOR 1
#define MINOR 0
#define PATCH 2
#define VERSION MAJOR << 16 | MINOR << 8 | PATCH

static struct timeval tv_start;

enum FSM fsm;

pthread_t threadid;

void enter_communication(void)
{
    fsm = COMMUNICATION_IDLE;
	printf("ENTER_COMMUNICATION!!\n");
	//
	 //response
    slave_rsp_enter_communication(COMMUNICATION_RET_SUCCESS , JTAGID , REVISION, VERSION);
}

void exit_communication(void)
{
  //Usually need to reset device
  fsm = NORMAL;
  gettimeofday(&tv_start, NULL);
  printf("EXIT_COMMUNICATION!!\n");
}

void get_fsm(void)
{
    printf("GET_FSM %d\n", fsm);
    slave_rsp_char_data(COMMUNICATION_RET_SUCCESS , fsm);
}
void normal_fsm_process(void)
{
  unsigned char buffer[300];
  int size = communication_slave_read(buffer,100);
  if (size){
    if (verify_checksum(buffer, size)){
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
    }
  }   
}

void set_fsm(unsigned char current_fsm, unsigned char target_fsm)
{
  printf("Change fsm from %d to %d\n", current_fsm, target_fsm);
  fsm = target_fsm;
  slave_rsp_nodata(COMMUNICATION_RET_SUCCESS);
}

void communication_idle_fsm_process(void)
{
  unsigned char buffer[300];
  int size = communication_slave_read(buffer,100);
  if (size){
    if (verify_checksum(buffer, size)){
      switch (fetchCommandType(buffer, size)){
        case EXIT_COMMUNICATION:
        {                
          exit_communication();
        }                
        break;
        case GET_FSM:
        {
          get_fsm();
        }
        break;
        case SET_FSM:
        {
          if (fetchDataLength(buffer,size) == 1){
            set_fsm(fsm, fetchCharData(buffer,size));
            
          }else{
            printf("SET_FSM data length error\n");
            exit(-1);
          }
        }
        break;
        default:
          printf("communication_idle: UNKNOW CMD\n");
        break;
      }      
    }
   
  }
  //
  
}

void save_binary_file_fsm_process(void)
{
   unsigned char buffer[300];
   unsigned char filename[300];
   unsigned char binarydata[300];
   int filename_len = 0;
   int binary_data_size = 0;
   short binary_data_crc = 0;
   //fetch filename
   int size = communication_slave_read(buffer,100);
   if ((size) && (verify_checksum(buffer, size)) && 
         ((fetchCommandType(buffer, size))==SEND_STRING)){
     filename_len = fetchDataLength(buffer, size);
     fetchData(buffer, filename);
     filename[filename_len] = 0;
     slave_rsp_nodata(COMMUNICATION_RET_SUCCESS);
   }else{
     printf("save_binary_file_fsm_process error to get string %d\n", fetchCommandType(buffer, size));
     exit(-1);
   }
   //
   FILE *fp  = fopen(filename, "wb");
   if (!fp){
     printf("fail to create file [%s]\n", filename);
     exit(-1);
   }
   //fetch filesize
   size = communication_slave_read(buffer,100);
   if ((size) && (verify_checksum(buffer, size)) &&  
         ((fetchCommandType(buffer, size))==SEND_BINARY_SIZE)){
     binary_data_size = fetchIntData(buffer, size);
     slave_rsp_nodata(COMMUNICATION_RET_SUCCESS);
   }else{
     printf("save_binary_file_fsm_process error to get size\n");
     exit(-1);
   }   
   //fetch data
   for(;;){
     size = communication_slave_read(buffer,100);
     if ((size) && (verify_checksum(buffer, size))){
       char type = fetchCommandType(buffer, size);
       if (type==SEND_BINARY_DATA){
         int data_len = fetchDataLength(buffer, size);
         fetchData(buffer, binarydata);
         fwrite(binarydata,1,data_len, fp);
         slave_rsp_nodata(COMMUNICATION_RET_SUCCESS);
       } else if (type==SEND_BINARY_DATA_CHECKSUM){
         binary_data_crc = fetchShortData(buffer, size);
         slave_rsp_nodata(COMMUNICATION_RET_SUCCESS);
         break;   
       }
    }
   }
   //
   fclose(fp);
   // change fsm from SAVE_BINARY_FILE to COMMUNICATION_IDLE
   fsm = COMMUNICATION_IDLE;
}
int main(void)
{
    fsm = NORMAL;
    communication_slave_start("host_to_slave.pipe", "respone.pipe");
    unsigned char buffer[300];
    static struct timeval tv_end;
    static struct timeval diff_tv;
    gettimeofday(&tv_start, NULL);
    for(;;){
      gettimeofday(&tv_end,NULL);
      timersub(&tv_end, &tv_start, &diff_tv);
      //printf("diff_tv.tv_sec=%ld\n",diff_tv.tv_sec);
      if ((fsm == NORMAL ) && (diff_tv.tv_sec > 5))
        break;
      //
      switch (fsm){
        case NORMAL:
        {
          normal_fsm_process();
        }
        break;
        case COMMUNICATION_IDLE:
        {
          communication_idle_fsm_process();
        }
        break;
        case SAVE_BINARY_FILE:
        {
          save_binary_file_fsm_process();
        }
        break;
        default:
          printf("UNKNOW FSM ERROR\n");
        break;
      }
    }
    printf("5s after\n");
    return 0;
}

