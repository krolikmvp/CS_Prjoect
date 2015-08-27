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

typedef struct element_and_size{
	char* item;
	int size;
} es;

int set_msg_type( uint8_t * msg ){

	int type=0;
	memcpy(&type,msg,sizeof(int));
	return type;

}

int process_cd(char*,uint8_t *, int);
int process_pwd(int);
int process_cat(char*,uint8_t *, int);
int process_ls(int);


int execute_command(char* directory ,uint8_t * command,int fd,int msg_type){

	switch(msg_type){

	        case CD: process_cd(directory,command,fd); break;
            case PWD: process_pwd(fd); break;
            case CAT: process_cat(directory,command,fd); break;
            case LS: process_ls(fd); break;

	}

}


int process_cat(char* directory, uint8_t *buffer, int fd)
{

	int msg_type=CAT;
	int pos=sizeof(int);
	int string_size=0;
    long int size=2*(sizeof(int));
	FILE *fp;
	uint8_t *bitstring;
    es * elements=malloc(sizeof(es));
    uint8_t temp[128];
    bzero(temp,128);   
	memcpy(&string_size,buffer+pos,sizeof(int));
	pos+=sizeof(int);

	char buf[string_size];
	bzero(buf,string_size+1);
	memcpy(&buf,buffer+pos,string_size);
    
	
	fp = fopen(buf, "r");
  	if (fp == NULL) {
  	  printf("ERROR\n");
  	} else {
        fseek(fp, 0L, SEEK_END);
	    printf("SIZE of file :%lu\n",ftell(fp));
        size+=ftell(fp);
        fseek(fp,0L,SEEK_SET);
    }
    pos=0;
    bitstring=malloc(size);
    memcpy(bitstring,&msg_type,sizeof(int));
    pos+=sizeof(int);
    memcpy(bitstring+pos,&size,sizeof(int));
    pos+=sizeof(int);

    printf("SIZEOF temp : %zu\n",sizeof(temp)); 
  	while (fgets(temp, sizeof(temp)-1 , fp)!=NULL){
	    memcpy(bitstring+pos,temp,strlen(temp));
        pos+=strlen(temp); 
        bzero(temp,128);

	}

    printf("POS : %d\n",pos);
    printf("SIZE : %lu\n",size);

    send(fd,&size,sizeof(size_t),0);
	send(fd,bitstring,size,0);

 free(elements);
 free(bitstring);

 fclose(fp);

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

int process_cd(char* directory,uint8_t *buffer, int fd){
  int pos=sizeof(int);
  int string_size=0;
  memcpy(&string_size,buffer+pos,sizeof(int));
  pos+=sizeof(int);
  
    char directory_buff[BUFF_SIZE];
	bzero(directory_buff,BUFF_SIZE);
    getcwd(directory_buff,BUFF_SIZE-1);

    if(!strcmp(buffer+pos,"..")){
        if(strcmp(directory_buff,directory)){
            chdir(buffer+pos);
           	bzero(directory_buff,BUFF_SIZE);
            getcwd(directory_buff,BUFF_SIZE-1);
            printf("%s \n",directory_buff);
        }else{
         printf("NJE MASZ DOSTEPU SUKO\n");
        }
    }
	

}
