#include "sender_helper.h"
#define ALPHA 0.125
#define BETA  0.25
#define SAFETY_MARGIN 15




// int main(int argc, char* argv[]) {

//     FILE* fd = fopen(argv[1], "w");

//     for(int i = 0; i < 1460; i++ )
//         fprintf(fd,"%d", 1);
//     for(int i = 0; i < 1460; i++ )
//         fprintf(fd,"%d", 2);
//     fclose(fd);


//      int packet_number = read_file(argv[1], 1460*2);

//     for(int i = 0; i < packet_number; i++){
//         size_t len = file_data_array[i].length;
//         for(int j = sender_header_size; j < len; j++)
//             printf("%c", *(file_data_array[i].data + j));
//         printf("\n");
//     }

//     return 0;
// }

int read_file(char* filename, unsigned long long int bytesToTransfer){
    int i, j;
    FILE* fd = fopen(filename, "r");
    if (fd == NULL) {
        printf("ERROR: FILE OPEN FAILED\n");
    }
    
    /*find the file size*/
    fseek(fd, 0, SEEK_END); 
    int file_size = ftell(fd); 
    fseek(fd, 0, SEEK_SET);
    /*find the msg data size & max sequnce number*/
    size_t data_size = bytesToTransfer < (file_size) ? bytesToTransfer  : file_size;
    int packet_num = data_size / msg_body_size;
    if(data_size % msg_body_size)
        packet_num = packet_num + 1;
    /*malloc enough space for file_data array*/
    file_data_array = calloc(packet_num, sizeof(file_data));
    printf("packet number: %d\n", packet_num);
    printf("data size: %d\n", data_size);

    char *file = malloc(data_size);
    fread(file, data_size, 1, fd);
    fclose(fd);

    /*construct the file data array*/
    size_t cur_file_length = 0;
    for(i = 0; i < packet_num; i++ ){
        cur_file_length = msg_body_size;
        if((i == (packet_num - 1)) && (data_size - i*msg_body_size) != 0)
            cur_file_length = data_size - i*msg_body_size;
        char* start_point = file + i*msg_body_size;
        /*create message*/
        printf("cur file length:%d \n", cur_file_length);
        //char hee[10]
        size_t total = cur_file_length + 5;
        char* msg = (char* )malloc(total);
        memset(msg, '\0', cur_file_length + sender_header_size);
        msg[0] = 'M';
        msg[1] = (i % max_seq) / 255; //make sure the number is within one byte
        msg[2] = (i % max_seq) % 255;
        msg[3] = cur_file_length % 1400;
        msg[4] = cur_file_length % 255;
        printf("seq: %d->%d %d->%d\n",(i % max_seq) / 255, (uint8_t)msg[1], (i % max_seq) % 255, (uint8_t)msg[2]);
        for(j = 0; j < cur_file_length; j++ ){
            msg[j + 5] = *(start_point + j);
            //printf("%c", msg[j + 5]);
        }
        
        file_data_array[i].data = msg;
        file_data_array[i].length = sender_header_size + cur_file_length;
        file_data_array[i].status = -1;
        file_data_array[i].seq = i % max_seq;
        file_data_array[i].number = i;

    }
    printf("ds?");

    return packet_num;

}


/* 
Function timeout_interval(): calculate timeout interval for sending packet;
float estimated_rtt: rtt from last timeout_interval call;
float sampled_rtt: measured rtt from time();
*/
float timeout_interval(float sampled_rtt) {
    float estimated_rtt = senderInfo->estimated_rtt;
    float dev_rtt = senderInfo->dev_rtt;
    /*calculate dev_rrt*/
    dev_rtt = (1 - BETA) * (dev_rtt) + BETA * fabsf(sampled_rtt - estimated_rtt);
    /*calculate estimated_rtt*/
    estimated_rtt = (1 - ALPHA) * estimated_rtt + ALPHA * sampled_rtt + 4 * dev_rtt;

    /*update dev_rtt*/
    senderInfo->estimated_rtt = estimated_rtt;
    senderInfo->dev_rtt = dev_rtt;

    return estimated_rtt;
}

int adjust_window_size(int timeout_flag, int duplicate_flag){
    int cur_state = senderInfo->congestion_state;
    int cur_window_size = senderInfo->window_size;
    int cur_ssthresh = senderInfo->ssthresh;

    /*case 1: SLOW_START*/
    if (cur_state == SLOW_START) {
        /*duplicated ack*/
        if(senderInfo->duplicate_ack == 3){
            senderInfo->ssthresh = cur_ssthresh/2;
            senderInfo->window_size = cur_window_size + 3;
            senderInfo->congestion_state = FAST_RECOVERY;
        }
        else if(timeout_flag == 0){
            /*new ack*/
            senderInfo->window_size =  cur_window_size + 1;
            senderInfo->duplicate_ack = 0;
            /*reach ssthresh*/
            if (senderInfo->window_size >= cur_ssthresh)
                senderInfo->congestion_state =  CONGESTION_AVOID;
        }
        else if(timeout_flag == 1){
            /*timeout*/
            senderInfo->window_size =  1;
            senderInfo->ssthresh = cur_ssthresh/2;
            senderInfo->duplicate_ack = 0;
        }
    }

    /*case 2: CONGESTION_AVOID*/
    else if (cur_state == CONGESTION_AVOID) {
        if(timeout_flag == 1){
            /*time out*/
            senderInfo->ssthresh = cur_window_size/2;
            senderInfo->window_size = 1;
            senderInfo->duplicate_ack = 0;
        }
        else if(senderInfo->duplicate_ack != 3){
            /*new ack*/
            senderInfo->ca_extra += 1.0*(1.0/cur_window_size);
            senderInfo->duplicate_ack = 0;
            int temp = senderInfo->ca_extra;
            if(temp >= 1)
                senderInfo->window_size = cur_window_size + temp;
        }
        else if(senderInfo->duplicate_ack == 3){
            /*duplicate case*/
            senderInfo->ssthresh = cur_window_size/2;
            senderInfo->window_size = senderInfo->ssthresh + 3;
            senderInfo->congestion_state = FAST_RECOVERY;
        }
    }

    /*case 3: FAST_RECOVERY*/
    else if (cur_state == FAST_RECOVERY) {
        if(duplicate_flag != 1){
            senderInfo->window_size = senderInfo->ssthresh;
            senderInfo->duplicate_ack = 0;
        }
        else if(duplicate_flag == 1){
            senderInfo->window_size =  cur_window_size + 1;
        }
        else if(timeout_flag == 1){
            senderInfo->ssthresh = cur_window_size/2;
            senderInfo->window_size = 1;
            senderInfo->duplicate_ack = 0;
            senderInfo->congestion_state = SLOW_START;
        }
    }
    
    return 0;
}

int init_sender(){
    senderInfo = malloc(sizeof(sender_info));
    /*init sender structure*/
    senderInfo->timeout = 25.0; /*25ms*/
    senderInfo->estimated_rtt = 0.0;
    senderInfo->dev_rtt = 0.0;
    senderInfo->congestion_state = SLOW_START;
    senderInfo->ssthresh = max_window_size;
    senderInfo->window_packet = NULL;
    senderInfo->window_size = 0;
    senderInfo->timer_start = (struct timeval*) malloc(sizeof(struct timeval));
    senderInfo->last_ack_seq = -1;
    senderInfo->duplicate_ack =  -1;
    senderInfo->ca_extra = 0.0;
    /*sender enter LISTEN state*/
    senderInfo->handshake_state =  LISTEN;
    senderInfo->packet_number = -1;
    return 0;
}



