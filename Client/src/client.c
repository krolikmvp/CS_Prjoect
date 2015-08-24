#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h> 
#include <stdlib.h>
#include <getopt.h>
#define _cd "cd"
#define _ls "ls"
#define _pwd "pwd"
#define _cat "cat"
#define _exit "exit"
#define BUFF_SIZE 5242880


const char * program_name;
const struct option long_options[]={

	{"ip", 1 , NULL , 'i' },
	{"port", 1 , NULL , 'p' },
	{"help", 0 , NULL , 'h' },
	{ NULL , 0, NULL , 0}

};

void print_usage (FILE* stream, int exit_code) 
{

 fprintf (stream, "Usage: %s options [ inputfile .... ]\n", program_name); 

 fprintf (stream, 

          "  -h  --help            Display this usage information.\n" 

          "  -i  --ip <ip_adress>          IP of the server you want connect.\n" 

          "  -p  --port <server_port>        Port of the server you want connect.\n"); 

 exit (exit_code); 

}
 
void error(char *msg)
{
    perror(msg);
    exit(0);
}

int validate_send(char *);

int main(int argc, char *argv[])
{
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int next_option;
    const char* const short_options="i:p:h";
    program_name=argv[0];   
    do{
	next_option = getopt_long(argc,argv,short_options,long_options,NULL);
	
	switch(next_option)
	{
	  case 'i':server = gethostbyname(optarg);break;
	  case 'p':portno = atoi(optarg); break;
	  case 'h':print_usage(stdout,0);break;
	  case '?':exit(0);
	  //case -1: break;
	}
      }while(next_option!=-1);
    char *buffer=(char*)malloc(sizeof(char)*BUFF_SIZE);
    
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
           error("ERROR connecting");

  for(;;){
	int valid=0;

	do{
		printf("Please enter the message: ");
    		bzero(buffer,strlen(buffer));
    		fgets(buffer,BUFF_SIZE-1,stdin);
		valid=validate_send(buffer);		
	}while(valid==0);

    if(valid<0) break;   
    
    n = write(sockfd,buffer,strlen(buffer)-1);
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,strlen(buffer));
    n = read(sockfd,buffer,BUFF_SIZE-1);
    printf("%zu \n",strlen(buffer));
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
	}
    free(buffer);
    close(sockfd);
    printf("Closing client\n");
    return 0;

}

int validate_send(char * buff){

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
	             return 1;

	}
	if(!strncmp(buff,_ls,strlen(_ls))){
		if( (strlen(buff)-1)==strlen(_ls) )
	             return 1;

	}
	if(!strncmp(buff,_pwd,strlen(_pwd))){
		if( (strlen(buff)-1)==strlen(_pwd) )
	             return 1;
	}
	if(!strncmp(buff,_exit,strlen(_exit))){
		if( (strlen(buff)-1)==strlen(_exit) )
	             return -1;
	}

printf("ERROR: Wrong command\n");
return 0;
}


