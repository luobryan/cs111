//NAME: Bryan Luo
//EMAIL: luobryan@ucla.edu
//ID: 605303956

#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h> 
#include <stdbool.h>
#include "zlib.h"

#define BUFFERSIZE 2048
char buffer[BUFFERSIZE]; 

void sigpipe_handler(){
	fprintf(stderr,"%s","error: sigpipe caught"); 
	exit(0);
}
int server_connect(unsigned int port_num){ //port_num for port to listen to....returns socket for communciation 
	int sockfd, new_fd;  //listen on sock_fd, new connection on new_fd
	struct sockaddr_in my_addr; //my address
	struct sockaddr_in their_addr; //connector address
	unsigned int sin_size; 

	//create socket
	sockfd = socket(AF_INET, SOCK_STREAM,0);

	//set address info
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port_num); //converts from host byte order to network byte order 
	my_addr.sin_addr.s_addr = INADDR_ANY; //client can connect aony one of the host's IP Address

	//pad zeros
	memset(my_addr.sin_zero,'\0',sizeof(my_addr.sin_zero)); 

	//bind socket to the IP Address and port number
	bind(sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr));  
	listen(sockfd, 5); //maximum 5 pending connections 
	//wait for client's connection, their_addr stores client's address
	sin_size = sizeof(struct sockaddr_in);
	new_fd = accept(sockfd,(struct sockaddr*)&their_addr, &sin_size); 
	return new_fd;
}
int main(int argc, char* argv[]){
	//process commandline arguments
	struct option args[] = {
		{"port",1,NULL,'p'},
		{"compress",0,NULL,'c'},
		{0,0,0,0}
	}; 
	int port = -1; 
	bool compress = false;
	int x; 
	while ( (x=getopt_long(argc,argv,"",args,NULL)) != -1){
		switch(x)
		{
			case 'p':
				port = strtol(optarg, NULL, 10); 
				break;
			case 'c':
				compress = true; 
				break;
			default:
				fprintf(stderr,"usage: accepted options are [--port= --log= --compress]\n");
				exit(1);
				break;
		}
	}	

	//from piazza post, we can assume the port # will be valid
    if (port == -1){ //means no port # was inputted
        fprintf(stderr, "error: you must input a port # using the --port option"); 
        exit(1); 
    }

	
	//register sigpipe handler
	signal(SIGPIPE,sigpipe_handler);
	//create pipes
	int i; 
	int to_shell[2];
	int from_shell[2]; 
	if(pipe(to_shell)==-1){
		fprintf(stderr,"error in creating pipe to child shell.");
		exit(1);
	}
	if(pipe(from_shell)==-1){
		fprintf(stderr,"error in creating pipe from child shell.");
		exit(1); 
	}; 
	//fork new process
	int ret = fork();
	if (ret < 0){
		fprintf(stderr,"error in fork");
		exit(1); 
	}



	///


	if (ret == 0){ //child process
		//redirection
		if(close(to_shell[1])<0){
			fprintf(stderr,"error in closing pipe to shell"); 
			exit(1);
		} //child doesn't write to this pipe
		if(close(from_shell[0])<0){
			fprintf(stderr,"error in closing pipe from shell");
			exit(1);
		} //child doesn't read from this pipe 
		//redirect from stdin
		if(close(0)<0){
			fprintf(stderr,"error in closing stdin");
			exit(1);
		} //child doesn't take input from stdin
		if(dup(to_shell[0])<0){
			fprintf(stderr,"error doing dup for to shell pipe");
			exit(1);
		} // 0 (lowest unused file descriptor) maps to read end of to_shell pipe

		//redirect from stdout 
		if(close(1)<0){
			fprintf(stderr,"error in closing stdout");
			exit(1);
		} //child doesn't output to stdout
		if(dup(from_shell[1])<0){
			fprintf(stderr,"error in doing dup for from shell pipe"); 
			exit(1); 
		} // 1 file descriptor maps to write end of from_shell pipe
		if(close(2)<0){
			fprintf(stderr,"error in closing stderr");
			exit(1); 
		} //child doesn't output error to stderr
		if(dup(from_shell[1])<0){
			fprintf(stderr,"error in doing dup for from shell pipe"); 
			exit(1); 
		} // 2 file descriptor maps to write end of from_shell pipe
		
		if(close(to_shell[0])<0){
			fprintf(stderr,"error in closing to shell pipe"); 
			exit(1);
		} //no longer needed
		if(close(from_shell[1])<0){
			fprintf(stderr,"error in closing from shell pipe.");
			exit(1); 
		}; //no longer needed
		int q = 0;
		q = execlp("/bin/bash","bash",NULL); 
		if(q==-1){
			fprintf(stderr,"error in starting child shell"); 
			exit(1);
		}
	}

	/////


	else if(ret > 0){  //parent process
		int socket_fd = server_connect(port); 
		if(close(to_shell[0])<0){
			fprintf(stderr,"error in closing to shell pipe"); 
			exit(1);
		} //parent process doesn't read from this pipe
		if(close(from_shell[1])<0){
			fprintf(stderr,"error in closing from shell pipe.");
			exit(1); 
		} //parent process doesn't write to this pipe 

		//We replace stdin with the socket
		if(close(0)<0){
			fprintf(stderr,"error in closing stdin");
			exit(1);
		}
		if(dup(socket_fd)<0){
			fprintf(stderr,"error in doing dup for the socket"); 
			exit(1);
		}

		//We replace stdout with the socket
		if(close(1)<0){
			fprintf(stderr,"error in closing stdout"); 
			exit(1);
		}
		if(dup(socket_fd)<0){
			fprintf(stderr,"error in doing dup for the socket");
			exit(1); 
		}

		//close the unneeded socket_fd 
		if(close(socket_fd)<0){
			fprintf(stderr,"error in closing socket file descriptor");
			exit(1);
		}

		//setup polls
		struct pollfd pollfds[2];
		pollfds[0].fd = 0;
		pollfds[0].events = POLLIN + POLLHUP + POLLERR;
		pollfds[1].fd = from_shell[0];
		pollfds[1].events = POLLIN + POLLHUP + POLLERR; 

		while(1){
			if(poll(pollfds,2,-1)<0){
				fprintf(stderr,"error in doing poll()"); 
				exit(1); 
			} //2 means 2 items in fds array, -1 means no timeout

			//if socket ready to read
			if(pollfds[0].revents&POLLIN){
				int numBytesRead = read(0,buffer,BUFFERSIZE); //read input from socket
				if (numBytesRead<0){
					fprintf(stderr,"error in reading");
					exit(1); 
				}
				unsigned char outbuf[BUFFERSIZE];
				if(compress){
					z_stream strm;
					//initialize
					strm.zalloc = Z_NULL;
					strm.zfree = Z_NULL;
					strm.opaque = Z_NULL;
					int res = inflateInit(&strm); 
					if (res != Z_OK){
						fprintf(stderr,"error in the compression");
						exit(1); 
					}
					//do the inflation
					strm.avail_in = numBytesRead; 
					strm.next_in = (unsigned char*) buffer;
					strm.next_out = outbuf; 
					strm.avail_out = BUFFERSIZE; 
					while(strm.avail_in > 0){
						inflate(&strm, Z_SYNC_FLUSH); 
					}
					//
					memcpy(buffer,outbuf,BUFFERSIZE-strm.avail_out); 
					numBytesRead = BUFFERSIZE-strm.avail_out;
					//
					inflateEnd(&strm); 
            	}
		
				//write values from socket to shell
				//we only write to the shell, not the socket (fd 1), unless it's ^C or ^D 
				for(i = 0; i < numBytesRead; i++){
					int a  = 0; 
					if(buffer[i] == '\r' || buffer[i] == '\n'){
						a = write(to_shell[1],"\n",1); 
					}
					else if(buffer[i] == 0x03){
						if(kill(ret, SIGINT)<0){
							fprintf(stderr,"error in killing child shell");
							exit(1); 
						} 
					}
					else if(buffer[i] == 0x04){
						if(close(to_shell[1])<0){
							fprintf(stderr,"error in closing to shell pipe.");
							exit(1);
						} 
						int status = 0;
						if(waitpid(ret, &status, 0)<0){
							fprintf(stderr,"error in waitpid"); 
							exit(1); 
						} 
						fprintf(stderr,"SHELL EXIT SIGNAL=%d STATUS=%d", WTERMSIG(status), WEXITSTATUS(status)); 
						if(close(from_shell[0])<0){
							fprintf(stderr,"error in closing pipe from shell");
							exit(1);
						}
						exit(0); 
					}
					else{
						a = write(to_shell[1],&buffer[i],1); 
					}
					if (a == SIGPIPE){ //need to consider if server gets EOF from shell as well? in addition to SIGPIPE
						if(close(to_shell[1])<0){
							fprintf(stderr,"error in closing pipe to shell");
							exit(1); 
						}
						int status = 0;
						if(waitpid(ret, &status, 0)<0){
							fprintf(stderr,"error in waitpid for child process"); 
							exit(1); 
						}
						fprintf(stderr,"SHELL EXIT SIGNAL=%d STATUS=%d", WTERMSIG(status), WEXITSTATUS(status));
						if(close(from_shell[0])<0){
							fprintf(stderr,"error in closing pipe from shell"); 
							exit(1); 
						}
						exit(0);                            
					}
					else if (a == -1){
						fprintf(stderr,"error in writing to shell"); 
						exit(1); 
					}
				}
			}

			//if from_shell[0] is ready to read
			if(pollfds[1].revents & POLLIN){
				int numBytesRead = read(from_shell[0], buffer, BUFFERSIZE); //read input from child shell 
				if (numBytesRead<0){
					fprintf(stderr,"error in reading");
					exit(1); 
				}
				//write values from child shell to socket  
				unsigned char outbuf[BUFFERSIZE];
				if(compress){
					z_stream strm;
					//initialize
					strm.zalloc = Z_NULL;
					strm.zfree = Z_NULL;
					strm.opaque = Z_NULL;
					int res = deflateInit(&strm,Z_DEFAULT_COMPRESSION); 
					if (res != Z_OK){
						fprintf(stderr,"error in the compression");
						exit(1); 
					}
					//do the deflation
					strm.avail_in = numBytesRead; 
					strm.next_in = (unsigned char*) buffer;
					strm.next_out = outbuf; 
					strm.avail_out = BUFFERSIZE; 
					while(strm.avail_in > 0){
						deflate(&strm, Z_SYNC_FLUSH); 
					}
					//
					if(write(1,outbuf,BUFFERSIZE-strm.avail_out)<0){
						fprintf(stderr,"error in writing compressed data");
						exit(1);
					}
					//
					deflateEnd(&strm); 
            	}
				else{
						if(write(1,buffer,numBytesRead)<0){
							fprintf(stderr,"error in wriitng to socket"); 
							exit(1); 
						}
				}
			}
			
		
	 		//poll error
			if (pollfds[0].revents & (POLLHUP | POLLERR)){
				if(close(to_shell[1])<0){
					fprintf(stderr,"error in closing pipe to shell"); 
					exit(1); 
				}
				int w;
				if(waitpid(ret,&w,0)<0){
					fprintf(stderr,"error in waitpid"); 
					exit(1); 
				}
				fprintf(stderr,"SHELL  EXIT SIGNAL=%d, STATUS=%d,", WTERMSIG(w),WEXITSTATUS(w)); 
				if(close(from_shell[0])<0){
					fprintf(stderr,"error in closing pipe from shell"); 
					exit(1); 
				} 
				exit(0); 
			}
			//poll error
			if (pollfds[1].revents & (POLLHUP | POLLERR)){
				if(close(to_shell[1])<0){
					fprintf(stderr,"error in closing pipe to shell");
					exit(1); 
				}
				int w;
				if (waitpid(ret,&w,0)<0){
					fprintf(stderr,"error in waitpid");
					exit(1); 
				}
			//	fprintf(stderr,"top");
				fprintf(stderr,"SHELL  EXIT SIGNAL=%d, STATUS=%d,", WTERMSIG(w),WEXITSTATUS(w)); 
				if(close(from_shell[0])<0){
					fprintf(stderr,"error in closing pipe from shell"); 
					exit(1);
				}
				exit(0);
			}
		}
	}
return 0;
}

