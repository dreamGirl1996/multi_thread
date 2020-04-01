#ifndef __UTILS_H__
#define __UTILS_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <sys/time.h>

void closeSockfd(int & socket_fd) {
    if (socket_fd >= 0) {
        close(socket_fd);
        socket_fd = -1;
    }
}

void freeSockAddrList(struct addrinfo * host_info_list) {
    if (host_info_list != NULL) {
        freeaddrinfo(host_info_list);
        host_info_list = NULL;
    }
}

double delay(int req_delay) {
    struct timeval start, check;
    double elapsed_seconds;
    gettimeofday(&start, NULL);
    do {
        gettimeofday(&check, NULL);
        elapsed_seconds = (check.tv_sec + (check.tv_usec/1000000.0)) - \
        (start.tv_sec + (start.tv_usec/1000000.0));
    } while (elapsed_seconds < req_delay);
    std::cout<<"elapsed_second "<<elapsed_seconds<<std::endl;
    return elapsed_seconds;
}

#endif