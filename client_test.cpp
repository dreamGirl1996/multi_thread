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
#include "serversocket.h"

#define BUFFER_SIZE 10
#define NUM_THREADS 100
//using namespace std;

std::mutex mtx;


struct clientObject
{
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
        struct server_bucket *my_data;

        my_data = (struct server_bucket *) threadarg;

        // char buffer[50];
        // memset(buffer, 0, 50);
        // char char_delay_count[50];
        // sprintf(char_delay_count,"%d",my_data->delay_count);
        // char char_bucket_item[50];
        // sprintf(char_bucket_item,"%d",my_data->bucket_item);
        // strcpy(buffer,char_delay_count);
        // strcat(buffer,",");
        // strcat(buffer,char_bucket_item);
        // strcat(buffer,",");
        std::string sendMsg = std::to_string(my_data->delay_count) + "," + \
        std::to_string(my_data->bucket_item) + ",";

        std::cout << "client send buffer " << sendMsg << std::endl;
        send(my_data->socket_fd, sendMsg.c_str(), sendMsg.size(), 0);

        //send(my_data->socket_fd,my_data, sizeof(my_data),0);
        //send(my_data->socket_fd,&my_data->delay_count,sizeof(int), 0);
        //send(my_data->socket_fd,&my_data->bucket_item,sizeof(int), 0);

        double success=0;
        recv(my_data->socket_fd,&success, sizeof(int),0);

    try {
        // using a local lock_guard to lock mtx guarantees unlocking on destruction / exception:
        std::lock_guard<std::mutex> lck (mtx);
        std::cout<<"new item value after update in client "<<success<<std::endl;

        std::cout << "delay_count : " << my_data->delay_count ;
        std::cout << " bucket_item : " << my_data->bucket_item << std::endl;
        std::cout << "id is "<<my_data->thread_id<<std::endl;
        std::cout<<"------------"<<std::endl;
    }
    catch (std::logic_error&) {
        std::cout << "[exception caught]\n";
    }



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
    struct clientObject obj;
    int rc;
    int i;

    for( i=0; i < NUM_THREADS; i++ ){
        obj.delay_count = delay_count;
        obj.bucket_item = bucket_item;
        obj.socket_fd = socket_fd;
        obj.thread_id =i;
        //td[i].message = (char*)"This is message";
        rc = pthread_create(&threads[i], NULL,
                            ClientSend, (void *)&obj);
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
