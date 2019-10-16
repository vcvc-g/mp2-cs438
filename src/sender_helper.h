#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>
#include <stdint.h>

#define max_window_size 360
#define msg_body_size 1460
#define sender_header_size 6
#define msg_total_size 1466
#define max_seq 720
#define million 1000

typedef struct file_data_struct{
    size_t length;
    int number;
    int seq;
    char* data;

    int status; /** 
                 * 0 means not send
                 * 1 means send not ack
                 * 2 means ack 
                 **/
} file_data;

typedef struct sender_window{




} sw;

typedef struct sender_info {
    double timeout;
    double estimated_rtt;
    double dev_rtt;
    int congestion_state;
    int ssthresh;
    file_data* window_packet;
    int window_size;
    struct timeval* timer_start;
    int last_ack_seq;
    int duplicate_ack;
    float ca_extra;
    volatile int handshake_state;
    int packet_number;
    int packet_sent;
} sender_info;


enum congestion_state {
    SLOW_START,
    CONGESTION_AVOID,
    FAST_RECOVERY,  
};

enum handshake_state {
    LISTEN,
    SYNSENT,
    ESTAB,
    CLOSE_WAIT,
    CLOSED
};



sender_info* senderInfo;

file_data* file_data_array;

int read_file(char* filename, unsigned long long int bytesToTransfer);

double timeout_interval(int RTT, int time);

int adjust_window_size(int timeout_flag, int duplicate_flag);

int init_sender();

