#include "receiver_helper.h"



void write_file(char *buf, int length, FILE* fptr){

    if(fptr){
        fwrite(buf, 1, length, fptr);
        //puts("msg packet write into file OK\n");
    }

}
   

void handle_data(file_data *data, unsigned long long int recv_seq, recv_info* recvInfo, FILE* dest, file_data* file_array, unsigned long long int magic){
    //size_t data_len = 0;
    /*check if recv_seq in window */
    unsigned long long int expected_seq = recvInfo->next_expected;
    size_t i = expected_seq;
    /*check if in the window*/
    /*recv_----->720, expected---->720*/
    if(expected_seq <= recv_seq ){
        if(file_array[recv_seq].status == -10)
            memcpy(&file_array[recv_seq], data, sizeof(file_data));
            
        //int idx = 0;
        //int next_seq = 0;
        while(file_array[i].status != -10){
            write_file(file_array[i].data, file_array[i].length, dest);
            i++;
            if(i == magic)
                break;
        }
        recvInfo->next_expected = i;
    }
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
