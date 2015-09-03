#include "server.h"


int main(int argc, char *argv[])
{
   
    int next_option;
    const char* const short_options="d:p:h";
    size_t size=0;
    char directory[BUFF_SIZE];
    int prt=0;
    char *program_name=argv[0];   
    do{
	next_option = getopt_long(argc,argv,short_options,long_options,NULL);
	
	    switch(next_option)
	    {
	        case 'd':strcpy(directory,optarg);break;
	        case 'p':prt = atoi(optarg); break;
	        case 'h':print_usage(stdout,0,program_name);break;
	        case '?':exit(0);
	    }
      }while(next_option!=-1);

    if (argc < 3) {
        printf("Invalid usage. Use -h or --help for usage informations\n");
       exit(0);
    }	
	if( chdir(directory) ){
        error("WRONG STARTING DIRECTORY");
        exit(0);        
    }
    int exit_flag=0;
	int user_number=0;
	int i = 0;
	int srv_fd = -1;
	int cli_fd = -1;
	int epoll_fd = -1;
	ssize_t readb=0;
	struct sockaddr_in srv_addr;
	struct sockaddr_in cli_addr;
	socklen_t cli_addr_len=0;
	struct epoll_event e, es[user_number];

	memset(&srv_addr, 0, sizeof(srv_addr));
	memset(&cli_addr, 0, sizeof(cli_addr));
	memset(&e, 0, sizeof(e));

	srv_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (srv_fd < 0) {
		printf("Cannot create socket\n");
		return 1;
	}

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	srv_addr.sin_port = htons(prt);
	if (bind(srv_fd, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0) {
		printf("Cannot bind socket\n");
		close(srv_fd);
		return 1;
	}

	if (listen(srv_fd, 1) < 0) {
		printf("Cannot listen\n");
		close(srv_fd);
		return 1;
	}

	epoll_fd = epoll_create(1);
	if (epoll_fd < 0) {
		printf("Cannot create epoll\n");
		close(srv_fd);
		return 1;
	}

	e.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	e.data.fd = srv_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, srv_fd, &e) < 0) {
		printf("Cannot add server socket to epoll\n");
		close(epoll_fd);
		close(srv_fd);
		return 1;
	}
	////////////MAIN SERVER LOOP
	for(;;) {
		i = epoll_wait(epoll_fd, es, 1, -1);

		if (i < 0) {
			printf("Cannot wait for events\n");
			close(epoll_fd);
			close(srv_fd);
			return 1;
		}

		for (--i; i > -1; --i) {
			if (es[i].data.fd == srv_fd) {
				cli_fd = accept(srv_fd, (struct sockaddr*) &cli_addr, &cli_addr_len);
				if (cli_fd < 0) {
					printf("Cannot accept client1\n");
					close(epoll_fd);
					close(srv_fd);
					return 1;
				}

				e.data.fd = cli_fd;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cli_fd, &e) < 0) {
					printf("Cannot accept client2\n");
					close(epoll_fd);
					close(srv_fd);
					return 1;
				}
			} else {
				cli_fd=es[i].data.fd;
				if (es[i].events & EPOLLERR || es[i].events & EPOLLHUP || es[i].events & EPOLLRDHUP) {

                    printf("Client with file descriptor [%d] disconnected\n",cli_fd);
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cli_fd, &e);
					close(cli_fd);
				    exit_flag=1;

				}
				else if ( es[i].events & EPOLLIN ){
					// Odczytywanie msg od klienta
					uint16_t msg_len=0;
                    uint8_t msg_type=0;
					read(cli_fd, &msg_len, C_SIZE);
                    uint8_t *buffer=malloc(msg_len);
					readb=read(cli_fd, buffer,msg_len );
                    msg_type=set_msg_type(buffer);

					printf("Expected : %zu Read: %zu, message type :%d\n",msg_len,readb,msg_type);

					if (readb > 0) {
						execute_command(directory,buffer,cli_fd,msg_type);	
						printf("MSG HAS BEEN SENT\n");
					} else {

                        printf("CANNOT READ DATA FROM CLIENT\n");
                    }
				    bzero(buffer,msg_len);
                    free(buffer);
		
				}
				else {
                                        printf("Client %d disconnected\n",cli_fd);
                                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cli_fd, &e);
				                    	close(cli_fd);
                                        exit_flag=1;
			    }
			}
            
		}
        if(exit_flag)break;
	}
    printf("Closing Server\n");
	return 0;
}

void print_usage (FILE* stream, int exit_code,char * program_name) 
{

 fprintf (stream, "Usage: %s [OPTIONS]\n", program_name); 

 fprintf (stream, 

          "  -h  --help            Display this usage information.\n" 

          "  -d  --directory <directory>          Starting(root) directory of your remote file program.\n" 

          "  -p  --port <server_port>        Port of the server you want connect.\n"); 

 exit (exit_code); 

}

