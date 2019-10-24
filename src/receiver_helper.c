#include "receiver_helper.h"



void write_file(char *buf, int length, FILE* fptr){

    if(fptr){
        fwrite(buf, 1, length, fptr);
        //puts("msg packet write into file OK\n");
    }

}
   

void handle_data(char *data, int recv_seq, recv_info* recvInfo, FILE* dest, int length){
    size_t data_len, i;
    /*check if recv_seq in window */
    int expected_seq = recvInfo->next_expected;
    /*check if in the window*/
    /*recv_----->720, expected---->720*/
    //if(((0 <= (recv_seq - expected_seq)) && ((recv_seq - expected_seq) < RWS)) ||
            //(recv_seq + MAX_SEQ_NUM - expected_seq < RWS)){
        int window_idx = recv_seq % RWS;
        //printf("window_idx = %d\n", window_idx);
        /*check duplicate*/
        if(recvInfo->recv_window[window_idx] != 1){
            /* copy packet data to receiver buffer, mark 1 for packet in recv_window */
            memcpy(recvInfo->recv_buffer[window_idx], data, length); // MSG_BODY_SIZE OF TESTING, SHOULD BE DATA_LEN
            recvInfo->recv_dataLen[window_idx] = length; // MSG_BODY_SIZE OF TESTING, SHOULD BE DATA_LEN
            recvInfo->recv_window[window_idx] = 1;
            //sprintf("recvInfo->recv_buffer[%d]: %s\n",window_idx, recvInfo->recv_buffer[window_idx]);
            int idx = 0;
            for(i = 0; i < RWS; i++){
                idx = (expected_seq + i) % RWS;
                if(recvInfo->recv_window[idx]){
                //printf("recvInfo->recv_buffer[%d]: %s\n",window_idx, recvInfo->recv_buffer[window_idx]);
                    write_file(recvInfo->recv_buffer[idx], recvInfo->recv_dataLen[idx], dest); 
                    recvInfo->recv_window[idx] = 0;
                }
                else
                break;
            }
            recvInfo->next_expected = idx % MAX_SEQ_NUM;
        }

        //recvInfo->next_expected = idx % MAX_SEQ_NUM;
        //printf("expected number: %d\n", recvInfo->next_expected);
    //}
    return;

}

   
/*init receiver structure*/
recv_info* int_receiver(){
    recv_info* recvInfo = malloc(sizeof(recv_info));
    recvInfo->state = CLOSED; // FOR TESTING
    recvInfo->last_ack = -1;
    recvInfo->next_expected = 0;
    recvInfo->handshake_state  = -1;
    memset(recvInfo->recv_window, 0, RWS);
    //printf("receiver init OK\n");

    return recvInfo;
}
