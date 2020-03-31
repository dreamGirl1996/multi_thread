//
// Created by 陆清 on 3/28/20.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <netdb.h>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <mutex>


#define NUM_THREADS 100
//using namespace std;

std::mutex mtx;

struct client_obj{
    int delay_count;
    int bucket_item;
    int socket_fd;
    int thread_id;
};
/*
void *PrintHello(void *threadarg)
{
    try {
        // using a local lock_guard to lock mtx guarantees unlocking on destruction / exception:
        std::lock_guard<std::mutex> lck (mtx);
        struct client_obj *my_data;
        
        my_data = (struct client_obj *) threadarg;
        send(my_data->socket_fd,&my_data->delay_count,sizeof(int), 0);
        send(my_data->socket_fd,&my_data->bucket_item,sizeof(int), 0);

        int success=0;
        recv(my_data->socket_fd,&success, sizeof(int),0);
        std::cout<<"succ "<<success<<std::endl;

        std::cout << "delay_count : " << my_data->delay_count ;
        std::cout << " bucket_item : " << my_data->bucket_item << std::endl;
        std::cout << "id is "<<my_data->thread_id<<std::endl;

        pthread_exit(NULL);
	return NULL;
    }
    catch (std::logic_error&) {
        std::cout << "[exception caught]\n";
	return NULL;
    }
}*/
void *ClientSend(void *threadarg)
{
    /*try {
        // using a local lock_guard to lock mtx guarantees unlocking on destruction / exception:
        std::lock_guard<std::mutex> lck (mtx);*/
        struct client_obj *my_data;

        my_data = (struct client_obj *) threadarg;
        //send(my_data->socket_fd,&my_data, sizeof(my_data),0);
        send(my_data->socket_fd,&my_data->delay_count,sizeof(int), 0);
        send(my_data->socket_fd,&my_data->bucket_item,sizeof(int), 0);

        int success=0;
        recv(my_data->socket_fd,&success, sizeof(int),0);
        std::cout<<"succ "<<success<<std::endl;

        std::cout << "delay_count : " << my_data->delay_count ;
        std::cout << " bucket_item : " << my_data->bucket_item << std::endl;
        std::cout << "id is "<<my_data->thread_id<<std::endl;

        pthread_exit(NULL);
        return NULL;
//    }
//    catch (std::logic_error&) {
//        std::cout << "[exception caught]\n";
//        return NULL;
//    }
}


int main(int argc, char *argv[]){
    int delay_count=0;
    int bucket_item=0;
    if(argc==3){
        delay_count=atoi(argv[1]);
        bucket_item=atoi(argv[2]);
    }

    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *hostname = "vcm-13661.vm.duke.edu";
    const char *port     = "12345";

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        printf("wrong getaddrinfo");
        return -1;
    } //if

    socket_fd = socket(host_info_list->ai_family,
                       host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1) {
        printf("wrong socked_fd");
        return -1;
    }
    status=connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        printf("wrong connect 1");
        return -1;
    }

    pthread_t threads[NUM_THREADS];
    struct client_obj obj[NUM_THREADS];
    int rc;
    int i;

    for( i=0; i < NUM_THREADS; i++ ){
        std::cout <<"main() : creating thread, " << i << std::endl;
        obj[i].delay_count = delay_count;
        obj[i].bucket_item = bucket_item;
        obj[i].socket_fd = socket_fd;
        obj[i].thread_id =i;
        //td[i].message = (char*)"This is message";
        rc = pthread_create(&threads[i], NULL,
                            ClientSend, (void *)&obj[i]);
        if (rc){
            std::cout << "Error:unable to create thread," << rc << std::endl;
            exit(-1);
        }
    }

    freeaddrinfo(host_info_list);
    close(socket_fd);
    pthread_exit(NULL);

    return 0;
}
