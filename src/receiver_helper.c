#include "receiver_helper.h"



void write_file(char *buf, int length, FILE* fptr){

    if(fptr){
        printf("writing length: %d\n", length);
        fwrite(buf, length, 1, fptr);
        // puts("msg packet write into file OK\n");
    }

}
   

void handle_data(char *data, int recv_seq, recv_info* recvInfo, FILE* dest, int length){
    size_t data_len, i;

    printf("////IN HANDLE_DATA FUNCTION////\n");
    /*check if recv_seq in window */
    int expected_seq = recvInfo->next_expected;
    printf("recv_seq: %d\n exp_seq: %d\n",recv_seq,expected_seq);
    /*check if in the window*/
    /*recv_----->720, expected---->720*/
    if(((0 <= (recv_seq - expected_seq)) && ((recv_seq - expected_seq) < RWS)) ||
            (recv_seq + MAX_SEQ_NUM - expected_seq < RWS)){
        printf("in window check OK\n");
        int window_idx = expected_seq % RWS;
        /*check duplicate*/
        if(recvInfo->recv_window[window_idx] != 1){
            /* copy packet data to receiver buffer, mark 1 for packet in recv_window */
            memcpy(recvInfo->recv_buffer[window_idx], data, length); // MSG_BODY_SIZE OF TESTING, SHOULD BE DATA_LEN
            recvInfo->recv_dataLen[window_idx] = length; // MSG_BODY_SIZE OF TESTING, SHOULD BE DATA_LEN
            recvInfo->recv_window[window_idx] = 1;
            printf("marking recvInfo->recv_buffer[%d]: %s OK\n",window_idx, recvInfo->recv_buffer[window_idx]);
        }
        for(i = 0; i < RWS; i++){
            window_idx = (window_idx+i)%RWS;
            if(recvInfo->recv_window[window_idx]){
            printf("writing recvInfo->recv_buffer[%d]: %s OK\n",window_idx, recvInfo->recv_buffer[window_idx]);
            write_file(recvInfo->recv_buffer[window_idx], recvInfo->recv_dataLen[window_idx], dest); 
            recvInfo->recv_window[window_idx] = 0;
            }
            else
                break;
        }
        recvInfo->next_expected = (recvInfo->next_expected + i)% MAX_SEQ_NUM;
    }
    
    printf("////END HANDLE_DATA FUNCTION////\n");
    return;

}

   
/*init receiver structure*/
recv_info* int_receiver(){
    recv_info* recvInfo = malloc(sizeof(recv_info));
    // recvInfo->state = LISTEN; // FOR TESTING
    recvInfo->last_ack = -1;
    recvInfo->next_expected = 0;
    recvInfo->handshake_state  = LISTEN;
    memset(recvInfo->recv_window, 0, RWS);
    printf("receiver init OK\n");

    return recvInfo;
}
