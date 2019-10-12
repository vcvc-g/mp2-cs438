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

#define msg_header_size 5
#define msg_body_size 1460
#define msg_total_size 1463
#define MAX_SEQ_NUM 720
#define RWS 360

typedef struct receiver_info {
    int state;
    int last_ack;// last seq
    int next_expected;//next seq
    int recv_window[RWS]; // -1 for not received, 0 for out of order received, 1 for in order ack
    char recv_buffer[RWS][msg_body_size];
    int recv_dataLen[RWS];
    int  handshake_state;
    struct timeval timer_start;

}recv_info;


//012345
//[012]-> [123]
enum recv_state {
    LISTEN,
    ESTAB,
    CLOSED_WAIT,
    CLOSED
};

//recv_info *recvInfo;
//FILE *fPtr;

void handle_data(char *packet, int recv_seq, recv_info* recvInfo, FILE* dest, int length);
void recv_packet(FILE* dest, recv_info* recvInfo);
recv_info* int_receiver();

