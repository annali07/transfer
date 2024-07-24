#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>
#include "latency_helpers.h"

#define PAGE_SIZE 4096 // 4 kB
#define BILLION  1000000000L

struct thread_args {
    int sockfd;
    int file_size;
    char *buffer;
    int requests_per_thread;
    double *latencies;
    int thread_index;
};

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

int validate_success_message(int sock) {
    int32_t net_number;
    if (recv(sock, &net_number, sizeof(net_number), 0) < 0) {
        printf("Failed to receive success message\n");
        return -1;
    }
    int number = ntohl(net_number);
    if (number != 6) {
        printf("Failed to receive success message\n");
        return -1;
    }
    return 0;
}

int send_file(int sock, int file_size, char *buffer) {
    // Send file data
    long bytes_to_send;
    int total_bytes_sent = 0;
    int bytes_sent;

    while (total_bytes_sent < file_size) {
        // Calculate how many bytes to send in this iteration
        bytes_to_send = file_size - total_bytes_sent;
        if (bytes_to_send > PAGE_SIZE) {
            bytes_to_send = PAGE_SIZE;  // Send in chunks of PAGE_SIZE
        }

        // Send the bytes from the buffer
        bytes_sent = send(sock, buffer + total_bytes_sent, bytes_to_send, 0);
        if (bytes_sent < 0) {
            perror("Failed to send file data");
            free(buffer);
            close(sock);
            return -1;
        }
        total_bytes_sent += bytes_sent;
    }

    if (validate_success_message(sock) < 0) {
        close(sock);
        return -1;
    }
    // printf("File sent successfully.\n");
    return 0;
}

void *send_files(void *args){
    struct thread_args *targs = (struct thread_args *)args;
    struct timespec start, stop;
    double latency;
    int result;
    int start_index = targs->thread_index * targs->requests_per_thread;

    for(int i = 0; i < targs->requests_per_thread; i++){
        if(clock_gettime(CLOCK_REALTIME, &start) < 0){
            printf("clock get start time error");
            return NULL;
        }
        result = send_file(targs->sockfd, targs->file_size, targs->buffer);
        if(result < 0){
            printf("failed to send file\n");
            return NULL;
        }
        if(clock_gettime(CLOCK_REALTIME, &stop) < 0){
            printf("clock get stop time error");
            return NULL;
        }

        latency = (stop.tv_sec - start.tv_sec) * (double)BILLION + (double)(stop.tv_nsec - start.tv_nsec);
        latency = latency / 1000.0;

        targs->latencies[start_index + i] = latency;
    }

    close(targs->sockfd);
    return NULL;
}

int main(int argc, const char** argv){
    
    if (argc != 7){
        fprintf(stderr, "invalid arguments: must be server ip, server port, block size, number of threads, target metrics, total requests\n");
        exit(1);
    }

    const char *SERVER_IP = argv[1];
    char *endptr = NULL;

    // Convert SERVER_PORT to int
    int SERVER_PORT = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid SERVER_PORT: %s\n", argv[2]);
        exit(1);
    }

    // Convert file_size to long int
    long int file_size = strtol(argv[3], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid file_size: %s\n", argv[3]);
        return EXIT_FAILURE;
    }

    // Convert num_threads to int
    int num_threads = strtol(argv[4], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid num_threads: %s\n", argv[4]);
        return EXIT_FAILURE;
    }

    // Convert target_metric to int
    int target_metric = strtol(argv[5], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid target_metric: %s\n", argv[5]);
        return EXIT_FAILURE;
    }

    // Convert total_requests to int
    int total_requests = strtol(argv[6], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid total_requests: %s\n", argv[6]);
        return EXIT_FAILURE;
    }
    // End of Args

    // Begin Server Connection

    char *buffer;
    pthread_t threads[num_threads];
    struct thread_args targs[num_threads];
    double *latencies;
    double total_latency = 0;

    buffer = malloc(file_size+1);
    if (!buffer) {
        perror("Failed to allocate buffer");
        return -1;
    }

    // Threads
    latencies = malloc(sizeof(double) * total_requests);
    if (!latencies) {
        perror("Failed to allocate buffer");
        return -1;
    }

    for (int i = 0; i < total_requests; i++) {
        latencies[i] = 0;
    }

    int requests_per_thread = total_requests / num_threads;

    // Concurrency
    for (int i = 0; i < num_threads; i++) {
        int sockfd = connect_to_server(SERVER_IP, SERVER_PORT);
        if (sockfd == -1){
            return -1;
        }
        
        // Check file size limit
        if (file_size < 8 || file_size > 1048576) {
            fprintf(stderr, "File size is out of the expected range (8B to 1024kB)\n");
            close(sockfd);
            return -1;
        }

        // Send file size to the server
        if (send(sockfd, &file_size, sizeof(file_size), 0) < 0) {
            printf("Failed to send file size");
            close(sockfd);
            return -1;
        }
        printf("File size sent success\n");
        
        targs[i].sockfd = sockfd;
        targs[i].file_size = file_size;
        targs[i].buffer = buffer;
        targs[i].requests_per_thread = requests_per_thread;
        targs[i].latencies = latencies;
        targs[i].thread_index = i;
        pthread_create(&threads[i], NULL, send_files, (void *)&targs[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

	Statistics LatencyStats;
	Percentiles PercentileStats;
	GetStatistics(latencies, (size_t)total_requests, &LatencyStats, &PercentileStats);
	printf(
		"Result for %d requests of %ld bytes (%.2lf seconds):\nRPS: %.2lf RPS\nStdErr: %.2lf\n", 
		total_requests,
		(long) file_size,
		(total_latency / 1000000),
		((size_t)total_requests / total_latency * 1000000),
		LatencyStats.StandardError
	);
	switch (target_metric) {
        case 1:
            printf("Min: %.2lf, Max: %.2lf, 50th: %.2lf, 90th: %.2lf, 99th: %.2lf, 99.9th: %.2lf, 99.99th: %.2lf\n",
                   LatencyStats.Min,
                   LatencyStats.Max,
                   PercentileStats.P50,
                   PercentileStats.P90,
                   PercentileStats.P99,
                   PercentileStats.P99p9,
                   PercentileStats.P99p99);
            break;
        case 2:
            printf("Min: %.2lf\n", LatencyStats.Min);
            break;
        case 3:
            printf("Max: %.2lf\n", LatencyStats.Max);
            break;
        case 4:
            printf("50th: %.2lf\n", PercentileStats.P50);
            break;
        case 5:
            printf("90th: %.2lf\n", PercentileStats.P90);
            break;
        case 6:
            printf("99th: %.2lf\n", PercentileStats.P99);
            break;
        case 7:
            printf("99.9th: %.2lf\n", PercentileStats.P99p9);
            break;
        case 8:
            printf("99.99th: %.2lf\n", PercentileStats.P99p99);
            break;
        default:
            printf("Invalid target metric.\n");
            break;
    }
	free(latencies);
    return 0;
}