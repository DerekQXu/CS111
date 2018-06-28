/* Derek Xu
 * derekqxu@g.ucla.edu
 * 704751588
 */
#include <unistd.h>
#include <getopt.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>

#include <stdio.h>
#include <signal.h>
#include <mraa/gpio.h>
#include <mraa/aio.h>
#include <math.h>
#include <time.h>

#include <openssl/ssl.h>

// general
sig_atomic_t volatile run_flag = 1;
int i;
float res, temperature;
enum scale{
    Celsius = 0,
    Fahrenheit = 1
};
struct pollfd fd;
char command_buf[256] = {0};
char input_buf[256] = {0};
char shell_buf[256] = {0};

// TCP connection
int id;
char *host;
int port_num;
int sock;
struct hostent *server_name;
struct sockaddr_in server;

// SSL connection
SSL *sslClient;
SSL_CTX *sslContext;

// options
int report_flag = 1;
int sample_rate = 1; //per second
int unit_flag = Fahrenheit;
char *log_filename;
int log_flag = 0;
int log_file = 0;

// sensor initialization
mraa_aio_context temp_sensor;
mraa_gpio_context button;
time_t rawtime;
struct tm *info;

// unit conversion
void raw2temp(int raw, float *temp){
    res = (float) (1023-raw)*10000/raw;
    // datasheet gives Celsius conversion
    *temp = 1/(log(res/10000)/3975+1/298.15)-273.15;
    // manual Fahrenheit conversion
    if (unit_flag == Fahrenheit)
        *temp = *temp * 1.8 + 32;
}

// button interrupt handler
void log_and_exit()
{
    run_flag = 0;
}

// sensor polling
void *sensor_poll()
{
    // setup timing
    time(&rawtime);
    info = localtime(&rawtime);
    int current_sec = info->tm_sec;
    int counter = 0;
    // sensor processing
    while (run_flag)
    {
        // grab times
        time(&rawtime);
        info = localtime(&rawtime);
        // increment counter (elapsed seconds)
        if (info->tm_sec != current_sec){
            current_sec = info->tm_sec;
            ++counter;
        }
        // check counter
        if (counter >= sample_rate){
            counter = 0;
            memset(shell_buf, 0, 256);
            raw2temp(mraa_aio_read(temp_sensor), &temperature);
            sprintf(shell_buf, "%02d:%02d:%02d %.1f\n", info->tm_hour, info->tm_min, info->tm_sec, temperature);
            // write to log
            if (report_flag && run_flag){
                SSL_write( sslClient, shell_buf, strlen(shell_buf) );
		if (log_flag)
			write(log_file, shell_buf, strlen(shell_buf));
            }
        }
    }
    return 0;
}

// command processing
void *command_proc()
{
    int read_size, offset;
    // while loop
    while (run_flag)
    {
        poll(&fd, 1, 0);
        if (fd.revents & POLLIN){
            // get command
            read_size = SSL_read( sslClient, input_buf, 256);
            offset = 0;
            for (i = 0; input_buf[i] != '\0'; ++i){
                // parse input
                if (input_buf[i] == '\n'){
                    // create command_buf
                    memset(command_buf, 0, 256);
                    memcpy(command_buf, &input_buf[offset], i-offset+1);
                    offset = i+1;

                    // command processing
                    if (strstr(command_buf,"SCALE=")){
                        if (read_size > 7){
                            if (command_buf[6] == 'C')
                                unit_flag = Celsius;
                            else if (command_buf[6] == 'F')
                                unit_flag = Fahrenheit;
                        }
                    }
                    else if (strstr(command_buf,"PERIOD=")){
                        if (read_size > 8){
                            char temp [256] = {0};
                            memcpy(temp, &command_buf[7], strlen(command_buf)-8);
                            sample_rate = atoi(temp);
                        }
                    }
                    else if (strstr(command_buf,"STOP"))
                        report_flag = 0;
                    else if (strstr(command_buf,"START"))
                        report_flag = 1;
                    else if (strstr(command_buf,"LOG ")){
                        ;/*
                            if (read_size > 5){
                            char temp [256] = {0};
                            memcpy(temp, &command_buf[4], read_size-5);
                            if (log_flag)
                            write(log_file, temp, strlen(temp));
                            }
                            */
                    }
                    else if (strstr(command_buf,"OFF"))
                        run_flag = 0;
                    else
                        continue;
                    // write to log
                    if (log_flag)
                        write(log_file, command_buf, strlen(command_buf));
                }
            }
            memset(input_buf, 0, 256);
        }
        else if (fd.revents & (POLLHUP | POLLERR | POLLHUP)){
            fprintf(stderr, "Error: Polling\n");
            exit(1);
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    // option parsing
    static struct option long_options[] =
    {
        {"period", required_argument, NULL, 'p'},
        {"scale", required_argument, NULL, 's'},
        {"log", required_argument, NULL, 'l'},
        {"id", required_argument, NULL, 'i'},
        {"host", required_argument, NULL, 'h'},
        {0,0,0,0}
    };
    while ((i = getopt_long(argc, argv, "p:s:l", long_options, 0)) != -1)
    {
        switch(i)
        {
            case 'p':
                sample_rate = atoi(optarg);
                break;
            case 's':
                if (optarg[0] == 'C')
                    unit_flag = Celsius;
                else if (optarg[0] == 'F')
                    unit_flag = Fahrenheit;
                break;
            case 'l':
                log_flag = 1;
                log_filename = optarg;
                break;
            case 'i':
                id = atoi(optarg);
                break;
            case 'h':
                host = optarg;
                break;
            default:
                fprintf(stderr, "Error: unrecognized argument.\n");
                exit(1);
        }
    }
    if (argc > optind)
        for (i = 1; i < argc; ++i)
            if (argv[i][0] != '-')
                port_num = atoi(argv[i]);

    // setup logging
    if (log_flag){
        log_file = open(log_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (log_file < 0){
            fprintf(stderr, "Cannot open file.\n");
            exit(1);
        }
    }

    // setup the pins
    temp_sensor = mraa_aio_init(1);
    button = mraa_gpio_init(60);
    mraa_gpio_dir(button, MRAA_GPIO_IN);
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &log_and_exit, NULL);

    //setup SSL
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    sslContext = SSL_CTX_new(TLSv1_client_method());
    if (sslContext == NULL){
        fprintf(stderr, "Cannot get SSL context. Error.\n");
        exit(1);
    }
    sslClient = SSL_new(sslContext);
    if (sslClient == NULL){
        fprintf(stderr, "Cannot get SSL structure. Error.\n");
        exit(1);
    }
    //setup socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        fprintf(stderr, "Could not create socket");
        exit(0);
    }
    server_name = gethostbyname(host);
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
        fprintf(stderr, "connect failed. Error.\n");
        exit(1);
    }
    if (!SSL_set_fd(sslClient,sock) )
        fprintf(stderr, "error can not connect SSL to fd\n");
    int sslErr = SSL_connect(sslClient);
    if(sslErr != 1)
        fprintf(stderr, "Server rejected connection\n");

    // send ID
    memset(shell_buf, 0, 256);
    sprintf(shell_buf, "ID=%d\n", id);
    SSL_write( sslClient, shell_buf, strlen(shell_buf) );
    if (log_flag)
        write(log_file, shell_buf, strlen(shell_buf));

    // setup poll
    fd.fd = sock;
    fd.events = POLLIN|POLLHUP|POLLERR|POLLHUP;

    // setup multithreading
    pthread_t *tid;
    tid = malloc(sizeof(pthread_t)*2);
    pthread_create(&tid[0], NULL, sensor_poll, NULL);
    pthread_create(&tid[1], NULL, command_proc, NULL);

    // wait for threads
    for (i = 0; i < 2; ++i)
        pthread_join(tid[i], NULL);

    // close the pins
    time(&rawtime);
    info = localtime(&rawtime);
    memset(shell_buf, 0, 256);
    sprintf(shell_buf, "%02d:%02d:%02d SHUTDOWN\n", info->tm_hour, info->tm_min, info->tm_sec);
    SSL_write( sslClient, shell_buf, strlen(shell_buf) );
    // write to shell
    if (log_flag)
        write(log_file, shell_buf, strlen(shell_buf));

    // close everything
    SSL_shutdown(sslClient);
    SSL_free(sslClient);
    mraa_aio_close(temp_sensor);
    mraa_gpio_close(button);
    close(log_file);
    exit(0);
}
