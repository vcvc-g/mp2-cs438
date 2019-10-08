#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <sys/time.h>

#define msg_body_size 1460
#define sender_header_size 3
#define msg_total_size 1463
#define max_seq 720

typedef struct file_data_struct{
    size_t length;
    char* data;

    int status; /** -1 means not send
                 * 0  means send not ack
                 * 1 means ack 
                 **/
} file_data;

typedef struct sender_window{




} sw;

typedef struct sender_info {
    float timeout;
    float estimated_rtt;
    float dev_rtt;
    int congestion_state;
    int ssthresh;
    file_data* window_packet;
    int window_size;
    struct timeval* timer_start;
    int last_ack_seq;
} sender_info;


enum congestion_state {
    SLOW_START,
    CONGESTION_AVOID,
    FAST_RECOVERY,  
};



sender_info* senderInfo;

file_data* file_data_array;

int read_file(char* filename, unsigned long long int bytesToTransfer);

float timeout_interval(float sampled_rtt);

int adjust_window_size();

int init_sender();

