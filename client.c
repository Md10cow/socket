#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define CONF "config"
#define MAX_SOCK_NAME 100
#define MAX_READ 11

char* get_sock_name() {
    FILE* conf_f = fopen(CONF, "r");
    char sock_name[MAX_SOCK_NAME];
    fgets(sock_name, MAX_SOCK_NAME, conf_f);
    fclose(conf_f);

    char* sock_path = malloc(MAX_SOCK_NAME + 6);
    snprintf(sock_path, MAX_SOCK_NAME + 6, "/tmp/%s", sock_name);
    return sock_path;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("./client client_id nums_to_send");
        return 1;
    }

    int client_id = atoi(argv[1]);
    int nums_count = atoi(argv[2]);
    char* sock_name = get_sock_name();

    float sleep_time = 1000 * (argc == 4 ? atof(argv[3]) * 1000 : (rand() % 255 + 1));
    printf("client: %d with delay: %f\n", client_id, sleep_time/1000000);

    struct sockaddr_un sockaddr = {0};
    sockaddr.sun_family = AF_UNIX;
    strncpy(sockaddr.sun_path, sock_name, sizeof(sockaddr.sun_path)-1);

    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    connect(sock_fd, (struct sockaddr*) &sockaddr, sizeof(sockaddr));

    time_t t0 = time(0);
    char send_buf[MAX_READ] = {0};
    char recv_buf[MAX_READ] = {0};

    for (int i = 0; i < nums_count; i++) {
        scanf("%10s", send_buf);
        usleep(sleep_time);
        write(sock_fd, send_buf, strlen(send_buf));
        read(sock_fd, recv_buf, MAX_READ);
    }

    printf("client time: %f\n", difftime(time(0), t0));
    close(sock_fd);
    free(sock_name);

    return 0;
}
