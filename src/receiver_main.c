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
#include <fcntl.h>
#include "receiver_helper.h"




struct sockaddr_in si_me, si_other;
int s ;
socklen_t slen;
//static int fptr;

int reliablyReceive(unsigned short int myUDPport, char* destinationFile);

void diep(char *s) {
    perror(s);
    exit(1);
}

FILE* create_file(char *fileName){
    FILE* fPtr = fopen(fileName, "wb"); //FILE NAME OUTPUT FOR TESTING
	if (!fPtr )
		//printf("create file failed");
    
	//printf("file create OK\n");
    return fPtr;
}

void recv_packet(FILE* dest, recv_info* recvInfo){
    unsigned long long int magic = 1000;
    file_data* file_data_array = calloc(magic, sizeof(file_data));
    for(int i = 0; i < magic; i++)
        file_data_array[i].status = -10;
    file_data *recieve_data = malloc(sizeof(file_data));
    ACK to_sender_ACK;
    int msg_total_size = sizeof(file_data);
    int sentBytes = 0;
    int recvBytes = 0;

    while(1){
        //printf("start buffer: %s\n", recvBuffer);
        //sleep(1);
        //printf("recv is running now\n");
         /* CLOSED WAIT state, check if sender get FINACK */
         recieve_data->type[0] = 'L';
         recieve_data->type[1] = 'L';
         recieve_data->type[2] = 'L';
        if (recvInfo->handshake_state == CLOSED_WAIT){
            /*generate ACK*/
            to_sender_ACK.type[0] = 'F';
            to_sender_ACK.type[1] = 'F';
            to_sender_ACK.type[2] = 'F';

            if ((sentBytes = sendto(s, &to_sender_ACK, msg_total_size, 0, (struct sockaddr*)&si_other, slen)) == -1){
                recvInfo->handshake_state = CLOSED;
                break;
            }
            
            usleep(25*1000); //sleep for 25ms, not spam bandwidt
            break;
        }

        if ((recvBytes = recvfrom(s, recieve_data, msg_total_size , 0, (struct sockaddr*)&si_other, &slen)) == -1) {
            perror("receiver recv_packet failed\n");
            exit(1);
            //printf("?");
            //continue;
        }
        //printf("recv data: %c\n", recieve_data->type[0]);
        //if(recvBytes != 0)
            // /printf("message: %s\n", recvBuffer);
        /* LISTEN state, synthesis with sender */
        if (recvInfo->handshake_state == LISTEN){
            /*Wait for the SYN from Sender*/
            if(recieve_data->type[0] == 'S'){
                /*generate ACK*/
                to_sender_ACK.type[0] = 'S';
                to_sender_ACK.type[1] = 'S';
                to_sender_ACK.type[2] = 'S';
                sentBytes = sendto(s, &to_sender_ACK, msg_total_size, 0, (struct sockaddr*)&si_other, slen);
                recvInfo->handshake_state = ESTAB;
                to_sender_ACK.type[0] = 'L';
                to_sender_ACK.type[1] = 'L';
                to_sender_ACK.type[2] = 'L';
                continue;
            }
        }
        /* ESTAB state, start writing file */
        else if (recvInfo->handshake_state == ESTAB) {
            /*check if its SYN */
            //printf("after buffer: %s\n", recvBuffer); 
            if (to_sender_ACK.type[0] == 'S'){
                //printf("go back to listen\n");
                recvInfo->handshake_state = LISTEN;
                continue;
            }

            /*check if its FINbit */
            if (recieve_data->type[0] == 'F'){
                recvInfo->handshake_state = CLOSED_WAIT;
                /*generate ACK*/
                //printf("Sending FIN bit\n");
                to_sender_ACK.type[0] = 'F';
                to_sender_ACK.type[1] = 'F';
                to_sender_ACK.type[2] = 'F';
                sentBytes = sendto(s, &to_sender_ACK, msg_total_size, 0, (struct sockaddr*)&si_other, slen);
                continue;
            }

            /*check if its data packet*/
            if(recieve_data->type[0] == 'M'){
                //printf("hello?\n");
                /*get sequnce number*/
                unsigned long long int cur_seq = recieve_data->number;
                //xwwprintf("cur_seq : %llu\n", cur_seq);
                if(cur_seq >= magic){
                    unsigned long long int old_magic = magic;
                    magic = magic + cur_seq;
                    file_data_array = realloc(file_data_array, magic * sizeof(file_data));
                        for(int i = old_magic; i < magic; i++)
                            file_data_array[i].status = -10;
                }
                handle_data(recieve_data, cur_seq, recvInfo, dest, file_data_array, magic);
                /*generate ACK*/
                to_sender_ACK.type[0] = 'A';
                to_sender_ACK.type[1] = 'A';
                to_sender_ACK.type[2] = 'A';
                to_sender_ACK.number =  cur_seq;;
                sentBytes = sendto(s, &to_sender_ACK, msg_total_size, 0, (struct sockaddr*)&si_other, slen);
            }
        }     

    }
}


int reliablyReceive(unsigned short int myUDPport, char* destinationFile) {
    
    slen = sizeof (si_other);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    //fcntl(s, F_SETFL, O_NONBLOCK);

    memset((char *) &si_me, 0, sizeof (si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(myUDPport);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    //printf("Now binding\n");
    if (bind(s, (struct sockaddr*) &si_me, sizeof (si_me)) == -1)
        diep("bind");

    // fcntl(s, F_SETFL, O_NONBLOCK);
	// /* Create recv_output file */
    FILE* dest = create_file(destinationFile);
    /*init reciever*/
    recv_info* recvInfo = int_receiver();
    /*receieve enter LISTEN state*/
    recvInfo->handshake_state  = LISTEN;
    /*start recieve data*/
    recv_packet(dest, recvInfo);
    //int recvBytes  = 0;
    //char recvBuffer[msg_total_size];
   // printf("this code has problem");
//  while(1){
//         if ((recvBytes = recvfrom(s, recvBuffer, msg_total_size, 0, (struct sockaddr*)&si_other, &slen)) == -1){
//             printf("sd?\n");
//             //continue;
//         }
//         break;
//    }
//         printf("1111111111111111");
//         //break;
//     //}
        
//     //     //perror("receiver recv_packet failed\n");
//     //     //exit(1);
//     // // }
    //int flag = 0;
    //while(1){
        //printf("????????????????????/");
   // int sentBytes = sendto(s, "ABC", msg_total_size, 0, (struct sockaddr*)&si_other, slen);
        //break;
    //}
    
	/* Now receive data and send acknowledgements */   
    //pthread_t recv_tid;
    //pthread_create(&recv_tid, 0, recv_packet, (void *)0);
    //pthread_join(recv_tid, NULL);
    //printf("Thread finished\n");
    // pthread_join(time_tid, NULL);


    close(s);
    fclose(dest);
	//printf("%s received.", destinationFile);
    return 0;
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

    return 0;
}

