#include "client.h"



int main(int argc, char *argv[])
{
  
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int next_option;
    const char* const short_options="i:p:h";
    size_t size=0;
    //program_name=argv[0];   
    do{
	next_option = getopt_long(argc,argv,short_options,long_options,NULL);
	
	    switch(next_option)
	    {
	        case 'i':server = gethostbyname(optarg);break;
	        case 'p':portno = atoi(optarg); break;
	        case 'h'://print_usage(stdout,0);break;
	        case '?':exit(0);
	    }
      }while(next_option!=-1);

    if (argc < 3) {
       exit(0);
    }
   
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
           error("ERROR connecting");
     
    char buff[TEMP_BUFF_SIZE];
    bzero(buff,TEMP_BUFF_SIZE);

  for(;;){

    int msg_type=0;

    do{
		printf("Please enter the message: ");
        bzero(buff,TEMP_BUFF_SIZE);
    	fgets(buff,TEMP_BUFF_SIZE-1,stdin);
		//strcpy(buff,"cd ..\n");
		msg_type=validate_send(buff);		
    }while(msg_type==0);

	

    if(msg_type<0) break;
   
    write_to_srv(sockfd,buff,msg_type);
    read_from_server(sockfd);

  } 
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


