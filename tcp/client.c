#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "latency_helpers.h"

#define PAGE_SIZE 4096 // 4 kB
#define BILLION  1000000000L

int g_file_size = 0; 

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
        perror("Failed to receive success message\n");
        return -1;
    }
    int number = ntohl(net_number);
    if (number != 6) {
        perror("Failed to receive success message\n");
        return -1;
    }
}

int send_file(int sock, const char *file_path) {
    FILE *file;
    long file_size;
    char *buffer;
    // Open file
    file = fopen(file_path, "rb");
    if (file == NULL) {
        perror("Cannot open file");
        close(sock);
        return -1;
    }

    // Get file size
    fseek(file, 0L, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    // Check file size limit
    if (file_size < 1024 || file_size > 1048576) {
        fprintf(stderr, "File size is out of the expected range (1kB to 1024kB)\n");
        fclose(file);
        close(sock);
        return -1;
    }

    // Send file size to the server
    if (send(sock, &file_size, sizeof(file_size), 0) < 0) {
        perror("Failed to send file size");
        fclose(file);
        close(sock);
        return -1;
    }

    g_file_size = file_size;

    // Allocate memory for file buffer
    buffer = malloc(1024 * 1024);
    if (!buffer) {
        perror("Failed to allocate buffer");
        fclose(file);
        close(sock);
        return -1;
    }

    // Send file data
    long bytes_read;
    while ((bytes_read = fread(buffer, 1, PAGE_SIZE, file)) > 0) {
        if (send(sock, buffer, bytes_read, 0) < 0) {
            perror("Failed to send file data");
            free(buffer);
            fclose(file);
            close(sock);
            return -1;
        }
    }
    if (validate_success_message(sock) < 0) {
        close(sock);
        return -1;
    }

    free(buffer);
    fclose(file);
    printf("File sent successfully.\n");
    return 0;
}

int main(int argc, const char** argv){
    struct timespec start, stop;
    double latency, total_latency;
    double *latencies;
    int test_rounds = 10;
    int result = 0;
    
    if (argc != 4){
        fprintf(stderr, "invalid arguments: must be server ip, server port, file location\n");
        exit(1);
    }
    const char* SERVER_IP = argv[1];
    char** temp = NULL;
    int SERVER_PORT = strtol(argv[2], temp, 10);
    const char* file_location = argv[3];
    
    int sockfd =  connect_to_server(SERVER_IP, SERVER_PORT);
    if (sockfd == -1){return -1;}

    // Start Timing 
    total_latency = 0;
    latencies = malloc(sizeof(double) * test_rounds);
    for (size_t i = 0; i != test_rounds; i++) {
	     latencies[i] = 0;
	}

    for (size_t i = 0; i < test_rounds; i++) {
		// sends file
		if(clock_gettime(CLOCK_REALTIME, &start) == -1) {
			perror("clock gettime");
			return -1;
		}
		result = send_file(sockfd, file_location);
		
        if (result < 0) {
			perror("fail");
			exit(0);
		}
        

		if(clock_gettime( CLOCK_REALTIME, &stop) == -1) {
			perror("clock gettime");
			return -1;
		}

		latency = (stop.tv_sec - start.tv_sec) * (double)BILLION
			     + (double)(stop.tv_nsec - start.tv_nsec);
		latency = latency / 1000.0;

		latencies[i] = latency;
		total_latency += latency;
	}

	Statistics LatencyStats;
        Percentiles PercentileStats;
        GetStatistics(latencies, (size_t)test_rounds, &LatencyStats, &PercentileStats);
        printf(
                "Result for %d requests of %d bytes (%.2lf seconds): %.2lf RPS, Min: %.2lf, Max: %.2lf, 50th: %.2lf, 90th: %.2lf, 99th: %.2lf, 99.9th: %.2lf, 99.99th: %.2lf, StdErr: %.2lf\n",
                test_rounds,
                g_file_size,
                (total_latency / 1000000),
                (test_rounds / total_latency * 1000000),
                LatencyStats.Min,
                LatencyStats.Max,
                PercentileStats.P50,
                PercentileStats.P90,
                PercentileStats.P99,
                PercentileStats.P99p9,
                PercentileStats.P99p99,
                LatencyStats.StandardError);

	free(latencies);
    close(sockfd);
    return 0;
}