#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/times.h>
#include <sys/time.h>

#define PAGE_SIZE 4096 // 4 kB
#define MAX_FILE_SIZE 1048576 // Maximum file size is 1024kB

int listen_to_client(int port){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        fprintf(stderr, "Error creating socket\n");
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    int addrlen = sizeof(addr);

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        fprintf(stderr, "Error in setsockopt\n");
        close(server_fd);
        return -1;
    }

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr))<0) {
        fprintf(stderr, "Error binding \n");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        fprintf(stderr, "Error listening for connections\n");
        close(server_fd);
        return -1;
    }

    int sockfd = accept(server_fd, (struct sockaddr *)&addr, (socklen_t*)&addrlen);
    if (sockfd < 0) {
        fprintf(stderr, "Error in accepting\n");
        close(server_fd);
        return -1;
    }

    close(server_fd);
    return sockfd;
}

int send_success_message(int client_sock) {
    int number = 6;
    int32_t net_number = htonl(number);  // Convert to network byte order
    if (send(client_sock, &net_number, sizeof(net_number), 0) < 0) {
        perror("Failed to send number");
        return -1;
    }
    return 0;
}

int receive_file(int client_sock) {
	struct timespec start, end;
    int64_t elapsed_time_ns = 0;

    long file_size;
    if (read(client_sock, &file_size, sizeof(file_size)) < 0) {
        perror("Failed to read file size");
        return -1;
    }

    if (file_size < 1024 || file_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Received file size is out of the expected range (1kB to 1024kB)\n");
        return -1;
    }
    printf("Received file size: %ld\n", file_size);
    
    char* buffer = malloc(PAGE_SIZE);
        if (!buffer) {
            perror("Failed to allocate buffer");
            return -1;
    }

    for(int i = 0; i < 10; ++i){
        long total_bytes_received = 0;
        int bytes_received = 0;
        while(total_bytes_received < file_size){
            bytes_received = read(client_sock, buffer, PAGE_SIZE);
            if (bytes_received < 0) {
                perror("Error receiving data");
                free(buffer);
                return -1;
            }
            if (bytes_received == 0) {
                fprintf(stderr, "Connection closed by peer\n");
                free(buffer);
                return -1;
            }
            total_bytes_received += bytes_received;
        }

        if (total_bytes_received != file_size) {
            fprintf(stderr, "Mismatch in the file size received and expected\n");
        } else {
            // printf("File received successfully, total bytes: %ld\n", total_bytes_received);
            if (send_success_message(client_sock) < 0) {
                return -1;  // handle error appropriately
            }
        }
    }
    free(buffer);
    return 0;
}

int main(int argc, const char** argv) {
    if (argc != 2){
        fprintf(stderr, "invalid arguments: must be port\n");
        exit(1);
    }
    char** temp = NULL;
    int PORT = strtol(argv[1], temp, 10);

    int clientfd = listen_to_client(PORT);
    if (clientfd == -1){
        fprintf(stderr, "Error listening to client\n");
        return -1;
    }
    printf("Connected to client\n");

    if (receive_file(clientfd) < 0) {
            fprintf(stderr, "Failed to receive file\n");
    }

    close(clientfd);
    return 0;
}