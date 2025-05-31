#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdarg.h>

#define MAX_CLIENTS 150
#define CONFIG_FILE "config"
#define MAX_SOCKET_NAME_LENGTH 100
#define MAX_BUFFER_SIZE 11

int current_state = 0;
FILE* log_file_ptr;

void write_log(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int log_length = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char* log_message = malloc(log_length + 1);
    va_start(args, format);
    vsnprintf(log_message, log_length + 1, format, args);
    va_end(args);
    fwrite(log_message, 1, log_length, log_file_ptr);
    fflush(log_file_ptr);
    free(log_message);
}

char* get_socket_path() {
    FILE* config_file = fopen(CONFIG_FILE, "r");
    char* socket_name = malloc(MAX_SOCKET_NAME_LENGTH);
    fgets(socket_name, MAX_SOCKET_NAME_LENGTH, config_file);
    int path_length = snprintf(NULL, 0, "/tmp/%s", socket_name);
    char* socket_path = malloc(path_length + 1);
    snprintf(socket_path, path_length + 1, "/tmp/%s", socket_name);
    free(socket_name);
    fclose(config_file);
    return socket_path;
}

int create_server_socket() {
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un address;
    memset(&address, 0, sizeof(struct sockaddr_un));

    address.sun_family = AF_UNIX;
    char* socket_path = get_socket_path();
    strcpy(address.sun_path, socket_path);
    free(socket_path);

    int option_value = 0;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&option_value, sizeof(option_value));

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("Failed to listen on socket");
        exit(EXIT_FAILURE);
    }
    return server_fd;
}

void run_server(int server_fd) {
    write_log("Server started successfully\n");
    fd_set read_fds;
    int client_fds[MAX_CLIENTS] = {0};

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        int max_fd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] > 0) {
                FD_SET(client_fds[i], &read_fds);
                if (client_fds[i] > max_fd) {
                    max_fd = client_fds[i];
                }
            }
        }

        int ready_fds = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (ready_fds < 0) {
            perror("Error in select");
            continue;
        }

        if (FD_ISSET(server_fd, &read_fds)) {
            int new_client_fd = accept(server_fd, NULL, NULL);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_fds[i] == 0) {
                    client_fds[i] = new_client_fd;
                    long heap_break = (long)sbrk(0);
                    write_log("Accepted new client (fd: %d). Current heap break: %ld\n",
                             client_fds[i], heap_break);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] > 0 && FD_ISSET(client_fds[i], &read_fds)) {
                char buffer[MAX_BUFFER_SIZE] = {0};
                int bytes_read = recv(client_fds[i], buffer, MAX_BUFFER_SIZE, 0);

                if (bytes_read <= 0) {
                    close(client_fds[i]);
                    client_fds[i] = 0;
                    continue;
                }

                int received_number = atoi(buffer);
                current_state += received_number;
                write_log("Received number: %d; Current state: %d\n",
                          received_number, current_state);

                char response[MAX_BUFFER_SIZE] = {0};
                snprintf(response, MAX_BUFFER_SIZE, "%d", current_state);
                send(client_fds[i], response, strlen(response), 0);
            }
        }
    }
}

int main() {
    log_file_ptr = fopen("/tmp/server.log", "w");
    if (!log_file_ptr) {
        perror("Failed to open log file");
        return EXIT_FAILURE;
    }

    int server_fd = create_server_socket();
    run_server(server_fd);
    close(server_fd);
    fclose(log_file_ptr);

    return EXIT_SUCCESS;
}
