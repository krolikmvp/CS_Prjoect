#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h> 
#include <stdlib.h>
#include <getopt.h>
#include <inttypes.h>
#define _cd "cd"
#define _ls "ls"
#define _pwd "pwd"
#define _cat "cat"
#define _exit "exit"
#define BUFF_SIZE 5242880
#define TEMP_BUFF_SIZE 128

enum msg_type{
   CD=1,
   PWD,
   CAT,
   LS
};

//const char * program_name;
/*
void print_usage (FILE* stream, int exit_code) 
{

 fprintf (stream, "Usage: %s options [ inputfile .... ]\n", program_name); 

 fprintf (stream, 

          "  -h  --help            Display this usage information.\n" 

          "  -i  --ip <ip_adress>          IP of the server you want connect.\n" 

          "  -p  --port <server_port>        Port of the server you want connect.\n"); 

 exit (exit_code); 

}*/

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

int validate_send(char *);
int write_to_srv(int fd, char * buff, int msg_type)
{

    uint8_t *buffer;
    int pos=0;
    size_t size=sizeof(int);

    switch(msg_type){

    case 1://cd
           size+=sizeof(char)*( strlen(buff)-strlen(_cd) )-2; //-2 bo \n i cd_
           buffer=malloc(size);
           memcpy(buffer,&msg_type,sizeof(int));
           pos+=sizeof(int);
           memcpy(buffer+pos,buff+(strlen(_cd)+1),strlen(buff+strlen(_cd)+1) );
           break;
    case 2://pwd
           buffer=malloc(size);
           memcpy(buffer,&msg_type,size);
           break;
    case 3://cat
	   size+=sizeof(char)*(strlen(buff)-strlen(_cat))-2; //-2 bo \n i cat_
           buffer=malloc(size);
           memcpy(buffer,&msg_type,sizeof(int));
           pos+=sizeof(int);
           memcpy(buffer+pos,buff+(strlen(_cat)+1),strlen(buff+strlen(_cat)+1) );
	   break;
    case 4://ls
           buffer=malloc(size);
           memcpy(buffer,&msg_type,size);
           break;

    }

  write(fd,&size,sizeof(size_t));
  write(fd,buffer,size);

  free(buffer);


}

