#include "sender_helper.h"
#define ALPHA 0.125
#define BETA  0.25
#define SAFETY_MARGIN 15

int read_file(char* filename, unsigned long long int bytesToTransfer){
    unsigned long long int i, j;
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
    //printf("packet number: %d\n", packet_num);
    //printf("data size: %d\n", data_size);

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
        //printf("cur file length:%d \n", cur_file_length);
        //char hee[10]
        file_data_array[i].length = cur_file_length;

        for(j = 0; j < cur_file_length; j++ ){
           *(file_data_array[i].data + j) = *(start_point + j);
        }
        file_data_array[i].type[0] = 'M';
        file_data_array[i].type[1] = 'M';
        file_data_array[i].type[2] = 'M';
        file_data_array[i].status = -1;
        file_data_array[i].number = i;
    }
    //printf("packet_number: %d\n", packet_num);

    return packet_num;

}


/* 
Function timeout_interval(): calculate timeout interval for sending packet;
float estimated_rtt: rtt from last timeout_interval call;
float sampled_rtt: measured rtt from time();
*/
long double timeout_interval(unsigned long long int idx) {


    double  estimated_rtt = senderInfo->estimated_rtt;
    double  dev_rtt = senderInfo->dev_rtt;
    double result;
    struct timeval timer_now, timer_diff;

    gettimeofday(&timer_now, NULL);
            /*grab the lock*/
            //pthread_mutex_lock(&sender_mutex);
    timersub(&timer_now, &file_data_array[idx].timer, &timer_diff);
    double sampled_rtt = (double)(timer_diff.tv_usec / million);
            //printf("sameple rtt: %f\n", sample_rtt );
            //senderInfo->timeout = timeout_interval(sample_rtt);

    /*calculate dev_rrt*/
    //dev_rtt = (1 - BETA) * (dev_rtt) + BETA * fabsf(sampled_rtt - estimated_rtt);
    /*calculate estimated_rtt*/
    estimated_rtt = (1 - ALPHA) * estimated_rtt + ALPHA * sampled_rtt;
    // if(estimated_rtt > 100.00){
    //     estimated_rtt = 25.00;
    //     senderInfo->estimated_rtt = estimated_rtt;
    // }



    dev_rtt = (1 - BETA) * (dev_rtt) + BETA * fabsf(sampled_rtt - estimated_rtt);
    // if(estimated_rtt > 100.00){
    //     dev_rtt = 2.00;
    //     senderInfo->dev_rtt = dev_rtt;
    // }
    senderInfo->dev_rtt = dev_rtt;
    /*update dev_rtt*/
    //senderInfo->estimated_rtt = estimated_rtt;
    result = 1.5* estimated_rtt + 4*dev_rtt;
    
   
    printf("----------------->%lf\n",result );
    //senderInfo->dev_rtt = dev_rtt;

    return result;
}

int adjust_window_size(int timeout_flag, int duplicate_flag){
    int cur_state = senderInfo->congestion_state;
    int cur_window_size = senderInfo->window_size;
    int cur_ssthresh = senderInfo->ssthresh;

    /*case 1: SLOW_START*/
    if (cur_state == SLOW_START) {
        /*duplicated ack*/
        if(senderInfo->duplicate_ack == 3){
            senderInfo->ssthresh = cur_ssthresh/2 + 3;
            senderInfo->window_size = cur_window_size + 5;
            senderInfo->congestion_state = FAST_RECOVERY;
        }
        else if(timeout_flag == 0){
            /*new ack*/
            senderInfo->window_size =  cur_window_size*8;
            senderInfo->duplicate_ack = 0;
            /*reach ssthresh*/
            if (senderInfo->window_size >= cur_ssthresh)
                senderInfo->congestion_state =  CONGESTION_AVOID;
        }
        else if(timeout_flag == 1){
            /*timeout*/
            senderInfo->window_size =  1;
            senderInfo->ssthresh = cur_window_size/2 + 3;
            senderInfo->duplicate_ack = 0;
        }
    }

    /*case 2: CONGESTION_AVOID*/
    else if (cur_state == CONGESTION_AVOID) {
        if(timeout_flag == 1){
            /*time out*/
            senderInfo->ssthresh = cur_window_size/2 + 3;
            senderInfo->window_size = 1;
            senderInfo->duplicate_ack = 0;
            senderInfo->congestion_state =  SLOW_START;
        }
        else if(senderInfo->duplicate_ack != 3){
            /*new ack*/
            senderInfo->ca_extra += 
            senderInfo->duplicate_ack = 0;
            senderInfo->window_size = cur_window_size + 2;
            // int temp = senderInfo->ca_extra;
            // if(temp >= 1)
            //     senderInfo->window_size = cur_window_size + temp;
        }
        else if(senderInfo->duplicate_ack == 3){
            /*duplicate case*/
            senderInfo->ssthresh = cur_window_size/2 + 3;
            senderInfo->window_size = senderInfo->ssthresh /2  + 3;
            senderInfo->congestion_state = FAST_RECOVERY;
        }
    }

    /*case 3: FAST_RECOVERY*/
    else if (cur_state == FAST_RECOVERY) {
        if(duplicate_flag != 1){
            senderInfo->window_size = senderInfo->ssthresh/2;
            senderInfo->duplicate_ack = 0;
            senderInfo->congestion_state = CONGESTION_AVOID;
        }
        else if(duplicate_flag == 1){
            senderInfo->window_size =  cur_window_size + 1;
        }
        else if(timeout_flag == 1){
            senderInfo->ssthresh = cur_window_size/2 + 3;
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
    senderInfo->timeout = 30.0; /*25ms*/
    senderInfo->estimated_rtt = 0.0;
    senderInfo->dev_rtt = 0.0;
    senderInfo->congestion_state = SLOW_START;
    senderInfo->ssthresh = 2000;
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



