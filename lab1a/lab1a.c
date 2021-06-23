//NAME: Bryan Luo
//EMAIL: luobryan@ucla.edu
//ID: 605303956

#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h> 
#include <poll.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>

#define BUFFERSIZE 2048
char buffer[BUFFERSIZE]; 

void sigpipe_handler(){
	fprintf(stderr,"%s","error: sigpipe caught"); 
	exit(0);
}
void terminal_setup(void){
	struct termios tmp;
	if(tcgetattr(0,&tmp)<0){
		fprintf(stderr,"error in getting terminal settings"); 
		exit(1); 
	}
	tmp.c_iflag = ISTRIP; 
	tmp.c_oflag = 0; 
	tmp.c_lflag = 0; 
	if(tcsetattr(0, TCSANOW, &tmp)<0){
		fprintf(stderr,"error in setting terminal settings."); 
		exit(1); 
	} 

}
int main(int argc, char* argv[]){
	bool shell = false;
	struct option args[] = {
		{"shell",0,NULL,'s'},
		{0,0,0,0}
	}; 
	int x; 
	while ( (x=getopt_long(argc,argv,"",args,NULL)) != -1){
		switch(x)
		{
			case 's':
			shell = true;
			break;
			default:
			fprintf(stderr,"unacceptable arguments. the only accepted argument is --shell.");
			exit(1);
		}
	}	
	//setup terminal (set the stdin into non-canonical input, no echo mode)
        struct termios original;
        if(tcgetattr(0, &original)<0){
			fprintf(stderr,"error in getting terminal settings");
			exit(1); 
		}
        terminal_setup();
	if(!shell){
	//read from stdin, write to stdout
		char temp; 
		int stat = read(0,&temp,1);
		if (stat < 0){
			fprintf(stderr,"error in reading from stdin"); 
			exit(1);
		}
		while(stat > 0){
			if(temp == 0x4){
				write(1,"^D",2);
				tcsetattr(0, TCSANOW, &original);
				exit(0);
			}
			else if(temp=='\r'||temp=='\n'){
				write(1,"\r\n",2);
			}
			else{
				write(1,&temp,1);
			}
			stat = read(0,&temp,1); 
			if (stat < 0){
				fprintf(stderr,"error in reading from stdin"); 
				exit(1);
			}

		}

	}
	if(shell){
		signal(SIGPIPE,sigpipe_handler);
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
		int ret = fork();
		if (ret < 0){
			fprintf(stderr,"error in fork");
			exit(1); 
		}
		if (ret == 0){ //child process
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
		else if(ret > 0){  //parent process
			if(close(to_shell[0])<0){
				fprintf(stderr,"error in closing to shell pipe"); 
				exit(1);
			} //parent process doesn't read from this pipe
			if(close(from_shell[1])<0){
				fprintf(stderr,"error in closing from shell pipe.");
				exit(1); 
			} //parent process doesn't write to this pipe 
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
				if(pollfds[0].revents&POLLIN){
					int numBytesRead = read(0,buffer,BUFFERSIZE); //read input from keyboard (stdin)
					if (numBytesRead<0){
						fprintf(stderr,"error in reading");
						exit(1); 
					}
					//write values from stdin to stdout (echo) and shell
					for(i = 0; i < numBytesRead; i++){
						int a  = 0; 
						if(buffer[i] == '\r' || buffer[i] == '\n'){
                            if(write(1,"\r\n",2)<0){
								fprintf(stderr,"error in writing to stdout"); 
								exit(1); 
							}
							a = write(to_shell[1],"\n",1); 
                        }
                        else if(buffer[i] == 0x03){
							if(write(1,"^C",2)<0){
								fprintf(stderr,"error in writing to stdout");
								exit(1);
							} 
							if(kill(ret, SIGINT)<0){
								fprintf(stderr,"error in killing child shell");
								exit(1); 
							} 
						}
						else if(buffer[i] == 0x04){
							if(write(1,"^D",2)<0){
								fprintf(stderr,"error in writing to stdout");
								exit(1); 
							} 
							if(close(to_shell[1])<0){
								fprintf(stderr,"error in closing to shell pipe.");
								exit(1);
							} 
							int status = 0;
							if(waitpid(ret, &status, 0)<0){
								fprintf(stderr,"error in waitpid"); 
								exit(1); 
							} 
							if(tcsetattr(0, TCSANOW,&original)<0){
								fprintf(stderr,"error in restoring terminal settings.");
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
                            if(write(1,&buffer[i],1)<0){
								fprintf(stderr,"error in writing to stdout");
								exit(1); 
							}
							a = write(to_shell[1],&buffer[i],1); 
                        }
						if (a == SIGPIPE){
                            if(close(to_shell[1])<0){
								fprintf(stderr,"error in closing pipe to shell");
								exit(1); 
							}
                            int status = 0;
                            if(waitpid(ret, &status, 0)<0){
								fprintf(stderr,"error in waitpid for child process"); 
								exit(1); 
							}
                			if(tcsetattr(0, TCSANOW,&original)<0){
								fprintf(stderr,"error in setting terminal settings back to original");
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
				if(pollfds[1].revents & POLLIN){

					int numBytesRead = read(from_shell[0], buffer, BUFFERSIZE); //read input from child shell 
					//write values from child shell to stdout
					for(i = 0; i < numBytesRead; i++){
						if(buffer[i] == '\n'){  
							if(write(1,"\r\n",2)<0){
								fprintf(stderr,"error in wriitng to stdout"); 
								exit(1); 
							}
						}
						else{
							if(write(1,&buffer[i],1)<0){
								fprintf(stderr,"error in writing to stdout"); 
								exit(1); 
							}
						}
					}
				}
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
					if(tcsetattr(0, TCSANOW,&original)<0){
						fprintf(stderr,"error in setting terminal settings back to original");
						exit(1); 
					}
					exit(0); 
				}
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
                    fprintf(stderr,"SHELL  EXIT SIGNAL=%d, STATUS=%d,", WTERMSIG(w),WEXITSTATUS(w)); 
                    if(close(from_shell[0])<0){
						fprintf(stderr,"error in closing pipe from shell"); 
						exit(1);
					}
					if(tcsetattr(0, TCSANOW,&original)<0){
						fprintf(stderr,"error in setting terminal settings back to original"); 
					}
					exit(0);
				}
			}
		}
	}
	return 0;
}
