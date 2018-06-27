/* NAME: Derek Xu
 * EMAIL: derekqxu@g.ucla.edu
 * ID: 704751588 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>

extern int errno ;

void segfault()
{
    char *temp = NULL;
    *temp = 'c';
}

void handler(int sig){
    //NOTE: not using strerror b/c not a system call error
    if (sig == SIGSEGV){
        fprintf(stderr, "Error: caught and received SIGSEGV.\n");
        exit(4);
    }
}

int main (int argc, char **argv){
    // NOTE: I removed this line b/c it did not pass the sanity check
    /*
    if (argc == 1)
        exit(1);
    */

    static struct option long_options[] =
    {
        {"input", required_argument, NULL, 'i'},
        {"output", required_argument, NULL, 'o'},
        {"segfault", no_argument, NULL, 's'},
        {"catch", no_argument, NULL, 'c'},
        {0,0,0,0}
    };

    // check which options were specified
    int c;
    short flag_seg, flag_hand;
    char *input_path, *output_path;
    flag_seg = flag_hand = 0;
    input_path = output_path = 0;

    while ((c = getopt_long(argc, argv, "i:o:s:c", long_options, 0))!= -1)
    {
        switch (c)
        {
            case 'i':
                input_path = optarg;
                break;
            case 'o':
                output_path = optarg;
                break;
            case 's':
                flag_seg = 1;
                break;
            case 'c':
                flag_hand = 1;
                break;
            default:
                //NOTE: not using strerror b/c not a system call error
                fprintf(stderr, "Error: unrecognized argument.\nCorrect Usage:\n--input=(input_file)\n--output=(output_file)\n--segfault\n--catch\nExample of Correct Usage:\n$./lab0 --input=input.txt --output=output.txt\n");
                exit(1);
        }
    }

    // file redirection
    int input_fd, output_fd;

    if (input_path)
    {
        input_fd = open(input_path, O_RDONLY);
        if (input_fd < 0){
            fprintf(stderr, "Error %s: --input: no such file named %s.\n", strerror(errno), input_path);
            exit(2);
        }
        close(0);
        dup(input_fd);
        close(input_fd);
    }
    if (output_path)
    {
        output_fd = creat(output_path, 0666);
        if (output_fd < 0){
            fprintf(stderr, "Error %s: --output: no such file named %s.\n", strerror(errno), output_path);
            exit(3);
        }
        close(1);
        dup(output_fd);
        close(output_fd);
    }

    // register signal handler
    if (flag_hand)
        signal(SIGSEGV, handler);

    // cause the segfault
    if (flag_seg)
        segfault();

    // copy stdin to stdout
    char* buffer;
    buffer = (char *) malloc(sizeof(char));
    while ( read( 0, buffer, 1 ) )
    {
        write( 1, buffer, 1 );
    }

    exit(0);
}
