/* NAME: Derek Xu
 * EMAIL: derekqxu@g.ucla.edu
 * ID: 704751588
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#define BILLION 1000000000L

// general
int i;
long long counter = 0;

// locking mechanisms
pthread_mutex_t mutex;
volatile int exclusion;

// options
int iteration_num = 1;
int thread_num = 1;
short mutex_flag = 0;
short spin_lock_flag = 0;
short compare_swap_flag = 0;
short opt_yield = 0;

// analysis
char test_name[32];
int op_num;
long long dt_run_time;
long long dt_per_op;

void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
}

void *add_func(){
    int j, k;
    // add 1
    for (j = 0; j < iteration_num; ++j){
        if (mutex_flag){
            pthread_mutex_lock(&mutex);
            add(&counter, 1);
            pthread_mutex_unlock(&mutex);
        }
        else if (spin_lock_flag){
            while (__sync_lock_test_and_set(&exclusion, 1));
            add(&counter, 1);
            __sync_lock_release(&exclusion);
        }
        else if (compare_swap_flag){
            long long cur;
            long long next;
            do {
                cur = counter;
                next = cur + 1;
                if (opt_yield)
                    sched_yield();
            } while (__sync_val_compare_and_swap(&counter, cur, next) != cur);
        }
        else{
            add(&counter, 1);
        }
    }
    // subtract 1
    for (k = 0; k < iteration_num; ++k){
        if (mutex_flag){
            pthread_mutex_lock(&mutex);
            add(&counter, -1);
            pthread_mutex_unlock(&mutex);
        }
        else if (spin_lock_flag){
            while (__sync_lock_test_and_set(&exclusion, 1));
            add(&counter, -1);
            __sync_lock_release(&exclusion);
        }
        else if (compare_swap_flag){
            long long cur;
            long long next;
            do {
                cur = counter;
                next = cur - 1;
                if (opt_yield)
                    sched_yield();
            } while (__sync_val_compare_and_swap(&counter, cur, next) != cur);
        }
        else{
            add(&counter, -1);
        }
    }
    return 0;
}

int main (int argc, char *argv[])
{
    // option processing
    static struct option long_options[] =
    {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", no_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {0,0,0,0}
    };
    while ((i = getopt_long(argc, argv, "t:i:y:s", long_options, 0))!= -1)
    {
        switch (i)
        {
            case 't':
                thread_num = atoi(optarg);
                break;
            case 'i':
                iteration_num = atoi(optarg);
                break;
            case 'y':
                opt_yield = 1;
                break;
            case 's':
                if (optarg[0] == 'm')
                    mutex_flag = 1;
                else if (optarg[0] == 's')
                    spin_lock_flag = 1;
                else if (optarg[0] == 'c')
                    compare_swap_flag = 1;
                break;
            default:
                fprintf(stderr, "Error: unrecognized argument.\n");
                exit(1);
        }
    }
    //option processing
    strcpy(test_name, "add-");
    if (opt_yield)
        strcat(test_name, "yield-");
    if (mutex_flag){
        strcat(test_name, "m");
        pthread_mutex_init(&mutex, NULL);
    }
    else if (spin_lock_flag)
        strcat(test_name, "s");
    else if (compare_swap_flag)
        strcat(test_name, "c");
    else
        strcat(test_name, "none");

    // get start time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start); /* mark start time */

    // initialize threads
    pthread_t *tid = malloc(thread_num*sizeof(pthread_t));
    for (i = 0; i < thread_num; ++i)
        pthread_create(&(tid[i]), NULL, add_func, NULL);

    // wait for threads
    for (i = 0; i < thread_num; ++i)
        pthread_join(tid[i],NULL);

    // get end time
    clock_gettime(CLOCK_MONOTONIC, &end);/* mark the end time */

    // free memory
    free(tid);

    // process time
    dt_run_time = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    op_num = thread_num * iteration_num * 2;
    dt_per_op = dt_run_time / op_num;
    printf("%s,%d,%d,%d,%lld,%lld,%lld\n", test_name, thread_num, iteration_num, op_num, dt_run_time, dt_per_op, counter);
    exit(0);
}
