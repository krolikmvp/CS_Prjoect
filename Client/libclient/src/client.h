#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <inttypes.h>
#include <errno.h>
#include <unistd.h>
#define M_TYPE 1
#define M_SIZE sizeof(int)
#define C_SIZE sizeof(uint16_t)
#define _cd "cd"
#define _ls "ls"
#define _pwd "pwd"
#define _cat "cat"
#define _exit "exit"
#define ERRORMSG 10
#define TEMP_BUFF_SIZE 128
#define MTU_SIZE 1500
#define cpmv(a, b ,c, d) memcpy(a+c,&b,d);c+=d


enum msg_type{
   CD=1,
   PWD,
   CAT,
   LS,
   ERROR
};

typedef struct message_structure {

   int head;
   int size;
   int elements;
   char **content;
   char *string;

} msg;

const struct option long_options[]={

	{"ip", 1 , NULL , 'i' },
	{"port", 1 , NULL , 'p' },
	{"help", 0 , NULL , 'h' },
	{ NULL , 0, NULL , 0}

};


void error(char *msg)
{
    perror(msg);
    exit(0);
}

void print_usage (FILE*, int,char *);
uint8_t validate_send(char *);
void print_message(msg * );
void write_to_srv(int, char *, uint8_t);
void read_from_server(int);
