#include <stdio.h>
#include <sys/time.h>

#define msg_body_size 1460
#define SWS 360

typedef struct sender_info {
    float timeout;
    float estimated_rtt;
    float dev_rtt;
    int congestion_state;
    int window_base;
    int window_size;
    int ssthresh;
    int packet[SWS];
    struct timeval timer_start;

} sender_info;


enum congestion_state {
    SLOW_START,
    CONGESTION_AVOID,
    FAST_RECOVERY,  
};




