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

static pthread_mutex_t sender_mutex = PTHREAD_MUTEX_INITIALIZER;

void *recieve_ack();

void *reliablySend();

void diep(char *s) {
    perror(s);
    exit(1);
}

void *reliablySend(){
    struct timeval timer_now, timer_diff;
    while(1){
        // pthread_mutex_lock(&sender_mutex);
        //     volatile int sws = senderInfo->window_size;
        //     if(sws == 5){
        //         pthread_mutex_unlock(&sender_mutex);
        //         printf("OK\n");
        //         break;
        //     }
        //     pthread_mutex_unlock(&sender_mutex);

        /*take mutex before accessing the resource*/
        pthread_mutex_lock(&sender_mutex);
        /*take the window size and base*/
        volatile int sws = senderInfo->window_size;
        file_data* base = senderInfo->window_packet;
        int i;
        for(i = 0; i < sws; i++){
            /*case 1: sended and ack just skip*/
            if((base[i].status == 1))
                continue;
            /*case 2: not send yet*/
            else if((base[i].status == -1)){
                if(i == 0){
                    sendto(s, base[0].data, msg_total_size, 0, (struct sockaddr*)&si_other, sizeof(si_other));
                    gettimeofday(senderInfo->timer_start, NULL);
                    base[0].status = 0;
                }
                else{
                    sendto(s, base[i].data, msg_total_size, 0, (struct sockaddr*)&si_other, sizeof(si_other));
                    base[i].status = 0;
                }
            }

            /*case 3:sended not ack yet*/
            else if((base[i].status == 0)){
                if(i == 0){
                    /*check timeout*/
                    gettimeofday(senderInfo->timer_start, NULL);
                    timersub(&timer_now, senderInfo->timer_start, &timer_diff);
                    float sample_rtt = timer_diff.tv_usec / million;
                    if(sample_rtt > senderInfo->timeout){
                        adjust_window_size(1, 0);
                        /*reset index to resend*/
                        base[i].status = -1;
                        i = i - 1;
                        continue;
                    }
                    
                }
                else
                    continue;
            }
            
        }
        /*release lack*/
        pthread_mutex_unlock(&sender_mutex);
    }
    
    return NULL;
}

void *recieve_ack(){
    char recvBuf[100];
    int byte, i;
    struct sockaddr_in si_me;
    int cur_seq;
    struct timeval timer_now, timer_diff;
    while(1){
        // pthread_mutex_lock(&sender_mutex);
        //     if(senderInfo->window_size == 5){
        //         printf("changed break;");
        //         pthread_mutex_unlock(&sender_mutex);
        //         break;
        //     }
        //     senderInfo->window_size = 5;
        //     printf("change window_size\n");
        //     pthread_mutex_unlock(&sender_mutex);
        if ((byte = recvfrom(s, recvBuf, 1400 , 0, (struct sockaddr*)&si_me, sizeof(si_me))) == -1){
            perror("Recieve Failed");
            exit(1);
        }
        /*case recieve an ack*/
        if(recvBuf[0] == 'A'){
            /*calculate the new timeout interval*/
            gettimeofday(&timer_now, NULL);
            /*grab the lock*/
            pthread_mutex_unlock(&sender_mutex);
            timersub(&timer_now, senderInfo->timer_start, &timer_diff);
            float sample_rtt = timer_diff.tv_usec / million;
            senderInfo->timeout = timeout_interval(sample_rtt);

            /*find the sequence numebr*/
            cur_seq = recvBuf[1]*255 + recvBuf[2];
            int expected_seq = (senderInfo->window_packet)->seq;
            /*first ack*/
            if(senderInfo->last_ack_seq == -1)
                senderInfo->last_ack_seq = cur_seq;
            
            /*case 1: sequence number match*/
            if(expected_seq == cur_seq){
                senderInfo->last_ack_seq = cur_seq;
                /*find the coresponding packet for this ack*/
                /*this part can be improved*/            
                for(i = 0; i < senderInfo->window_size; i++){
                    if((senderInfo->window_packet + i)->seq == cur_seq)
                        (senderInfo->window_packet + i)->status = 1;
                }
                /*clear duplicate ACK*/
                senderInfo->duplicate_ack = 0;
                /*adjust window*/
                adjust_window_size(0, 0);
                /*chage window base*/
                senderInfo->window_packet = senderInfo->window_packet + i + 1;
                /*release the lock*/
                pthread_mutex_unlock(&sender_mutex);
                continue;
            }
            
            /*case 2: sequence number greater than expected*/
            if(expected_seq < cur_seq){
                /*change the status of currect ack*/
                for(i = 0; i < senderInfo->window_size; i++){
                    if((senderInfo->window_packet + i)->seq == cur_seq)
                        (senderInfo->window_packet + i)->status = 1;
                }
                /*increment duplicated ack*/
                if(senderInfo->duplicate_ack != -1)
                    senderInfo->duplicate_ack = senderInfo->duplicate_ack + 1;
                else
                    senderInfo->duplicate_ack = 1;
                /*adjust window size*/
                adjust_window_size(0, 1);
                /*release the lock*/
                pthread_mutex_unlock(&sender_mutex);
                continue;
            }

             
            /*case 3: sequence number less than expected*/
            if(expected_seq > cur_seq){
                /*increment duplicated ack*/
                if(senderInfo->duplicate_ack != -1)
                    senderInfo->duplicate_ack = senderInfo->duplicate_ack + 1;
                else
                    senderInfo->duplicate_ack = 1;
                /*adjust window size*/
                adjust_window_size(0, 1);
                /*release the lock*/
                pthread_mutex_unlock(&sender_mutex);
                continue;
            }
        }
        
    }
     return NULL;
}


void *reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer) {
    //Open the file
    // FILE *fp;
    // fp = fopen(filename, "rb");
    // if (fp == NULL) {
    //     printf("Could not open file to send.");
    //     exit(1);
    // }
    // fclose(fp);

	/* Determine how many bytes to transfer */


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

    /*init a mutex lock for sender thread*/
    if(pthread_mutex_init(&sender_mutex, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
    } 
  
    /* Send data and receive acknowledgements on s*/
    read_file(filename, bytesToTransfer);
    init_sender();

    /*send_msg thread for sending packet to reciever*/
    pthread_t send_msg_tid;
	pthread_create(&send_msg_tid, 0, reliablySend, (void*)0);

    /*receive_ack thread for recieve ack from reciever*/
	pthread_t receive_ACK_tid;
	pthread_create(&receive_ACK_tid, 0, recieve_ack, (void*)0);

    /*terminate thread*/
    pthread_join(send_msg_tid, NULL);
    pthread_join(receive_ACK_tid, NULL);

    
	
    // char* test = "weewew";
    // sendto(s, test, 20, 0, (struct sockaddr*)&si_other, sizeof(si_other));
    //printf("-------------------------------------------------------------");

    printf("Closing the socket\n");
    close(s);
     return NULL;
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


