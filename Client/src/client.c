#include "client.h"



int main(int argc, char *argv[])
{



    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int next_option;
    const char* const short_options="i:p:h";
    size_t size=0;
    char *program_name=argv[0];   
    do{
	next_option = getopt_long(argc,argv,short_options,long_options,NULL);
	
	    switch(next_option)
	    {
	        case 'i':server = gethostbyname(optarg);break;
	        case 'p':portno = atoi(optarg); break;
	        case 'h':print_usage(stdout,0,program_name);break;
	        case '?':exit(0);
	    }

    }while(next_option!=-1);

    if (argc < 3) { 
        printf("Invalid usage. Use -h or --help for usage informations\n");
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
		//strcpy(buff,"cat x\n");
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



void print_usage (FILE* stream, int exit_code,char * program_name) 
{

 fprintf (stream, "Usage: %s <ip_adress> <server_port>\n", program_name); 

 fprintf (stream, 

          "  -h  --help            Display this usage information.\n" 

          "  -i  --ip <ip_adress>          IP of the server you want connect.\n" 

          "  -p  --port <server_port>        Port of the server you want connect.\n"); 

 exit (exit_code); 

}
