/* 
 * File:   receiver_main.c
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
#include "receiver_helper.h"




struct sockaddr_in si_me, si_other;
int s ;
socklen_t slen;
//static int fptr;

void diep(char *s) {
    perror(s);
    exit(1);
}

FILE* create_file(char *fileName){
    FILE* fPtr = fopen(fileName, "wb"); //FILE NAME OUTPUT FOR TESTING
	if (!fPtr )
		printf("create file failed");
    
	printf("file create OK\n");
    return fPtr;
}

void recv_packet(FILE* dest, recv_info* recvInfo){
    char ACK[3]; // ACK msg for sender
    int recvBytes, sentBytes;
    char recvBuffer[msg_total_size];
    // int recv_seq = 0; // FOR TESTING

    while(1){

        memset(recvBuffer, 0, msg_total_size); // clean buffer needed 
        if ((recvBytes = recvfrom(s, recvBuffer, msg_total_size , 0, (struct sockaddr*)&si_other, &slen)) == -1) {
            perror("receiver recv_packet failed\n");
            exit(1);
        }

        /* LISTEN state, synthesis with sender */
        if (recvInfo->handshake_state == LISTEN){
            /*Wait for the SYN from Sender*/
            if(recvBuffer[0] == 'S'){
                 /*generate ACK*/
                ACK[0] = 'S';
                ACK[1] = 'S';
                ACK[2] = 'S';
                sentBytes = sendto(s, ACK, 3, 0, (struct sockaddr*)&si_other, slen);
                recvInfo->handshake_state = ESTAB;
            }            
        }

        /* ESTAB state, start writing file */
        else if (recvInfo->handshake_state == ESTAB) {
        
            /*check if its SYN */
            if (recvBuffer[0] == 'S'){
                recvInfo->handshake_state = LISTEN;
                continue;
            }

            /*check if its FINbit */
            if (recvBuffer[0] == 'F' && recvBuffer[1] == 'F' && recvBuffer[2] == 'F'){
                recvInfo->handshake_state = CLOSED_WAIT;
                /*generate ACK*/
                ACK[0] = 'F';
                ACK[1] = 'F';
                ACK[2] = 'F';
                sentBytes = sendto(s, ACK, 3, 0, (struct sockaddr*)&si_other, slen);
                continue;
            }

            /*check if its data packet*/
            if(recvBuffer[0] == 'M'){
                /*get sequnce number*/
                int cur_seq = recvBuffer[1]*255 + recvBuffer[2];
                /*get length*/
                int length = recvBuffer[3]*1400 + recvBuffer[4];
                ///// FOR TESTING /////
                length = recvBytes - msg_header_size; //
                //////////////////////
                printf("recieve bytes : %d\n", recvBytes);
                printf("length: %d\n",length);
                printf("data: %s\n",recvBuffer + msg_header_size);
                handle_data(recvBuffer + msg_header_size, cur_seq, recvInfo, dest, length);
                /*generate ACK*/
                ACK[0] = 'A';
                ACK[1] = recvBuffer[1];
                ACK[2] = recvBuffer[2];
                sentBytes = sendto(s, ACK, 3, 0, (struct sockaddr*)&si_other, slen);
                printf("sendto finshed\n");
                
            }
        }

        /* CLOSED WAIT state, check if sender get FINACK */
        else if (recvInfo->handshake_state == CLOSED_WAIT){
            /*generate ACK*/
            ACK[0] = 'F';
            ACK[1] = 'F';
            ACK[2] = 'F';
            if ((sentBytes = sendto(s, ACK, 3, 0, (struct sockaddr*)&si_other, slen))==-1){
                recvInfo->handshake_state = CLOSED;
                break;
            }
            
            usleep(25*1000); //sleep for 25ms, not spam bandwidth
        }

        //printf("receive packet OK\n");
        /* Read header and response */
        // if((SYNACK(seq=y, ACKnum=x+1))&(recvInfo->state == SYN_SENT)){
        //     sendto(ACK(ACKnum=y+1));
        //     recvInfo->state = ESTAB;
        // }

        // if(recvInfo->state == ESTAB){
            // int recv_seq = recvBuffer[1]*255 + recvBuffer[2];
        /*generate ACK*/

        //printf("handle msg packet OK\n\n\n");

        // printf("send ACK packet OK\n");
        // }

        // else if(){
        //     recvInfo->state = FIN_WAIT2
        // }   
        // else if(){
        //     recvInfo->state = TIME_WAIT
        //     timer 30s
        //     recvInfo->state = CLOSED
        // }           

    }
}


void reliablyReceive(unsigned short int myUDPport, char* destinationFile) {
    
    slen = sizeof (si_other);


    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_me, 0, sizeof (si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(myUDPport);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Now binding\n");
    if (bind(s, (struct sockaddr*) &si_me, sizeof (si_me)) == -1)
        diep("bind");
 
	/* Create recv_output file */
    FILE* dest = create_file(destinationFile);
    /*init reciever*/
    recv_info* recvInfo = int_receiver();
    /*receieve enter LISTEN state*/
    recvInfo->handshake_state  = LISTEN;
    /*start recieve data*/
    recv_packet(dest, recvInfo);


    
	/* Now receive data and send acknowledgements */   
    //pthread_t recv_tid;
    //pthread_create(&recv_tid, 0, recv_packet, (void *)0);
    //pthread_join(recv_tid, NULL);
    //printf("Thread finished\n");
    // pthread_join(time_tid, NULL);



    close(s);
    fclose(dest);
	printf("%s received.", destinationFile);
    return;
}

/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;

    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);

    reliablyReceive(udpPort, argv[2]);
}

