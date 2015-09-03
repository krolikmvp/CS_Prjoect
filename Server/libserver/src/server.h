#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <limits.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <getopt.h>
#include <errno.h>
#include <inttypes.h>
#define M_TYPE 1
#define M_SIZE sizeof(int)
#define C_SIZE sizeof(uint16_t)
#define BUFF_SIZE 512
#define SMALL_BUFF 128
#define cpmv(a, b ,c, d) memcpy(a+c,&b,d);c+=d
#define MTU_SIZE 1500

enum msg_type{
   CD=1,
   PWD,
   CAT,
   LS,
   ERROR
};

enum cd_flags{

    CH_DIR=10,
    PREMISSION_ERROR = 17,     // "PREMISSION DENIED"
    EXIST_ERROR = 24,   // "DIRECTORY DOES NOT EXIST"
    FILE_ERROR = 19,    // "FILE DOES NOT EXIST"
    OUT_OF_MEMORY_ERROR = 13,  //"OUT OF MEMORY"
    UNKNOWN_ERROR = 18  //"UNKNOWN ERROR TYPE"
};

const struct option long_options[]={

	{"directory", 1 , NULL , 'd' },
	{"port", 1 , NULL , 'p' },
	{"help", 0 , NULL , 'h' },
	{ NULL , 0, NULL , 0}

};

typedef struct element_and_size{
	char* item;
	int size;
} es;

void execute_command(char* ,uint8_t *,int,uint8_t);
uint8_t set_msg_type( uint8_t *);
void print_usage (FILE*, int,char *);
void send_error_msg(int,int);
char* error_switch(int);
void process_cd(char*,uint8_t *, int);
void process_pwd(char*, int);
void process_cat(char*,uint8_t *, int);
void process_ls(int);
int send_message(int,size_t, uint8_t *);
void error(char *);


