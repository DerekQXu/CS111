/* NAME: Derek Xu
 * EMAIL: derekqxu@g.ucla.edu
 * ID: 704751588
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <getopt.h>
#include <sys/wait.h>
#include <string.h>
#include <poll.h>
#include <errno.h>

/* for reference:
 * \003 - ^C
 * \004 - ^D
 * 0x0A - Line Feed
 * 0x0D - Carriage Return
 * 0x04 - End of File
 */

// use this variable to remember original terminal attributes
struct termios saved_attributes;

// variables for inter-process control:
// pid, pipes, polls
short pid_flag = 0;
int pid, to_child[2], from_child[2], i;
struct pollfd fdtab[2];

// for error handling
extern int errno;

/* NOTE: This is a SIGPIPE (NOT SIGINT) handler. It is used in the following extreme case:
 *	 If we "forcefully writing to the pipe" when the child was dead before the program exits -> SIGPIPE raised
 *	 I tested this above case (by delaying the exit provided in POLLERR handling)... the handler was called. :)
 */
void signal_handler(int signum){
	if (signum==SIGPIPE){
		if (close(from_child[0]) < 0)
			fprintf(stderr, "Error: Close 1 in Signal Handler (%s)\n", strerror(errno));
		if (close(to_child[1]) < 0)
			fprintf(stderr, "Error: Close 2 in Signal Handler (%s)\n", strerror(errno));
		exit(0);
	}
}

// reset original terminal attributes
void reset_input_mode (void)
{
	tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
	if (pid_flag){
		int status;
		if (waitpid(pid, &status, 0) < 0)
			fprintf(stderr, "Error: waitpid in terminal reset (%s)\n", strerror(errno));
		// NOTE: I did not use WIFEXITSTATUS and the like, b/c these functions behave properly in both signal and normal exit
		int stat = WEXITSTATUS(status);
		int sig = WTERMSIG(status);
		fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n",sig,stat);	
	}
}

// set new terminal attributes
void set_input_mode (void)
{
	struct termios tattr;

	// Make sure stdin is a terminal
	if (!isatty (STDIN_FILENO))
	{
		fprintf (stderr, "Not a terminal.\n");
		exit (EXIT_FAILURE);
	}

	// Save the terminal attributes so we can restore them later
	tcgetattr (STDIN_FILENO, &saved_attributes);
	atexit (reset_input_mode);

	// Set the funny terminal modes (got from the spec)
	tcgetattr (STDIN_FILENO, &tattr);
	tattr.c_iflag = ISTRIP;
	tattr.c_oflag = 0;
	tattr.c_lflag = 0;
	tcsetattr (STDIN_FILENO, TCSANOW, &tattr);
}

int main (int argc, char **argv){
	// allow proper handling of SIGPIPE
	if (signal(SIGPIPE, signal_handler) == SIG_ERR)
		fprintf(stderr, "Error: signal handling in main (%s)\n", strerror(errno));

	//parse options
	static struct option long_options[] =
	{
		{"shell", no_argument, NULL, 's'},
		{0,0,0,0}
	};
	while ((i = getopt_long(argc, argv, "s", long_options, 0))!= -1)
	{
		switch (i)
		{
			case 's':
				pid_flag = 1;
				break;
			default:
				fprintf(stderr, "Error: unrecognized argument.\nCorrect Usage:\n--shell\n<none>\n");
				exit(1);
		}
	}


	// apply terminal settings
	set_input_mode ();

	//multiprocessing setup
	if (pid_flag){
		// setup the pipes
		if (pipe(to_child)==-1)
		{
			fprintf(stderr, "Pipe Failed (%s)", strerror(errno));
			return 1;
		}
		if (pipe(from_child)==-1)
		{
			fprintf(stderr, "Pipe Failed (%s)", strerror(errno));
			return 1;
		}	
		// fork the processes
		pid = fork();
		/* NOTE: From TA OH: I do not need to check the validity of the system calls here to strerror for following reasons:
		*	 1. Failure to properly setup the pipes will cause the program to poll POLLERR|POLLHUP
		*	    This will return the polling error: i.e. no undefined behavior  
		*	 2. Adding these extra if(close(pipe))fprintf("Err...",strerror(errno)); will obfuscate code and hurt readability
		*	 3. Adding these extra "..." will slow the program at cost of an already caught and descriptive error
		*/
		if (pid > 0){
			// close unused pipes
			close(to_child[0]);
			close(from_child[1]);
			// setup polling (only used in parent process)
			fdtab[0].fd = STDIN_FILENO;
			fdtab[0].events = POLLIN|POLLHUP|POLLERR;
			fdtab[1].fd = from_child[0];
			fdtab[1].events = POLLIN|POLLHUP|POLLERR;
		}
		else if (pid == 0){
			// close unused pipes
			close(to_child[1]);
			close(from_child[0]);
			// map stdin stdout and stderr to corresp. pipes
			dup2(to_child[0], STDIN_FILENO);
			dup2(from_child[1], STDOUT_FILENO);
			dup2(from_child[1], STDERR_FILENO);
			// close unused pipes
			// stdin, stdout, and stderr used to transmit data thru pipes
			close(to_child[0]);
			close(from_child[1]);
			// transfer proc. control to execvp
			char *myargs[1];
			myargs[0] = "/bin/bash";
			myargs[1] = NULL;
			execvp(myargs[0], myargs);
		}
	}

	// main program:
	char concat_str[256];
	int len;
	while (1)
	{
		// check which state the program is in (from pole)
		// it will always execute the first else-if IFF --shell not selected
		poll(fdtab, 2, 0);
		// write shell output
		if ((fdtab[1].revents & POLLIN) && pid_flag){
			// read 256 char from the shell at a time
			len = read(from_child[0], concat_str, 256);
			// properly display the read characters
			for(i = 0; i < len; ++i){
				if (concat_str[i] == 0x0A || concat_str[i] == 0x0D){
					concat_str[i] = 0x0D;
					if (write(STDOUT_FILENO, &concat_str[i], 1) < 0)
						fprintf(stderr, "Error: Write output <cr><lf> to screen (%s)\n", strerror(errno));
					concat_str[i] = 0x0A;
					if (write(STDOUT_FILENO, &concat_str[i], 1) < 0)
						fprintf(stderr, "Error: Write output <cr><lf> to screen (%s)\n", strerror(errno));
				}
				else
					if (write(STDOUT_FILENO, &concat_str[i], 1) < 0)
						fprintf(stderr, "Error: Write output to screen (%s)\n", strerror(errno));
			}
		}
		// write user input
		else if ((fdtab[0].revents & POLLIN) || (pid_flag == 0)){
			// read 256 char from terminal at a time
			len = read (STDIN_FILENO, concat_str, 256);
			if (len < 0)
				fprintf(stderr, "Error: Read in main (%s)\n", strerror(errno));
			// properly handle read in characters
			for (i = 0; i < len; ++i){
				// ^D handling
				if (concat_str[i] == '\004'){
					if (pid_flag){
						if (close(to_child[1]) < 0)
							fprintf(stderr, "Error: ^D in main (%s)\n", strerror(errno));
					}
					else
						exit(0);
				}
				// ^C handling
				else if (concat_str[i] == '\003'){
					if (pid_flag){
						if (kill(pid, SIGINT) < 0)
							fprintf(stderr, "Error: ^C in main (%s)\n", strerror(errno));
					}
					else {
						if (write(STDIN_FILENO, &concat_str[i], 1) < 0)
							fprintf(stderr, "Error: Write input to screen (%s)\n", strerror(errno));
					}
				}
				// <cr><lf> handling
				else if (concat_str[i] == 0x0A || concat_str[i] == 0x0D){
					concat_str[i] = 0x0D;
					if (write(STDIN_FILENO, &concat_str[i], 1) < 0)
						fprintf(stderr, "Error: Write <cr><lf> to screen (%s)\n", strerror(errno));
					concat_str[i] = 0x0A;
					if (pid_flag){
						if (write(to_child[1], &concat_str[i], 1) < 0)
							fprintf(stderr, "Error: Write <cr><lf> to shell (%s)\n", strerror(errno));
					}
					if (write(STDIN_FILENO, &concat_str[i], 1) < 0)
						fprintf(stderr, "Error: Write <cr><lf> to screen (%s)\n", strerror(errno));
				}
				// regular input
				else
				{
					if (write(STDIN_FILENO, &concat_str[i], 1) < 0)
						fprintf(stderr, "Error: Write input to screen (%s)\n", strerror(errno));
					if (pid_flag){
						if (write(to_child[1], &concat_str[i], 1) < 0)
							fprintf(stderr, "Error: Write input to shell (%s)\n", strerror(errno));
					}
				}
			}
		}
		// check for any errors
		if ((fdtab[1].revents & (POLLERR|POLLHUP)) && pid_flag){
			if (close(from_child[0]) < 0)
				fprintf(stderr, "Error: cannot close child pipe (%s)\n", strerror(errno));
			break;
		}
	}
	// normal program exit
	exit(0);
}
