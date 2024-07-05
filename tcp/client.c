#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PAGE_SIZE 4096 // 4 kB

int connect_to_server(const char* server_ip, int port){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { 
        fprintf(stderr, "Error connecting to server\n");
        return -1;
    }
    struct sockaddr_in addr; 
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(server_ip);

    int err = connect(sockfd, (struct sockaddr*) &addr, sizeof(addr));
    if (err == -1){
        fprintf(stderr, "Error connecting to server\n");
    }
    printf("Connected to server\n");
    return sockfd;
}

int fill_buffer_with_file(uint8_t* buffer, long long int buf_size, const char* file_location){
    FILE* f = fopen(file_location, "r"); 
    if (f == NULL){
        fprintf(stderr, "Error opening file\n");
        return -1;
    }
    int elems_read = fread(buffer, sizeof(uint8_t), buf_size, f);
    fclose(f);
    if (elems_read == -1){
        fprintf(stderr, "Error reading file\n");
        return -1;
    } 
    if (elems_read != buf_size){
        fprintf(stderr, "Insufficient data read from file\n");
        return -1;
    }
    return 0;
}

int loop_send(int fd, uint8_t* buf, long long int buf_size, int num_loops){
    int elems_written = 0;
    for (int i = 0; i < num_loops; i++){
        for (int j = 0; j < buf_size/PAGE_SIZE; j++){
            elems_written = write(fd, buf + (j * PAGE_SIZE), PAGE_SIZE);
            if (elems_written == -1) {
                fprintf(stderr, "%d", j);
                fprintf(stderr, "Error sending data to server\n");
                return -1;
            }
            if (elems_written != PAGE_SIZE){
                fprintf(stderr, "%i %d %lld %d",i,  j, buf_size/PAGE_SIZE, elems_written);
                fprintf(stderr, "Potential issue: didn't send PAGE_SIZE to server\n");
            }
        }
    }
    return 0;
}

int main(int argc, const char** argv){
    if (argc != 5){
        fprintf(stderr, "invalid arguments: must be server ip, server port, number of loops, file location\n");
        exit(1);
    }
    const char* SERVER_IP = argv[1];
    char** temp = NULL;
    int SERVER_PORT = strtol(argv[2], temp, 10);
    int num_loops = strtol(argv[3], temp, 10);
    const char* file_location = argv[4];
    
    int sockfd =  connect_to_server(SERVER_IP, SERVER_PORT);
    if (sockfd == -1){return -1;}
    
    long long int buf_size = 4096; // 1 gigabyte
    uint8_t* buf = malloc(buf_size); // I had an issue on my machine with more than 1 gig
    int err = fill_buffer_with_file(buf, buf_size, file_location);
    if (err) {
        free(buf);
        return -1;
    } 
    //maybe threadify later
    err = loop_send(sockfd, buf, buf_size, num_loops);
    free(buf);
    if (err) {return -1;}
    return 0;
    
}