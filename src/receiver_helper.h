#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/time.h>

#define msg_header_size 3
#define msg_body_size 1460
#define msg_total_size 1463
#define RWS 360

typedef struct receiver_info {
    int state;
    int last_ack;
    int next_expected;
    int recv_window[RWS]; // -1 for not received, 0 for out of order received, 1 for in order ack
    char recv_buffer[RWS][msg_body_size];
    int recv_dataLen[RWS];
    struct timeval timer_start;

}recv_info;

enum recv_state {
    SYN_SENT,
    ESTAB,
    FIN_WAIT1,
    FIN_WAIT2,
    TIME_WAIT,
    CLOSED
};

recv_info *recvInfo;
FILE *fPtr;

void handle_packet(char *packet, int recv_seq);
int int_receiver();