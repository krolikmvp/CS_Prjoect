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
   ITEM_LIST=1,
   DIRECTORY,
   FILE_CONTENT,
   ERROR
};

int set_msg_type( uint8_t * msg ){

	int type=0;
	memcpy(&type,msg,sizeof(int));
	return type;

}

int execute_command(char * command,int fd){

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

  }


  FILE *fp;
  char *buffer=(char*)malloc(sizeof(char)*BUFF_SIZE_OUT);
  char temp[128];
  bzero(temp,128);
  bzero(buffer,BUFF_SIZE_OUT);
  /* Open the command for reading. */
  fp = popen(command, "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  /* Read the output a line at a time - output it. */
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

}


