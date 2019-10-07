#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "receiver_helper.h"



void write_file(char *buf, int length, FILE* fptr){

    if(fptr){
        fwrite(&buf, length, 1, fptr);
        puts("Packet write into file\n");
    }

}
   

void handle_packet(char *packet){

    int window_idx, recv_seq, i; // packet sequence number 
    size_t data_len;
    /* need data length, sequence number in packet header */
    // data_len = ??
    // recv_seq = packet[??];
    window_idx = recv_seq % RWS;

    /* copy packet data to receiver buffer, mark 1 for packet in recv_window */
    if(recv_seq - recvInfo->next_expected <= RWS){
        memcpy(recvInfo->recv_buffer[window_idx], packet+msg_header_size, data_len);
        recvInfo->recv_dataLen[window_idx] = data_len;
        recvInfo->recv_window[window_idx] = 1;
    }

    /* loop through window, write in order data to file */
    for(i = 0; i < RWS; i++){
        window_idx = recvInfo->next_expected%RWS+i;
        if(recvInfo->recv_window[window_idx]){
            write_file(recvInfo->recv_buffer[window_idx], recvInfo->recv_dataLen[window_idx], fPtr); 
            recvInfo->recv_window[window_idx] = 0;
        }else
            break;
        
    }
    /* update expected seq number */
    recvInfo->next_expected += i;

}

   
/*init receiver structure*/
int int_receiver(){
    recvInfo = malloc(sizeof(recv_info));
    recvInfo->state = CLOSED;
    recvInfo->last_ack = -1;
    recvInfo->next_expected = 0;

    return 0;
}
