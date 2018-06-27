#define _GNU_SOURCE

#include <signal.h>
#include <unistd.h>    //write
#include <termios.h>
#include <getopt.h>
#include <sys/wait.h>
#include <poll.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <assert.h>
#include <zlib.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>

//general
int i, read_size;
extern int errno;
char temp;

//polling
struct pollfd fdtab[2];

//sockets
int sock;
struct hostent *server_name;
struct sockaddr_in server;
int port_num;

//logging
int log_file = 0;
char* log_filename;

//flags
int compress_flag = 0;
int log_flag = 0;

//termios
struct termios saved_attributes;
void reset_input_mode (void)
{
        tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
        int status;
        int stat = WEXITSTATUS(status);
        int sig = WTERMSIG(status);
        fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n",sig,stat);
}
void set_input_mode (void)
{
        struct termios tattr;
        if (!isatty (STDIN_FILENO))
        {
                fprintf (stderr, "Not a terminal.\n");
                exit (EXIT_FAILURE);
        }
        tcgetattr (STDIN_FILENO, &saved_attributes);
        atexit (reset_input_mode);
        tcgetattr (STDIN_FILENO, &tattr);
        tattr.c_iflag = ISTRIP;
        tattr.c_oflag = 0;
        tattr.c_lflag = 0;
        tcsetattr (STDIN_FILENO, TCSANOW, &tattr);
}

//SIGPIPE handler
void signal_handler(int signum){
        if (signum==SIGPIPE){
		fprintf(stderr, "SIGPIPE Caught!(%s)",strerror(errno));
                exit(0);
        }
}

//ascii conversion
char* itoa(int val){
	//max return value is 4000
	static char buf[5] = {0};
	for(i = 30; val && i ; --i, val /= 10)
		buf[i] = "0123456789"[val % 10];
	return &buf[i+1];
}
/* NOTE:
 * Please note that the implementation details are quite straightforward:
 * we simply take the string "0123456789abcdef" and the corresp. array
 * elt. depending on the base and append it to a string which we return.
 * i.e. integer to string(a) conversion :)
 *
 * Although this code has been optimized for the project, inspiration is
 * partly from the itoa v0.1 code by Robert Schaper.
 */

int main(int argc, char *argv[]) {
	//setup sigpipe handler
	signal(SIGPIPE, signal_handler);
	
	//option parsing
	static struct option long_options[] =
        {
                {"port", required_argument, NULL, 'p'},
                {"compress", no_argument, NULL, 'c'},
                {"log", required_argument, NULL, 'l'},
                {0,0,0,0}
        };
        while ((i = getopt_long(argc, argv, "p:c:l", long_options, 0))!= -1)
        {
                switch (i)
                {
                        case 'p':
                                port_num = atoi(optarg);
                                break;
                        case 'c':
                                compress_flag = 1;
                                break;
                        case 'l':
                                log_flag = 1;
                                log_filename = optarg;
                                break;
                        default:
                                fprintf(stderr, "Error: unrecognized argument.\n");
                                exit(1);
                }
        }

	//setup logging
	if (log_flag){
		log_file = open(log_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (log_file < 0)
		{
			fprintf(stderr, "Cannot open file (%s)",strerror(errno));
			exit(1);
		}
	}

	//setup noncanonical
	set_input_mode();
	
	//setup socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
        {
                fprintf(stderr, "Could not create socket");
		exit(0);
        }
	server_name = gethostbyname("127.0.0.1");
	if (server_name == NULL)
        {
                fprintf(stderr, "Could not find host");
		exit(0);
        }
	memset((char*) &server, 0, sizeof(server));
        server.sin_family = AF_INET;
	memcpy((char*) &server.sin_addr.s_addr, (char*) server_name->h_addr, server_name->h_length);
        server.sin_port = htons( port_num );
	//connect	
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		fprintf(stderr, "connect failed. Error");
		exit(1);
	}

	//setup polls
        fdtab[0].fd = STDIN_FILENO;
        fdtab[0].events = POLLIN|POLLHUP|POLLERR|POLLRDHUP;
        fdtab[1].fd = sock;
        fdtab[1].events = POLLIN|POLLHUP|POLLERR|POLLRDHUP;
	
	//setup compression
        z_stream stdin_to_shell;
        stdin_to_shell.zalloc = Z_NULL;
        stdin_to_shell.zfree = Z_NULL;
        stdin_to_shell.opaque = Z_NULL;
        if (compress_flag)
                deflateInit(&stdin_to_shell, Z_DEFAULT_COMPRESSION);
        z_stream shell_to_stdout;
        shell_to_stdout.zalloc = Z_NULL;
        shell_to_stdout.zfree = Z_NULL;
        shell_to_stdout.opaque = Z_NULL;
        if (compress_flag)
                inflateInit(&shell_to_stdout);

	//main program execution
	char client_message[4000];
        char shell_message[4000];
        char compressed_message [4000];
	while (1){
		if (poll(fdtab, 2, 0) < 0){
                	fprintf(stderr, "Poll error (%s)",strerror(errno));
			exit(1);
		}
		//server input
		if (fdtab[0].revents & POLLIN){
			memset(client_message, 0, sizeof(client_message));
			memset(compressed_message, 0, sizeof(client_message));
			//read input
			read_size = read(STDIN_FILENO, client_message, 4000);
			//echo input
			for(i = 0; i < read_size; ++i){
                                if (client_message[i] == 0x0A || client_message[i] == 0x0D){
                                        temp = 0x0D;
                                        if (write(STDOUT_FILENO, &temp, 1) < 0)
                                                fprintf(stderr, "Error: Write <cr><lf> to screen (%s)\n", strerror(errno));
                                        temp = 0x0A;
                                        if (write(STDOUT_FILENO, &temp, 1) < 0)
                                                fprintf(stderr, "Error: Write <cr><lf> to screen (%s)\n", strerror(errno));

                                }
                                else{
                                        if (write(STDOUT_FILENO, &client_message[i], 1) < 0)
                                                fprintf(stderr, "Error: Write input to screen (%s)\n", strerror(errno));
                                }
                        }
			//compress input
			if (compress_flag){
				stdin_to_shell.avail_in = (uInt) read_size;
                                stdin_to_shell.next_in = (Bytef*) client_message;
                                stdin_to_shell.avail_out = (uInt) 4000;
                                stdin_to_shell.next_out = (Bytef*) compressed_message;
                                do{
                                        deflate(&stdin_to_shell, Z_SYNC_FLUSH);
                                }while(stdin_to_shell.avail_in > 0);
				read_size = 4000-stdin_to_shell.avail_out;
			}	
			else{
				memcpy(compressed_message, client_message, sizeof(compressed_message));
			}
			//process input
			if (write(sock, compressed_message, read_size) < 0)
				fprintf(stderr, "Error: writing to server");
			//write to log
			if (log_flag){
				write(log_file, "SENT ",5);
				char *buffer = itoa(read_size);	
				write(log_file, buffer, strlen(buffer));
				write(log_file, " bytes: ", 8);
				write(log_file, compressed_message, read_size);
				write(log_file, "\n", sizeof(char));
			}
		}
		//client output
		if (fdtab[1].revents & POLLIN){
			memset(shell_message, 0, sizeof(shell_message));
			memset(compressed_message, 0, sizeof(compressed_message));
			//read output
			read_size = read(sock, compressed_message, 4000);
			//write to log
			if (log_flag){
				write(log_file, "RECEIVED ", 9);
				char *buffer = itoa(read_size);	
				write(log_file, buffer, strlen(buffer));
				write(log_file, " bytes: ", 8);
				write(log_file, compressed_message, read_size);
				write(log_file, "\n", sizeof(char));
			}
			//decompress output
			if (compress_flag){
                                shell_to_stdout.avail_in = (uInt) read_size;
                                shell_to_stdout.next_in = (Bytef*) compressed_message;
                                shell_to_stdout.avail_out = (uInt) 4000;
                                shell_to_stdout.next_out = (Bytef*) shell_message;
                                do{
                                        inflate(&shell_to_stdout, Z_SYNC_FLUSH);
                                }while(shell_to_stdout.avail_in > 0);
                        }
			else
			{
				memcpy(shell_message, compressed_message, sizeof(shell_message));
			}
			//echo output
			for(i = 0; i < read_size; ++i){
                                if (shell_message[i] == 0x0A || shell_message[i] == 0x0D){
                                        shell_message[i] = 0x0D;
                                        if (write(STDOUT_FILENO, &shell_message[i], 1) < 0)
                                                fprintf(stderr, "Error: Write <cr><lf> to screen (%s)\n", strerror(errno));
                                        shell_message[i] = 0x0A;
                                        if (write(STDOUT_FILENO, &shell_message[i], 1) < 0)
                                                fprintf(stderr, "Error: Write <cr><lf> to screen (%s)\n", strerror(errno));
                                }
                                else
                                {
                                        if (write(STDOUT_FILENO, &shell_message[i], 1) < 0)
                                                fprintf(stderr, "Error: Write input to screen (%s)\n", strerror(errno));
                                }
                        }
		}
		//error
		if (fdtab[1].revents & (POLLHUP | POLLERR | POLLRDHUP)){
			if (compress_flag){
				deflateEnd(&stdin_to_shell);
				inflateEnd(&shell_to_stdout);
			}
			fprintf(stderr, "Error: POLLHUP, POLLERR");
			exit(0);
		}
	}
	exit(0);
}
