#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#define msg_body_size 1460
#define sender_header_size 3
#define msg_total_size 1463
#define max_seq 720

#define SWS 360


typedef struct file_data_struct{
    size_t length;
    char* data;
} file_data;

typedef struct sender_window{




} sw;

typedef struct sender_info {
    float timeout;
    float estimated_rtt;
    float dev_rtt;
    int congestion_state;
    int ssthresh;
    int packet[SWS];
    int window_size;
    int window_base;
    struct timeval timer_start;

} sender_info;


enum congestion_state {
    SLOW_START,
    CONGESTION_AVOID,
    FAST_RECOVERY,  
};

int read_file(char* filename, unsigned long long int bytesToTransfer);




