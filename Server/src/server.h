#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <stddef.h>
#define BUFF_SIZE 256
#define BUFF_SIZE_OUT 5242880
#define _cd "cd "
#define  _ls "ls"
#define _pwd "pwd"
#define _cat "cat "

enum msg_type{
   CD=1,
   PWD,
   CAT,
   LS
};

int set_msg_type( uint8_t * msg ){

	int type=0;
	memcpy(&type,msg,sizeof(int));
	return type;

}

//int process_cd(uint8_t *, int);
int process_pwd(int);
int process_cat(char* directory,uint8_t *, int);
int process_ls(int);


int execute_command(char* directory ,uint8_t * command,int fd,int msg_type){


	switch(msg_type){

	//    case CD: process_cd(command,fd); break;
            case PWD: process_pwd(fd); break;
            case CAT: process_cat(directory,command,fd); break;
            case LS: process_ls(fd); break;

	}

/*

  if(!strncmp(command,_cd,strlen(_cd))){
	int status=chdir(command+strlen(_cd));
	char x[BUFF_SIZE];
	if(!status)
		getcwd(x,BUFF_SIZE);
	else
		strcpy(x,"No such file or directory");

        size_t size = strlen(x);
	send(fd,&size,sizeof(size),0);
	printf("%zu, bytes to send\n",size);
  	send(fd,x,size,0);
	return 0;

  }*/

/*
  FILE *fp;
  char *buffer=(char*)malloc(sizeof(char)*BUFF_SIZE_OUT);
  char temp[128];
  bzero(temp,128);
  bzero(buffer,BUFF_SIZE_OUT);
 
  fp = popen(command, "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }


  while (fgets(temp, sizeof(temp)-1, fp) != NULL) {
	strcat(buffer,temp);
	
  }

  if(strlen(buffer)==0)
	strcpy(buffer,"No such file or directory");
        
  if(fd>0){
        size_t size = strlen(buffer);
	send(fd,&size,sizeof(size),0);
	printf("%zu, bytes to send\n",size);
  	send(fd,buffer,size,0);
  }

  pclose(fp);
  free(buffer);
*/

}


int process_cat(char* directory, uint8_t *buffer, int fd)
{

	int msg_type=CAT;
	int pos=sizeof(int);
	int string_size=0;
	printf("current dir\n");
	printf("%s\n",directory);
	
	memcpy(&string_size,buffer+pos,sizeof(int));
	pos+=sizeof(int);

	char buf[string_size];

	memcpy(&buf,buffer+pos,string_size);

	printf("%s\n",buf);
	


}

int process_pwd(int fd)
{
	int msg_type=PWD;
        char directory_buff[BUFF_SIZE];
	bzero(directory_buff,BUFF_SIZE);
        getcwd(directory_buff,BUFF_SIZE-1);
	int buff_len=strlen(directory_buff);
	size_t size=2*sizeof(int)+buff_len;
	int pos=0;
	
	uint8_t *bitstring= malloc(size);
	bzero(bitstring,size);
		
	memcpy(bitstring,&msg_type,sizeof(int));
	pos+=sizeof(int);
	memcpy(bitstring+pos, &buff_len , sizeof(int));
	pos+=sizeof(int);
	memcpy(bitstring+pos, directory_buff , buff_len );
	
	send(fd,&size,sizeof(size_t),0);
	send(fd,bitstring,size,0);

	free(bitstring);

}

int process_ls(int fd)
{


	typedef struct element_and_size{
		char* item;
		int size;
	} es;

	es *elements=(es*)malloc(BUFF_SIZE*sizeof(es));
	int msg_type=LS;
	int elem_count=0;
	int i=0;
	int pos=0;
	size_t bitstring_size=2*sizeof(int); // msg type + elem_count
 	FILE *fp;
  	char temp[128];
  	bzero(temp,128);
  
  	fp = popen("ls", "r");
  	if (fp == NULL) {
  	  ///ERROR HANDLING
  	}

  	while (fgets(temp, sizeof(temp)-1, fp) != NULL) {
		elements[elem_count].item = (char*)malloc( strlen(temp) );
		elements[elem_count].size = strlen( temp) -1 ;
		memcpy(elements[elem_count].item,temp,elements[elem_count].size);
		bitstring_size += elements[elem_count].size + sizeof(int);
		elem_count++;
	}

	pclose(fp);

	uint8_t *bitstring= malloc(bitstring_size);
	bzero(bitstring,bitstring_size);
	memcpy(bitstring,&msg_type,sizeof(int));
	pos+=sizeof(int);
	memcpy(bitstring+pos,&elem_count,sizeof(int));
	pos+=sizeof(int);
	
	for(i=0; i < elem_count ; ++i){

		memcpy(bitstring+pos,&elements[i].size,sizeof(int));
		pos+=sizeof(int);
		memcpy(bitstring+pos,elements[i].item,elements[i].size);
		pos+=elements[i].size;
		free(elements[i].item);
	} 
	
	send(fd,&bitstring_size,sizeof(size_t),0);	
	send(fd,bitstring,bitstring_size,0);
	free(elements);
	free(bitstring);

} 
