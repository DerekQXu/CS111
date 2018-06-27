#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <getopt.h>
#include <sys/wait.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <assert.h>
#include "zlib.h"

//general
int i, read_size;
extern int errno;

//multiprocessing
int pid;
int to_child[2], from_child[2], i;
struct pollfd fdtab[2];

//sockets
int socket_desc, client_sock, client_size;
struct sockaddr_in server, client;
int port_num;

//flags
int compress_flag = 0;

//SIGPIPE handler
void signal_handler(int signum){
        if (signum==SIGPIPE){
		fprintf(stderr, "SIGPIPE Caught!(%s)",strerror(errno));
                exit(0);
        }
}
int main(int argc, char *argv[]){
	//setup sigpipe handler
	signal(SIGPIPE, signal_handler);

	//option parsing
	static struct option long_options[] =
	{
		{"port", required_argument, NULL, 'p'},
		{"compress", no_argument, NULL, 'c'},
		{0,0,0,0}
	};
	while ((i = getopt_long(argc, argv, "p:c", long_options, 0))!= -1)
	{
		switch (i)
		{
			case 'p':
				port_num = atoi(optarg);
				break;
			case 'c':
				compress_flag = 1;
				break;
			default:
				fprintf(stderr, "Error: unrecognized argument.\n");
				exit(1);
		}
	}

	//setup socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
        {
                fprintf(stderr, "Could not create socket");
        }
	server.sin_family = AF_INET;
	server.sin_port = htons( port_num );
	server.sin_addr.s_addr = INADDR_ANY;
	//bind socket
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		fprintf(stderr,"bind failed. Error");
		exit(1);
	}
	//listen for client
	listen(socket_desc, 3);
	client_size = sizeof(struct sockaddr_in);
	client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&client_size);
	if (client_sock < 0)
	{
		fprintf(stderr,"accept failed");
		exit(1);
	}

	//create pipes
	pipe(to_child);
	pipe(from_child);
	//fork shell
	pid = fork();
	if (pid == 0){
		//setup pipes
		close(to_child[1]);
                close(from_child[0]);
                dup2(to_child[0], STDIN_FILENO);
                dup2(from_child[1], STDOUT_FILENO);
                dup2(from_child[1], STDERR_FILENO);
                close(to_child[0]);
                close(from_child[1]);
                char *myargs[1];
                myargs[0] = "/bin/bash";
                myargs[1] = NULL;
                execvp(myargs[0], myargs);
        }
	//setup pipes
        close(to_child[0]);
        close(from_child[1]);
	//setup polls
        fdtab[0].fd = client_sock;
        fdtab[0].events = POLLIN|POLLHUP|POLLERR;
        fdtab[1].fd = from_child[0];
        fdtab[1].events = POLLIN|POLLHUP|POLLERR;

	//setup compression
	z_stream shell_to_stdout;
        shell_to_stdout.zalloc = Z_NULL;
        shell_to_stdout.zfree = Z_NULL;
        shell_to_stdout.opaque = Z_NULL;
        if (compress_flag)
                deflateInit(&shell_to_stdout, Z_DEFAULT_COMPRESSION);
        z_stream stdin_to_shell;
        stdin_to_shell.zalloc = Z_NULL;
        stdin_to_shell.zfree = Z_NULL;
        stdin_to_shell.opaque = Z_NULL;
        if (compress_flag)
                inflateInit(&stdin_to_shell);
	
	//main program execution
	char client_message[4000];
        char shell_message[4000];
        char compressed_message [4000];
	while(1){
		if (poll(fdtab, 2, 0) < 0){
                	fprintf(stderr, "Poll error (%s)", strerror(errno));
			exit(1);
		}
		//client input
		if (fdtab[0].revents & POLLIN){
			memset(client_message, 0, sizeof(client_message));
			memset(compressed_message, 0, sizeof(compressed_message));
			//read input
			read_size = read(client_sock, compressed_message, 4000);
			if (read_size == 0){
				fprintf(stderr, "Client disconnected");
				fflush(stdout);
				exit(0);
			}
			//decompress input
			if (compress_flag){
				stdin_to_shell.avail_in = (uInt) read_size;
                                stdin_to_shell.next_in = (Bytef*) compressed_message;
                                stdin_to_shell.avail_out = (uInt) 4000;
                                stdin_to_shell.next_out = (Bytef*) client_message;
                                do{
                                        inflate(&stdin_to_shell, Z_SYNC_FLUSH);
                                }while(stdin_to_shell.avail_in > 0);
			}
			else{
				memcpy(client_message, compressed_message, sizeof(client_message));
			}
			//process input
			for(i = 0; i < read_size; ++i){
				if (client_message[i] == '\004'){
					if (close(to_child[1]) < 0)
						fprintf(stderr, "Error ^D");
				}
				if (client_message[i] == '\003'){
					if (kill(pid, SIGINT) < 0)
						fprintf(stderr, "Error ^C");
					close(socket_desc);
					close(client_sock);
				}
				if (client_message[i] == 0x0A || client_message[i] == 0x0D){
					client_message[i] = 0x0A;
					if (write(to_child[1], &client_message[i], 1) < 0)
						fprintf(stderr, "Error: Write <cr><lf> to shell (%s)\n", strerror(errno));

				}
				else
				{
					if (write(to_child[1], &client_message[i], 1) < 0)
						fprintf(stderr, "Error: Write input to shell (%s)\n", strerror(errno));
				}
			}
		}
		//shell output
		if (fdtab[1].revents & POLLIN){
			memset(shell_message, 0, sizeof(shell_message));
			memset(compressed_message, 0, sizeof(compressed_message));
			//read output 
			read_size = read(from_child[0], shell_message, 4000);
			//compress output 
			if (compress_flag){
				shell_to_stdout.avail_in = (uInt) read_size;
                                shell_to_stdout.next_in = (Bytef*) shell_message;
                                shell_to_stdout.avail_out = (uInt) 4000;
                                shell_to_stdout.next_out = (Bytef*) compressed_message;
                                do{
                                        deflate(&shell_to_stdout, Z_SYNC_FLUSH);
                                }while(shell_to_stdout.avail_in > 0);
				read_size = 4000-shell_to_stdout.avail_out;
			}
			else {
				memcpy(compressed_message, shell_message, sizeof(compressed_message));
			}
			//process output
			write(client_sock, compressed_message, read_size);
		}
		//error
		if (fdtab[1].revents & (POLLHUP | POLLERR)){
			if (compress_flag){
				inflateEnd(&stdin_to_shell);
				deflateEnd(&shell_to_stdout);
			}
			//close the pipe
			close(to_child[1]);
			//read EOF
			while(read(from_child[0], shell_message, 4000));
			//collect and report termination status 
			int status;
			if (waitpid(pid, &status, 0) < 0)
				fprintf(stderr, "Error: waitpid in terminal reset (%s)\n", strerror(errno));
			int stat = WEXITSTATUS(status);
			int sig = WTERMSIG(status);
			fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n",sig,stat);
			//close network socket
			close(socket_desc);
			close(client_sock);
			exit(1);
		}
	}
	exit (0);
} 
