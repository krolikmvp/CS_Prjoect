#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
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
   LS,
   ERROR
};

enum cd_flags{

    CH_DIR=10,
    PREMISSION_ERROR = 17,     // "PREMISSION DENIED"
    EXIST_ERROR = 24,   // "DIRECTORY DOES NOT EXIST"
    FILE_ERROR = 19,    // "FILE DOES NOT EXIST"
    OUT_OF_MEMORY_ERROR = 13  //"OUT OF MEMORY"
};


typedef struct element_and_size{
	char* item;
	int size;
} es;
 
void error(char *msg)
{
    perror(msg);
    exit(0);
}

int set_msg_type( uint8_t * msg ){

	int type=0;
	memcpy(&type,msg,sizeof(int));
	return type;

}

int process_cd(char*,uint8_t *, int);
int process_pwd(int);
int process_cat(char*,uint8_t *, int);
int process_ls(int);
int send_error_msg(int,int);
char* error_switch(int);

char* error_switch(int err_type){

    switch(err_type){

        case PREMISSION_ERROR : return "PREMISSION DENIED";
        case EXIST_ERROR :      return "DIRECTORY DOES NOT EXIST";
        case FILE_ERROR :       return "FILE DOES NOT EXIST";
        case OUT_OF_MEMORY_ERROR : return "OUT OF MEMORY";
    }

}

int send_error_msg(int fd, int err_type){

    uint8_t *bitstring;
    int pos = 0;
    int err_size=err_type;
    int msg_type=ERROR;
    int size=2*sizeof(int)+err_size;

    bitstring=malloc(size);

    memcpy(bitstring,&msg_type,sizeof(int));
    pos+=sizeof(int);
    memcpy(bitstring+pos,&err_size,sizeof(int));
    pos+=sizeof(int);
    memcpy(bitstring+pos,error_switch(err_type),err_size);
    pos+=err_size;

    send(fd,&size,sizeof(size_t),0);
    send(fd,bitstring,size,0);

    free(bitstring);
}

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
    uint8_t temp[128];
    bzero(temp,128);   
	memcpy(&string_size,buffer+pos,sizeof(int));
	pos+=sizeof(int);

	char buf[string_size];
	bzero(buf,string_size+1);
	memcpy(&buf,buffer+pos,string_size);
    
	int status=0;
    struct stat st_buf;
	fp = fopen(buf, "r");
    status = stat (buf, &st_buf);
    if (status != 0) {
        printf ("Error, errno = %d\n", errno);
    }

  	if (fp == NULL || S_ISDIR (st_buf.st_mode) ) {
  	    send_error_msg(fd, FILE_ERROR);
    }  else {
        fseek(fp, 0L, SEEK_END);
        size+=ftell(fp);
        fseek(fp,0L,SEEK_SET);
        // size=size-1;////////////// 
         pos=0;
         bitstring=malloc(size);
         memcpy(bitstring,&msg_type,sizeof(int));
         pos+=sizeof(int);
         memcpy(bitstring+pos,&size,sizeof(int));
         pos+=sizeof(int);
         while (fgets(temp, sizeof(temp) , fp)!=NULL){

	             memcpy(bitstring+pos,temp,strlen(temp));
                 pos+=strlen(temp); 
                 bzero(temp,128);}
           
    
        	

    	if(send(fd,&size,sizeof(size_t),0) < 0)
          error("CANNOT SEND DATA");
    	if(send(fd,bitstring,size,0) < 0)
          error("CANNOT SEND DATA");
         free(bitstring);
         fclose(fp);
    }

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
	
	if(send(fd,&size,sizeof(size_t),0) < 0)
        error("CANNOT SEND DATA");
	if(send(fd,bitstring,size,0) < 0)
        error("CANNOT SEND DATA");

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
  	  send_error_msg(fd,OUT_OF_MEMORY_ERROR);
  	}
    else { 
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

	if(send(fd,&bitstring_size,sizeof(size_t),0) < 0)
        error("CANNOT SEND DATA");
	if(send(fd,bitstring,bitstring_size,0) < 0)
        error("CANNOT SEND DATA");
	    free(bitstring);
    }

    free(elements);

} 

int process_cd(char* directory,uint8_t *buffer, int fd){

    int msg_type=CD;
    int pos=sizeof(int);
    int size=2*sizeof(int);
    int string_size=0;
    int flag=0;
    int err=ERROR;
    int err_size=0;
    memcpy(&string_size,buffer+pos,sizeof(int));
    pos+=sizeof(int);
  
    char directory_buff[BUFF_SIZE];
	bzero(directory_buff,BUFF_SIZE);
    getcwd(directory_buff,BUFF_SIZE-1);
    char parameter[string_size];
    memset(parameter,0,string_size);
    memcpy(parameter,buffer+pos,string_size-1);
   

    if( !strcmp(parameter,"..")  && !strcmp(directory_buff,directory) ) //cd .. w katalogu root
           flag=PREMISSION_ERROR;
    else if ( !strcmp(parameter,"/") || !strcmp(parameter,"~")){

            chdir(directory);
            bzero(directory_buff,BUFF_SIZE);
            getcwd(directory_buff,BUFF_SIZE-1);
            string_size=strlen(directory_buff);
    }
    else {
           if( chdir(parameter) ){ //fail chdir

                flag=EXIST_ERROR;          

           } else { // no failed
                bzero(directory_buff,BUFF_SIZE);
                getcwd(directory_buff,BUFF_SIZE-1);
                string_size=strlen(directory_buff);         
                if( !strncmp( directory, directory_buff , strlen(directory)) )  // chdir zawiera sciezke roota
                        flag=CH_DIR;             
                else{
                        flag=PREMISSION_ERROR;     //chdir nie zawiera sciazki roota
                        chdir("-");
                }
           }
           
    }

    if(flag==CH_DIR)
    size+=string_size;
    else
    size+=flag;
    

    if(flag==CH_DIR){
               uint8_t *bitstring;
               bitstring=malloc(size);
               bzero(bitstring,size);
               pos=0;
               memcpy(bitstring,&msg_type,sizeof(int));
               pos+=sizeof(int);
               memcpy(bitstring+pos,&string_size,sizeof(int));
               pos+=sizeof(int);
               memcpy(bitstring+pos,directory_buff,string_size);
               pos+=string_size;             
               if(send(fd,&size,sizeof(size_t),0) < 0)
                      error("CANNOT SEND DATA");
	           if(send(fd,bitstring,size,0) < 0)
                      error("CANNOT SEND DATA");

               free(bitstring);  

    } else if(flag==EXIST_ERROR) {
          send_error_msg(fd, EXIST_ERROR);
    } else if(flag==PREMISSION_ERROR) {
          send_error_msg(fd, PREMISSION_ERROR);
    }

 

}
