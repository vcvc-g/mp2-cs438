#include "sender_helper.h"

file_data* file_data_array;


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
    FILE* fd = fopen(filename, "r");
    if (fd == NULL) {
        printf("ERROR: FILE OPEN FAILED\n");
    }

    /*find the file size*/
    fseek(fd, 0, SEEK_END); 
    int file_size = ftell(fd); 
    fseek(fd, 0, SEEK_SET);

    /*find the msg data size & max sequnce number*/
    size_t data_size = bytesToTransfer < file_size ? bytesToTransfer  : file_size;
    int packet_num = data_size / msg_body_size;
    if(data_size % msg_body_size)
        packet_num = packet_num + 1;
    /*malloc enough space for file_data array*/
    file_data_array = calloc(packet_num, sizeof(file_data));

    char *file = malloc(data_size);
    fread(file, data_size, 1, fd);
    fclose(fd);

    /*construct the file data array*/
    size_t cur_file_length = 0;

    for(int i = 0; i < packet_num; i++ ){
        cur_file_length = msg_body_size;
        if((i == (packet_num - 1)) && (data_size - i*msg_body_size) != 0)
            cur_file_length = data_size - i*msg_body_size;
            char* start_point = file + i*msg_body_size;
        /*create message*/
        char *msg = malloc(cur_file_length + sender_header_size); 
        msg[0] = 'S';
        msg[1] = (i % max_seq) / 255; //make sure the number is within one byte
        msg[2] = (i % max_seq) % 255;
        for(int j = 0; j < cur_file_length; j++ )
            msg[j + 3] = *(start_point + j);

        file_data_array[i].data = msg;
        file_data_array[i].length = sender_header_size + cur_file_length;
    }

     return packet_num;

}



