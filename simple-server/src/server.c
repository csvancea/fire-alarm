#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define GUID_LEN 36

size_t ReadBytes(int sockfd, char* buf, size_t len)
{
    size_t bytes_read = 0;
    ssize_t rc;

    while (bytes_read < len) {
        rc = recv(sockfd, &buf[bytes_read], len - bytes_read, 0);
        if (rc <= 0) {
            break;
        }

        bytes_read += (size_t)rc;
    }

    return bytes_read;
}

int main(int argc, char** argv)
{
    char buf[4096];
    int listenfd, clientfd;
    int opt;
    int rc;
    struct timeval tv;
    struct sockaddr_in sockaddr, clientaddr;
    socklen_t clientlen;

    char guid[GUID_LEN + 1] = "00000000-0000-0000-0000-000000000000";
    char* endptr;
    unsigned long encoded;
    int gas_value;
    int gas_detected;
    int flame_detected;

    if (argc < 2) {
        printf("%s [port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        printf("Can't open socket: %d\n", errno);
        return EXIT_FAILURE;
    }

    /* cheap fix for errno 98 - socket already in use */
    opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons((uint16_t)strtoul(argv[1], NULL, 10));

    if (bind(listenfd, (const struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        printf("Can't bind socket: %d\n", errno);
        return EXIT_FAILURE;
    }

    if (listen(listenfd, 64) < 0) {
        printf("Can't listen on socket: %d\n", errno);
        return EXIT_FAILURE;
    }

    tv.tv_sec = 15;
    tv.tv_usec = 0;

    while (1) {
        clientlen = sizeof(clientaddr);
        clientfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
        if (clientfd < 0) {
            printf("Can't accept connection: %d\n", errno);
            continue;
        }

        switch (fork()) {
        case -1:
            printf("[%s:%d:{%s}] Fork failed: %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), guid, errno);
            break;

        case 0:
            printf("[%s:%d:{%s}] -- ACCEPTED --\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), guid);

            /* timeout after 15 seconds of waiting on recv */
            setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&tv, sizeof(tv));

            guid[GUID_LEN] = 0;
            if (ReadBytes(clientfd, guid, GUID_LEN) != GUID_LEN) {
                printf("[%s:%d:{%s}] Invalid GUID message\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), guid);
            }
            else {
                /* we received sensor guid, now listen for more data */
                while ((rc = (int)ReadBytes(clientfd, buf, sizeof(buf) - 1))) {
                    /* remove new line chars */
                    if (rc - 2 >= 0 && buf[rc - 2] == '\r') {
                        rc -= 2;
                    }
                    else if (rc - 1 >= 0 && buf[rc - 1] == '\n') {
                        rc -= 1;
                    }
                    buf[rc] = 0;

                    /* print data */
                    printf("[%s:%d:{%s}] ", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), guid);

                    encoded = strtoul(buf, &endptr, 16);
                    if (endptr - buf == rc) {
                        /*
                         * special case: we just received encoded sensor data
                         * data format: MSB -> 0000|VVVVVVVVVV|G|F <- LSB
                         *                          ^          ^ ^
                         * gas_value (10 bits) -----'          | |
                         * gas_detected (1 bit) ---------------' |
                         * flame_detected (1 bit) ---------------'
                         */

                        gas_value = (encoded >> 2) & 1023;
                        gas_detected = (encoded >> 1) & 1;
                        flame_detected = (encoded) & 1;

                        printf("Gas: %d", gas_value);
                        if (gas_detected) {
                            printf(" | THRES");
                        }
                        if (flame_detected) {
                            printf(" | FLAME");
                        }
                        printf("\n");
                    }
                    else {
                        /* normal message - print it as it is */
                        printf("%s\n", buf);
                    }
                }
            }

            printf("[%s:%d:{%s}] --  CLOSED  --\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), guid);
            close(clientfd);
            exit(EXIT_SUCCESS);
            break;

        default:
            close(clientfd);
        }
    }

    return EXIT_SUCCESS;
}
