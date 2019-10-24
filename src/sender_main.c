/* 
 * File:   sender_main.c
 * Author: 
 *
 * Created on 
 */

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
#include "sender_helper.h"

struct sockaddr_in si_other;
int s, slen;

//static pthread_mutex_t sender_mutex = PTHREAD_MUTEX_INITIALIZER;

//void *recieve_ack();

void reliablySend();

void *reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer);

void diep(char *s) {
    perror(s);
    exit(1);
}

void reliablySend(){
    struct timeval timer_now, timer_diff;
    char ACK[msg_total_size];
    memset(ACK, 'K', msg_total_size);

    while(1){
        volatile int state = senderInfo->handshake_state;
        if(state == LISTEN){
            ACK[0] = 'S';
            ACK[1] = 'S';
            ACK[2] = 'S';
            sendto(s, ACK, msg_total_size, 0, (struct sockaddr*)&si_other, sizeof(si_other));
            /*change state to SYNSENT*/
            senderInfo->handshake_state = SYNSENT;
            /*start timer*/
            gettimeofday(senderInfo->timer_start, NULL);
        }
        else if(senderInfo->handshake_state == SYNSENT){
            /*check timeout*/
            gettimeofday(&timer_now, NULL);
            //printf("Waiting SYN \n");
            timersub(&timer_now, senderInfo->timer_start, &timer_diff);
            //printf("time out %f \n", senderInfo->timeout);
 
            double sample_rtt = timer_diff.tv_usec / million;
            //printf("rtt %f \n", sample_rtt );
            if(sample_rtt > senderInfo->timeout){
                senderInfo->handshake_state = LISTEN;
                continue;
            }
        }
        else if(senderInfo->handshake_state == CLOSE_WAIT){
            //printf("there you go ending state");
            gettimeofday(&timer_now, NULL);
            timersub(&timer_now, senderInfo->timer_start, &timer_diff);
            float sample_rtt = timer_diff.tv_usec / million;
            if(sample_rtt > senderInfo->timeout){
                ACK[0] = 'F';
                ACK[1] = 'F';
                ACK[2] = 'F';
                sendto(s, ACK, msg_total_size, 0, (struct sockaddr*)&si_other, sizeof(si_other));
                gettimeofday(senderInfo->timer_start, NULL);
            }
        }
        else if(senderInfo->handshake_state == CLOSED)
            return;
        else if(senderInfo->handshake_state == ESTAB){
            /*take the window size and base*/
            int sws = senderInfo->window_size;
            file_data* base = senderInfo->window_packet;
            int i;
            //printf("sws : %d\n" , sws);
            // if (sws == 1)
            for(i = 0; i < sws; i++){
                /* case 1: sended and ack just skip */
                if(base[i].status == 1){
                    continue;
                }
                /* case 2: not send yet */
                else if(base[i].status == -1){
                    if(i == 0){
                        //printf("try to send message with sws : %d i :%d\n", sws, i);
                        sendto(s, base[0].data, msg_total_size, 0, (struct sockaddr*)&si_other, sizeof(si_other));
                        gettimeofday(senderInfo->timer_start, NULL);
                        //printf("send message with sws : %d i :%d\n", sws, base[i].seq);
                        base[0].status = 0;
                    }
                    else{
                        //printf("try to send message with sws : %d i :%d\n", sws, base[i].seq);
                        sendto(s, base[i].data, msg_total_size, 0, (struct sockaddr*)&si_other, sizeof(si_other));
                        base[i].status = 0;
                    }
                }
                /* case 3:sended not ack yet */
                else if(base[i].status == 0){
                    if(i == 0){                  
                        gettimeofday(&timer_now, NULL);
                        timersub(&timer_now, senderInfo->timer_start, &timer_diff);
                        float sample_rtt = timer_diff.tv_usec / million;
                        if(sample_rtt > senderInfo->timeout){
                            adjust_window_size(1, 0);
                            /*reset index to resend*/
                            //printf("try to resend %d  message rtt: %f timeout: %f\n", base[i].seq, sample_rtt, senderInfo->timeout);
                            base[i].status = -1;
                            i = i - 1;
                        }
                    }
                    //printf("not ack %d\n", i);
                }
            }
            
        }

        /******************************recieve_ack*********************************************************/
        int byte, i;
        char recvBuf[msg_total_size];
        /*waiting SYN from receiver*/
        memset(recvBuf, 'L', 100); // clean buffer needed 
        //printf("recving ack running\n");
        if ((byte = recvfrom(s, recvBuf, msg_total_size, MSG_DONTWAIT, (struct sockaddr*)&si_other, &slen) == -1)){
            /*no recieve anything*/
            
            //printf("sddddddddddddddddddddddddddddddddddddddddddd");
            
        }

        if (senderInfo->handshake_state == CLOSE_WAIT){
            if(recvBuf[0] == 'F'){
                senderInfo->handshake_state = CLOSED;
                return;
            }
        }
        /*case reccieve SYN from reciever*/
        if(recvBuf[0] == 'S'){
            //printf("recieve SYN bit\n");
            gettimeofday(&timer_now, NULL);
            /*case when if sender just timeout but receive*/
            if(senderInfo->handshake_state != SYNSENT){
                //printf("we are in the wrong stare?");
                continue;
            }
           // printf("we are in the correct state\n");
            /*enter ESTAB state and calcualte the timeout value*/
            timersub(&timer_now, senderInfo->timer_start, &timer_diff);
            float sample_rtt = timer_diff.tv_usec / million;
            //senderInfo->timeout = timeout_interval(sample_rtt);
            senderInfo->handshake_state = ESTAB;

            /*prepare to send the first byte*/
            senderInfo->window_size = 1;
            senderInfo->window_packet = file_data_array;

            /*release mutex lock*/
            continue;
        }

        int sws = senderInfo->window_size;
        /*case recieve an ack*/
        if(recvBuf[0] == 'A'){
            /*calculate the new timeout interval*/
            gettimeofday(&timer_now, NULL);
            /*grab the lock*/
            //pthread_mutex_lock(&sender_mutex);
            timersub(&timer_now, senderInfo->timer_start, &timer_diff);
            float sample_rtt = timer_diff.tv_usec / million;
            //senderInfo->timeout = timeout_interval(sample_rtt);

            /*find the sequence numebr*/
            int cur_seq = ((uint8_t)recvBuf[1])*255 + (uint8_t) recvBuf[2];
            //printf("I recieve an ack seq: %d\n", cur_seq );
            int expected_seq = (senderInfo->window_packet)->seq;
            //printf("I recieve an ack seq: %d: %d \n", cur_seq, expected_seq );
            /*first ack*/
            if(senderInfo->last_ack_seq == -1)
                senderInfo->last_ack_seq = cur_seq;

            /*case 1: sequence number match*/
            if(expected_seq == cur_seq){
                
                senderInfo->last_ack_seq = cur_seq;
                /*find the coresponding packet for this ack*/
                /*this part can be improved*/            
                for(i = 0; i < (max_seq/2) ; i++){
                    if((senderInfo->window_packet + i)->seq == cur_seq)
                        (senderInfo->window_packet + i)->status = 1;                  
                }

                int count = 0;
                for(i = 0; i < (senderInfo->packet_number); i++){
                    //printf("1seq: %d status: %d\n", (senderInfo->window_packet+i)->seq, (senderInfo->window_packet+i)->status);
                    //printf("2seq: %d status: %d\n", (senderInfo->window_packet+i + 1)->seq, (senderInfo->window_packet + i + 1)->status);
                    //printf("2seq: %d status: %d\n", (senderInfo->window_packet+i + 2)->seq, (senderInfo->window_packet + i + 2)->status);
                    if((senderInfo->window_packet + i) ->status != 1){
                        //printf("%d i\n", i);
                        break;
                        //i = (senderInfo->packet_number);

                    }
                    count = count + 1;
                    if((senderInfo->window_packet + i)->number == (senderInfo->packet_number - 1)) break;
                    //printf("NUMBER :%d \n", (senderInfo->window_packet + i)->number);
                    //printf("------>%d ", count);            
                }
                //printf("------> i:%d, number : %d total : %d\n ", i,(senderInfo->window_packet + i - 1)->number, (senderInfo->packet_number - 1));  
                /*check if we reach the end*/
                if((senderInfo->window_packet + count - 1)->number == (senderInfo->packet_number - 1)){     
                    senderInfo->handshake_state = CLOSE_WAIT;
                    //printf("?>>>>>>>\n");
                    ACK[0] = 'F';
                    ACK[1] = 'F';
                    ACK[2] = 'F';
                    sendto(s, ACK, msg_total_size, 0, (struct sockaddr*)&si_other, sizeof(si_other));
                    gettimeofday(senderInfo->timer_start, NULL);
                    continue;
                }
                /*clear duplicate ACK*/
                senderInfo->duplicate_ack = 0;
                /*adjust window*/
                adjust_window_size(0, 0);
                /*chage window base*/
                /*reamining packet*/
                int reamining = (senderInfo->packet_number) - (senderInfo->window_packet + count) -> number;
                if(senderInfo->window_size > reamining)
                    senderInfo->window_size = reamining;
                if(senderInfo->window_size > 360)
                    senderInfo->window_size = 360;
                senderInfo->window_packet = senderInfo->window_packet + count;
                /*release the lock*/
                //printf("expected eq----: %d \n", senderInfo->window_packet->seq);
                continue;
            }
            
            /*case 2: sequence number greater than expected*/
            if(expected_seq < cur_seq || cur_seq < (sws - (max_seq - expected_seq))){
                //printf("I face a smaller case %d : %d\n", expected_seq, cur_seq);
                /*change the status of currect ack*/
                for(i = 0; i < (max_seq/2); i++){
                    if((senderInfo->window_packet + i)->seq == cur_seq){
                        (senderInfo->window_packet + i)->status = 1;
                    }
                }
                /*increment duplicated ack*/
                //if(senderInfo->duplicate_ack != -1)
                  //  senderInfo->duplicate_ack = senderInfo->duplicate_ack + 1;
                //else
                  //  senderInfo->duplicate_ack = 1;
                /*adjust window size*/
                //adjust_window_size(0, 1);
                continue;
            }     
            /*case 3: sequence number less than expected*/
            else if(expected_seq > cur_seq){
                 //      printf("I am now bigger\n");
                /*increment duplicated ack*/
                if(senderInfo->duplicate_ack != -1)
                    senderInfo->duplicate_ack = senderInfo->duplicate_ack + 1;
                else
                    senderInfo->duplicate_ack = 1;
                /*adjust window size*/
                adjust_window_size(0, 1);
                /*release the lock*/
                continue;
            }
        }
        
    }
    return ;
}

void *reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer) {

    /* Setup UDP connection */
    slen = sizeof (si_other);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_other, 0, sizeof (si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(hostUDPport);
    if (inet_aton(hostname, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    struct timeval timer_now, timer_diff, timer_start;
    gettimeofday(&timer_start, NULL);


    int number = read_file(filename, bytesToTransfer);
    init_sender();
    senderInfo->packet_number = number;
    
    /*sender enter LISTEN state*/
    senderInfo->handshake_state = LISTEN;

    reliablySend();

    gettimeofday(&timer_now, NULL);

    timersub(&timer_now, &timer_start, &timer_diff);

    close(s);
    return 0;
}


/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;
    unsigned long long int numBytes;

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    udpPort = (unsigned short int) atoi(argv[2]);
    numBytes = atoll(argv[4]);

    reliablyTransfer(argv[1], udpPort, argv[3], numBytes);

    return 0;
}


