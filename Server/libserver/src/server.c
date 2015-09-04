#include "server.h"


void error(char *msg)
{
    perror(msg);
    exit(0);
}

uint8_t set_msg_type( uint8_t * msg ){

	uint8_t type=0;
	memcpy(&type,msg,M_TYPE);
	return type;

}

char* error_switch(int err_type){

    switch(err_type){

        case PREMISSION_ERROR : return "PREMISSION DENIED";
        case EXIST_ERROR :      return "DIRECTORY DOES NOT EXIST";
        case FILE_ERROR :       return "FILE DOES NOT EXIST";
        case OUT_OF_MEMORY_ERROR : return "OUT OF MEMORY";
        default : return "UNKNOWN ERROR TYPE";
    }
    return "UNKNOWN ERROR TYPE";
}

void send_error_msg(int fd, int err_type){

    uint8_t *bitstring;
    int pos = 0;
    int err_size=err_type;
    uint8_t msg_type=(uint8_t)ERROR;
    int size=M_TYPE + M_SIZE +err_size;

    bitstring=malloc(size);
    cpmv(bitstring, msg_type ,pos,M_TYPE);
    cpmv(bitstring, err_size ,pos,M_SIZE);
    cpmv(bitstring, *error_switch(err_type) ,pos,err_size);

    send_message(fd,size,bitstring);

    free(bitstring);
}
int send_message(int fd,size_t size, uint8_t *bitstring){


        size_t position=0;
        size_t to_send=0;

        if( write(fd,&size,sizeof(size_t)) <0 ){
            error("SEND ERROR");
            return 0;
        }

        if(size<=MTU_SIZE){

              if( write(fd,bitstring,size) < 0){
                    error("SEND ERROR");
                    return 0;
              }

        } else {
              while(position!=size){
                    to_send=(size-position)>MTU_SIZE ? MTU_SIZE : size-position;
                    if( write(fd,bitstring+position,to_send) < 0) {
                         error("SEND ERROR");
                         return 0;
                    }
                    position+=to_send;
              }

        }
    return 1;
}

void execute_command(char* directory ,uint8_t * command,int fd,uint8_t msg_type){

	switch(msg_type){

	          case CD: process_cd(directory,command,fd); break;
            case PWD: process_pwd(directory,fd); break;
            case CAT: process_cat(directory,command,fd); break;
            case LS: process_ls(fd); break;

	}

}


void process_cat(char* directory, uint8_t *buffer, int fd)
{

	uint8_t msg_type=(uint8_t)CAT;
	int pos=M_TYPE;
	uint16_t string_size=0;
  long int size= M_TYPE + M_SIZE;
	FILE *fp;
	uint8_t *bitstring;
  char temp[SMALL_BUFF];
  bzero(temp,SMALL_BUFF);
	memcpy(&string_size,buffer+pos,C_SIZE);
  pos+=C_SIZE;

	char buf[string_size];
	bzero(buf,string_size+1);
	memcpy(&buf,buffer+pos,string_size);

	int status=0;
  struct stat st_buf;
	fp = fopen(buf, "r");

  char *check_path=realpath(buf,NULL);

  status = stat (buf, &st_buf);

    if (status != 0) {
        printf ("Error, errno = %d\n", errno);
    }

  	if (fp == NULL || S_ISDIR (st_buf.st_mode) ) { // czy plik jst katalogiem
  	    send_error_msg(fd, FILE_ERROR);
    }  else if( strncmp( directory, check_path , strlen(directory)) ){
         send_error_msg(fd, PREMISSION_ERROR);
    } else {
        fseek(fp, 0L, SEEK_END);
        string_size=ftell(fp);
        size+=string_size;
        fseek(fp,0L,SEEK_SET);
        pos=0;
        bitstring=malloc(size);
        cpmv(bitstring, msg_type ,pos,M_TYPE);
        cpmv(bitstring, string_size ,pos,M_SIZE);

         while (fgets(temp, sizeof(temp) , fp)!=NULL){
                 cpmv(bitstring, temp ,pos,strlen(temp));
                 bzero(temp,128);
         }
         send_message(fd,size,bitstring);
         free(bitstring);
         fclose(fp);
    }
    free(check_path);
}

void process_pwd(char* directory,int fd)
{
    uint8_t msg_type=(uint8_t)PWD;
    char directory_buff[BUFF_SIZE];
	  bzero(directory_buff,BUFF_SIZE);
    getcwd(directory_buff,BUFF_SIZE-1);
    size_t newdir_size=strlen(directory_buff)-strlen(directory);
    char* newdir;
    size_t buff_len=0;

    if(newdir_size){
        newdir=(char*)malloc(newdir_size);
        memcpy(newdir,directory_buff+strlen(directory),newdir_size);
        buff_len=newdir_size;
    } else {
        newdir=malloc(sizeof(char));
        newdir[0]='/';
        buff_len=sizeof(char);
    }

  	size_t size=M_TYPE + M_SIZE + buff_len;
	  int pos=0;

  	uint8_t *bitstring= malloc(size);//
	  bzero(bitstring,size);//
   // cpmv(bitstring, size ,pos,sizeof(size_t));//
    cpmv(bitstring, msg_type ,pos,M_TYPE);
    cpmv(bitstring, buff_len ,pos,M_SIZE);
    cpmv(bitstring, *newdir ,pos,buff_len);
    free(newdir);

    send_message(fd,size,bitstring);

	  free(bitstring);

}

void process_ls(int fd)
{
	es *elements=(es*)malloc(BUFF_SIZE*sizeof(es));
	uint8_t msg_type=(uint8_t)LS;
	int elem_count=0;
	int i=0;
	int pos=0;
	size_t bitstring_size=M_TYPE + M_SIZE; // msg type + elem_count
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
		    bitstring_size += elements[elem_count].size + M_SIZE;
		    elem_count++;
	      }

	      pclose(fp);

	      uint8_t *bitstring= malloc(bitstring_size);
	      bzero(bitstring,bitstring_size);
        cpmv(bitstring, msg_type ,pos,M_TYPE);
        cpmv(bitstring, elem_count ,pos,M_SIZE);

	      for(i=0; i < elem_count ; ++i){
            cpmv(bitstring, elements[i].size ,pos,M_SIZE);
            cpmv(bitstring, *elements[i].item ,pos,elements[i].size);
		        free(elements[i].item);
	      }

        send_message(fd,bitstring_size,bitstring);
        free(bitstring);
    }
  free(elements);

}

void process_cd(char* directory,uint8_t *buffer, int fd){

    int pos=M_TYPE;
    int string_size=0;
    int flag=0;
    memcpy(&string_size,buffer+pos,C_SIZE);
    pos+=C_SIZE;

    char directory_buff[BUFF_SIZE];
	  bzero(directory_buff,BUFF_SIZE);
    getcwd(directory_buff,BUFF_SIZE-1);
    char parameter[string_size];
    memset(parameter,0,string_size);
    memcpy(parameter,buffer+pos,string_size-1);


    if( !strcmp(parameter,"..")  && !strcmp(directory_buff,directory) ) //cd .. w katalogu root
            flag=PREMISSION_ERROR;
    else if ( !strcmp(parameter,"/") || !strcmp(parameter,"~")){
            flag=CH_DIR;
            chdir(directory);
            bzero(directory_buff,BUFF_SIZE);
            getcwd(directory_buff,BUFF_SIZE-1);

    }
    else {
           if( chdir(parameter) ){ //fail chdir

                flag=EXIST_ERROR;

           } else { // no failed
                bzero(directory_buff,BUFF_SIZE);
                getcwd(directory_buff,BUFF_SIZE-1);
                if( !strncmp( directory, directory_buff , strlen(directory)) )  // chdir zawiera sciezke roota
                        flag=CH_DIR;
                else{
                        flag=PREMISSION_ERROR;     //chdir nie zawiera sciazki roota
                        chdir(directory);
                }
           }

    }

    if(flag==CH_DIR){
          process_pwd(directory,fd);
    } else if(flag==EXIST_ERROR) {
          send_error_msg(fd, EXIST_ERROR);
    } else if(flag==PREMISSION_ERROR) {
          send_error_msg(fd, PREMISSION_ERROR);
    }



}
