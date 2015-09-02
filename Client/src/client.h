#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h> 
#include <stdlib.h>
#include <getopt.h>
#include <inttypes.h>
#include<errno.h>
#define _cd "cd"
#define _ls "ls"
#define _pwd "pwd"
#define _cat "cat"
#define _exit "exit"
#define TEMP_BUFF_SIZE 128
#define MTU_SIZE 1500

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
int validate_send(char *);
void print_message(msg * );

int write_to_srv(int fd, char * buff, int msg_type)
{

    uint8_t *buffer;
    int pos=0;
    size_t size=sizeof(int);
    int tmp=0;

    switch(msg_type){

    case 1://cd
	       tmp=strlen(buff)-strlen(_cd)-1;
           size+=sizeof(int) + tmp;
           buffer=malloc(size);
           memcpy(buffer,&msg_type,sizeof(int));
           pos+=sizeof(int);
           memcpy(buffer+pos,&tmp,sizeof(int));
           pos+=sizeof(int);
           memcpy(buffer+pos,buff+(strlen(_cd)+1),tmp );
           break;
    case 2://pwd
           buffer=malloc(size);
           memcpy(buffer,&msg_type,size);
           break;
    case 3://cat
	       tmp= strlen(buff)-strlen(_cat) - 2;
	       size+=sizeof(int)+tmp;
           buffer=(char*)malloc(size);
           memcpy(buffer,&msg_type,sizeof(int));
           pos+=sizeof(int);
	       memcpy(buffer+pos,&tmp,sizeof(int));
	       pos+=sizeof(int);
           memcpy(buffer+pos,buff+(strlen(_cat)+1), tmp );
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

int read_from_server(int fd)
{
    int size=0;
    int pos=0;
    int bytes_read=0;
    int msg_type=0;
    int i=0;
    int string_size=0;
    msg * message=malloc(sizeof(msg));
    uint8_t temp[MTU_SIZE];
    read(fd,&size,sizeof(size_t));
    message->size=size;
    uint8_t *buffer=malloc(size);
    bzero(buffer,size);

    int tmp_size=0;
    while(bytes_read!=size){
      bzero(temp,MTU_SIZE);
      tmp_size=read(fd,temp,MTU_SIZE-1);
      memcpy(buffer+pos,&temp,tmp_size);
      bytes_read+=tmp_size;
      pos+=tmp_size;
    }

   pos=0;

   memcpy(&message->head,buffer,sizeof(int));
   pos+=sizeof(int);

   if(message->head==LS){
	 memcpy(&message->elements,buffer+pos,sizeof(int));
 	 pos+=sizeof(int);
     message->content=(char**)malloc(sizeof(char*)*message->elements);
        
        for(i;i<message->elements;++i){
	         string_size=0;
	         memcpy(&string_size,buffer+pos,sizeof(int));
	         pos+=sizeof(int);
             message->content[i]=malloc(string_size+1);
	         bzero(message->content[i],string_size);
	         memcpy(message->content[i],buffer+pos,string_size);
             message->content[i][string_size]=0;
	         pos+=string_size;	

        }
    
        print_message(message);


        for(i=0;i<message->elements;++i){
	         free(message->content[i]);
	    }
        
        free(message->content);


   } else {
        
	    string_size=0;
        memcpy(&string_size,buffer+pos,sizeof(int));
        pos+=sizeof(int);
        message->string=malloc(string_size+1);
        bzero(message->string,string_size+1);
        memcpy(message->string,buffer+pos,string_size);
        message->string[string_size]=0;
        print_message(message);
        free(message->string);
       
   }  
 

free(buffer);
free(message);

}

void print_message( msg * message){

       printf("\n");
       printf("MESSAGE TYPE : %d\n",message->head);
       printf("MESSAGE SIZE : %d\n",message->size);

       if( message->head == LS )
       {
            printf("MESSAGE ELEMENTS : %d\n",message->elements);
            printf("MESSAGE CONTENT : \n");
            int i=0;
            for (i ; i< message->elements ; ++i)
                printf("%d : %s\n",i+1,message->content[i]);
       } else {

            printf("MESSAGE CONTENT : \n");
            printf("%s\n",message->string);


       }

       printf("\n");
}

int validate_send(char * buff){
////sprawdza poprawnosc wpisywanych komend
	if(!strncmp(buff,_cd,strlen(_cd))){
		if( (strlen(buff)-1)==strlen(_cd) ){
		     printf("ERROR: Parameters needed \"cd <directory_name>\"\n");
		     return 0;}
		else if( (strlen(buff)-1) > (strlen(_cd)+1) && buff[2]==' ' )
	             return 1;

	}
	if(!strncmp(buff,_cat,strlen(_cat))){
		if( (strlen(buff)-1)==strlen(_cat) ){
		     printf("ERROR: Parameters needed \"cat <file_name>\"\n");
		     return 0;}
		else if( (strlen(buff)-1) > (strlen(_cat)+1) && buff[3]==' ' )
	             return 3;

	}
	if(!strncmp(buff,_ls,strlen(_ls))){
		if( (strlen(buff)-1)==strlen(_ls) )
	             return 4;

	}
	if(!strncmp(buff,_pwd,strlen(_pwd))){
		if( (strlen(buff)-1)==strlen(_pwd) )
	             return 2;
	}
	if(!strncmp(buff,_exit,strlen(_exit))){
		if( (strlen(buff)-1)==strlen(_exit) )
	             return -1;
	}

printf("ERROR: Wrong command\n");
return 0;
}
