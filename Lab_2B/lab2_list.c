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
#include <signal.h>
#include "SortedList.h"

#define BILLION 1000000000L

// general
int i;
int *key2hash;
long long counter = 0;
int errno;

// list variables
SortedList_t **list_list;
SortedListElement_t *list_elements;

// locking mechanisms
pthread_mutex_t *mutex_list;
int *exclusion_list;

// options
int iteration_num = 1;
int thread_num = 1;
int list_num = 1;
short yield_INSERT_flag = 0;
short yield_DELETE_flag = 0;
short yield_LOOKUP_flag = 0;
short mutex_flag = 0;
short spin_lock_flag = 0;
int opt_yield = 0;

// analysis
char test_name[32];
int op_num;
long long dt_run_time;
long long dt_per_op;
long long dt_wait_for_lock = 0;

void *list_func(void *cur_thread){
    int j, k, l;
    SortedListElement_t *temp;
    struct timespec start_wait_for_lock, end_wait_for_lock;
    int offset = *((int*) cur_thread) * iteration_num;
    // insert each key into list
    for (j = 0; j < iteration_num; ++j){
        if (mutex_flag){
            clock_gettime(CLOCK_MONOTONIC, &start_wait_for_lock); /* mark start time */
            pthread_mutex_lock(&mutex_list[key2hash[j+offset]]);
            clock_gettime(CLOCK_MONOTONIC, &end_wait_for_lock);/* mark the end time */
            dt_wait_for_lock += BILLION * (end_wait_for_lock.tv_sec - start_wait_for_lock.tv_sec) + end_wait_for_lock.tv_nsec - start_wait_for_lock.tv_nsec;
            SortedList_insert(list_list[key2hash[j+offset]], &list_elements[j + offset]);
            pthread_mutex_unlock(&mutex_list[key2hash[j+offset]]);
        }
        else if (spin_lock_flag){
            clock_gettime(CLOCK_MONOTONIC, &start_wait_for_lock); /* mark start time */
            while (__sync_lock_test_and_set(&exclusion_list[key2hash[j+offset]], 1));
            clock_gettime(CLOCK_MONOTONIC, &end_wait_for_lock);/* mark the end time */
            dt_wait_for_lock += BILLION * (end_wait_for_lock.tv_sec - start_wait_for_lock.tv_sec) + end_wait_for_lock.tv_nsec - start_wait_for_lock.tv_nsec;
            SortedList_insert(list_list[key2hash[j+offset]], &list_elements[j + offset]);
            __sync_lock_release(&exclusion_list[key2hash[j+offset]]);
        }
        else {
            SortedList_insert(list_list[key2hash[j+offset]], &list_elements[j + offset]);
        }
    }
    for(l = 0; l < list_num; ++l){
    // get list length
    if (mutex_flag){
        clock_gettime(CLOCK_MONOTONIC, &start_wait_for_lock); /* mark start time */
        pthread_mutex_lock(&mutex_list[l]);
        clock_gettime(CLOCK_MONOTONIC, &end_wait_for_lock);/* mark the end time */
        dt_wait_for_lock += BILLION * (end_wait_for_lock.tv_sec - start_wait_for_lock.tv_sec) + end_wait_for_lock.tv_nsec - start_wait_for_lock.tv_nsec;
        if (SortedList_length(list_list[l]) == -1){
            fprintf(stderr, "Corrupted list! LENGTH (%s)\n", strerror(errno));
            exit(2);
        }
        pthread_mutex_unlock(&mutex_list[l]);
    }
    else if (spin_lock_flag){
        clock_gettime(CLOCK_MONOTONIC, &start_wait_for_lock); /* mark start time */
        while (__sync_lock_test_and_set(&exclusion_list[l], 1));
        clock_gettime(CLOCK_MONOTONIC, &end_wait_for_lock);/* mark the end time */
        dt_wait_for_lock += BILLION * (end_wait_for_lock.tv_sec - start_wait_for_lock.tv_sec) + end_wait_for_lock.tv_nsec - start_wait_for_lock.tv_nsec;
        if (SortedList_length(list_list[l]) == -1){
            fprintf(stderr, "Corrupted list! LENGTH (%s)\n", strerror(errno));
            exit(2);
        }
        __sync_lock_release(&exclusion_list[l]);
    }
    else {
        if (SortedList_length(list_list[l]) == -1){
            fprintf(stderr, "Corrupted list! LENGTH (%s)\n", strerror(errno));
            exit(2);
        }
    }
    }
    // look up + delete each key
    for (k = 0; k < iteration_num; ++k){
        if (mutex_flag){
            clock_gettime(CLOCK_MONOTONIC, &start_wait_for_lock); /* mark start time */
            pthread_mutex_lock(&mutex_list[key2hash[k+offset]]);
            clock_gettime(CLOCK_MONOTONIC, &end_wait_for_lock);/* mark the end time */
            dt_wait_for_lock += BILLION * (end_wait_for_lock.tv_sec - start_wait_for_lock.tv_sec) + end_wait_for_lock.tv_nsec - start_wait_for_lock.tv_nsec;
            temp = SortedList_lookup(list_list[key2hash[k+offset]], list_elements[k + offset].key);
            if (temp == NULL){
                fprintf(stderr, "Corrupted list! FIND (%s)\n", strerror(errno));
                exit(2);
            }
            if (SortedList_delete(temp)){
                fprintf(stderr, "Corrupted list! DELETE (%s)\n", strerror(errno));
                exit(2);
            }
            pthread_mutex_unlock(&mutex_list[key2hash[k+offset]]);
        }
        else if (spin_lock_flag){
            clock_gettime(CLOCK_MONOTONIC, &start_wait_for_lock); /* mark start time */
            while (__sync_lock_test_and_set(&exclusion_list[key2hash[k+offset]], 1));
            clock_gettime(CLOCK_MONOTONIC, &end_wait_for_lock);/* mark the end time */
            dt_wait_for_lock += BILLION * (end_wait_for_lock.tv_sec - start_wait_for_lock.tv_sec) + end_wait_for_lock.tv_nsec - start_wait_for_lock.tv_nsec;
            temp = SortedList_lookup(list_list[key2hash[k+offset]], list_elements[k + offset].key);
            if (temp == NULL){
                fprintf(stderr, "Corrupted list! FIND (%s)\n", strerror(errno));
                exit(2);
            }
            if (SortedList_delete(temp)){
                fprintf(stderr, "Corrupted list! DELETE (%s)\n", strerror(errno));
                exit(2);
            }
            __sync_lock_release(&exclusion_list[key2hash[k+offset]]);
        }
        else {
            temp = SortedList_lookup(list_list[key2hash[k+offset]], list_elements[k + offset].key); if (temp == NULL){
                fprintf(stderr, "Corrupted list! FIND (%s)\n", strerror(errno));
                exit(2);
            }
            if (SortedList_delete(temp)){
                fprintf(stderr, "Corrupted list! DELETE (%s)\n", strerror(errno));
                exit(2);
            }
        }
    }
    return 0;
}

//Hashing function
int hash_func(const char *key){
	return ((int) *key) % list_num; 
}
//SIGSEGV handler
void signal_handler(int signum){
    if (signum==SIGSEGV){
        fprintf(stderr, "SIGSEGV Caught!(%s)\n",strerror(errno));
            exit(2);
    }
}

int main (int argc, char *argv[])
{
    // setup signal handler
    signal(SIGSEGV, signal_handler);

    // option processing
    static struct option long_options[] =
    {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", required_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {"lists", required_argument, NULL, 'l'},
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
                for (i = 0; optarg[i] != '\0'; ++i){
                    if (optarg[i] == 'i')
                        yield_INSERT_flag = 1;
                    else if (optarg[i] == 'd')
                        yield_DELETE_flag = 1;
                    else if (optarg[i] == 'l')
                        yield_LOOKUP_flag = 1;
                }
                break;
            case 's':
                if (optarg[0] == 'm')
                    mutex_flag = 1;
                else if (optarg[0] == 's')
                    spin_lock_flag = 1;
                break;
            case 'l':
		// NOTE: we assume the user will not enter negative list number
		list_num = atoi(optarg);
		break;
            default:
                fprintf(stderr, "Error: unrecognized argument.\n");
                exit(1);
        }
    }
    //option processing
    strcpy(test_name, "list-");
    //yieldopts
    if (yield_INSERT_flag){
        strcat(test_name, "i");
        opt_yield |= INSERT_YIELD;
    }
    if (yield_DELETE_flag){
        strcat(test_name, "d");
        opt_yield |= DELETE_YIELD;
    }
    if (yield_LOOKUP_flag){
        strcat(test_name, "l");
        opt_yield |= LOOKUP_YIELD;
    }
    if (!yield_LOOKUP_flag && !yield_INSERT_flag && !yield_DELETE_flag)
        strcat(test_name, "none");
    //syncopts
    if (spin_lock_flag){
        strcat(test_name, "-s");
        exclusion_list = malloc(sizeof(int)*list_num);
        for (i = 0; i < list_num; ++i)
            exclusion_list[i] = 0;
    }
    else if (mutex_flag){
        strcat(test_name, "-m");
        mutex_list = malloc(sizeof(pthread_mutex_t)*list_num);
        for (i = 0; i < list_num; ++i)
            pthread_mutex_init(&mutex_list[i], NULL);
    }
    else
        strcat(test_name, "-none");

    // initialize lists 
    list_list = (SortedList_t **) malloc(sizeof(SortedList_t *)*list_num);
    for (i = 0; i < list_num; ++i){
            list_list[i] = malloc(sizeof(SortedList_t));
	    (list_list[i])->key = NULL;
	    (list_list[i])->next = list_list[i];
	    (list_list[i])->prev = list_list[i];
    }

    // initialize random keys
    list_elements = malloc(sizeof(SortedListElement_t) * thread_num * iteration_num);
    char *random_key;
    srand(time(NULL));
    for (i = 0; i < thread_num * iteration_num; ++i){
        random_key = malloc(sizeof(char));
        *random_key = "abcdefghijklmnopqrstuvwxyz"[random()%26];
        list_elements[i].key = random_key;
    }

    // initialize hash table
    key2hash = malloc(sizeof(int) * thread_num * iteration_num);
    for (i = 0; i < thread_num * iteration_num; ++i){
        key2hash[i] = hash_func(list_elements[i].key);
    }

    // get start time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start); /* mark start time */

    // initialize threads
    pthread_t *tid = malloc(thread_num * sizeof(pthread_t));
    int* targ = malloc(thread_num * sizeof(int));
    for (i = 0; i < thread_num; ++i){
        targ[i] = i;
        pthread_create(&(tid[i]), NULL, list_func, &(targ[i]));
    }

    // wait for threads
    for (i = 0; i < thread_num; ++i)
        pthread_join(tid[i],NULL);

    // get end time
    clock_gettime(CLOCK_MONOTONIC, &end);/* mark the end time */

    // free memory
    for (i = 0; i < list_num; ++i){
        SortedList_t *to_free = list_list[i];
        free(to_free);
    }
    //free(list_list);
    free(list_elements);
    free(tid);
    free(targ);
    free(key2hash);
    free(mutex_list);
    free(exclusion_list);

    // process time
    dt_run_time = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    op_num = thread_num * iteration_num * 3;
    dt_per_op = dt_run_time / op_num;
    dt_wait_for_lock /= op_num;
    printf("%s,%d,%d,%d,%d,%lld,%lld,%lld\n", test_name, thread_num, iteration_num, list_num, op_num, dt_run_time, dt_per_op, dt_wait_for_lock);
    exit(0);
}
