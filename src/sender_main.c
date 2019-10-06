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
    while(1){
        pthread_mutex_lock(&sender_mutex);
            volatile int sws = senderInfo->window_size;
            if(sws == 5){
                pthread_mutex_unlock(&sender_mutex);
                printf("OK\n");
                break;
            }
            pthread_mutex_unlock(&sender_mutex);
    //     int SWS = sender->window_size;
    //     int base = sender->window_base;
    //     for(int i =0; i < SWS; i++){
    //         if(i == 0){
    //             if(sender->packet[base+i] == 0){
    //                 snedto();
    //                 sneder->packet[base+i] = 1;
    //                 gettimeofday(&(sender->timer_start), NULL);
    //             }

    //         } else{
    //             if(sender->packet[base+i] == 0){
    //                 sendto();
    //                 sneder->packet[base+i] = 1;

    //             } else{
    //                 continue;
    //             }
    //         }
    //     }
    }
}

void *recieve_ack(){
    while(1){
        pthread_mutex_lock(&sender_mutex);
            if(senderInfo->window_size == 5){
                printf("changed break;");
                pthread_mutex_unlock(&sender_mutex);
                break;
            }
            senderInfo->window_size = 5;
            printf("change window_size\n");
            pthread_mutex_unlock(&sender_mutex);
    }
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
    // slen = sizeof (si_other);

    // if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    //     diep("socket");

    // memset((char *) &si_other, 0, sizeof (si_other));
    // si_other.sin_family = AF_INET;
    // si_other.sin_port = htons(hostUDPport);
    // if (inet_aton(hostname, &si_other.sin_addr) == 0) {
    //     fprintf(stderr, "inet_aton() failed\n");
    //     exit(1);
    // }

     if (pthread_mutex_init(&sender_mutex, NULL) != 0) 
    { 
        printf("\n mutex init has failed\n"); 
    
    } 
  

    read_file(filename, bytesToTransfer);
    int_sender();

    pthread_t send_msg_tid;
	pthread_create(&send_msg_tid, 0, reliablySend, (void*)0);

	pthread_t receive_ACK_tid;
	pthread_create(&receive_ACK_tid, 0, recieve_ack, (void*)0);

    pthread_join(send_msg_tid, NULL);
    pthread_join(receive_ACK_tid, NULL);

    //printf("------------------------------------------------1-------------");
    
	/* Send data and receive acknowledgements on s*/
    char* test = "weewew";
    sendto(s, test, 20, 0, (struct sockaddr*)&si_other, sizeof(si_other));
    //printf("-------------------------------------------------------------");

    printf("Closing the socket\n");
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


