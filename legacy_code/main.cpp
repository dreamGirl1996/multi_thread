#include <string>
#include "serversocket.h"
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <vector>
#include <pthread.h>
#include <mutex>
#include <sys/time.h>
#include "myqueue.h"
#include "utils.h"
#define BUFFER_SIZE 10



std::mutex mtx;

/*double delay(int req_delay) {
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
    }*/

std::vector<int> char_to_int(char * buffer,const char *delim){
    std::vector<int> ans;
    char *p = strtok(buffer,delim);
    while(p) {
        int x;
        sscanf(p, "%d", &x);
        ans.push_back(x); //存入结果数组
        p = strtok(NULL, delim);
    }
    return ans;
}

void* handle_prethread(void *serverBucket){
    //try {
    // using a local lock_guard to lock mtx guarantees unlocking on destruction / exception:
    //std::lock_guard<std::mutex> lck (mtx);
    int delay_count=0;
    int bucket_item=0;
    //struct client_obj *clientObj;
    //recv(client_fd,&clientObj, sizeof(clientObj),0);
    struct server_bucket *serverBucket1;
    serverBucket1=(struct server_bucket*)serverBucket;

    int i= serverBucket1->client_fd; //void *转int

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    recv(i,buffer, sizeof(buffer),0);
    //recv(i,serverBucket1, sizeof(serverBucket1),0);
    //delay_count=serverBucket1->delay_count;
    //bucket_item=serverBucket1->bucket_item;
    std::vector<int> recv_vec_buf=char_to_int(buffer,",");
    delay_count=recv_vec_buf[0];
    bucket_item=recv_vec_buf[1];
    std::cout<<"vect "<<delay_count<<" "<<bucket_item<<std::endl;

    //recv(i,&delay_count, sizeof(delay_count),0);
    //recv(i,&bucket_item, sizeof(bucket_item),0);

    try {
        // using a local lock_guard to lock mtx guarantees unlocking on destruction / exception:
        std::lock_guard<std::mutex> lck (mtx);
        double new_delay=delay(delay_count);
        std::cout<<"caculate delay "<<new_delay<<std::endl;
        serverBucket1->bucket[bucket_item]+=new_delay;
        std::cout<<"new item after update in main "<<serverBucket1->bucket[bucket_item]<<std::endl;
        std::cout<<"------------"<<std::endl;
    }
    catch (std::logic_error&) {
        std::cout << "[exception caught]\n";
    }


    double succ=serverBucket1->bucket[bucket_item];
    send(i,&succ, sizeof(succ),0);
    return NULL;
    // }
    //catch (std::logic_error&) {
    // std::cout << "[exception caught]\n";
    //return NULL;
    //}
}

void *handle_thread_pool(void * serverBucket){
    while (true){
        int pool_client=dequeue();
        if(pool_client){
            handle_prethread(serverBucket);
        }
    }
}


int main(int argc, char **argv) {

    ServerSocket server;
    server.hostname = nullptr;
    server.port = "12345";
    int bucket_size=0;
    if(argc==2){
        bucket_size=atoi(argv[1]);
	std::cout<<bucket_size<<std::endl;
	std::cout<<"success"<<std::endl;
    }



    try {
        server.setup();
    } catch (GeneralException &e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    /*int n=0;
    int client_fd;
    try {
        client_fd=server.ServerAccept();
    }
    catch (GeneralException &e) {
        std::cout << e.what() << std::endl;
    }

    struct server_bucket serverBucket;
    serverBucket.bucket_size=bucket_size;
    for(int i=0;i<bucket_size;++i){
        serverBucket.bucket.push_back(0);
    }
    serverBucket.client_fd=client_fd;*/

    if(argc==2) {
        int client_fd=0;
	try {
                client_fd=server.ServerAccept();
            }
            catch (GeneralException &e) {
                std::cout << e.what() << std::endl;
            }
        struct server_bucket serverBucket;
        serverBucket.bucket_size=bucket_size;
        for(int i=0;i<bucket_size;++i){
            serverBucket.bucket.push_back(0);
        }
        serverBucket.client_fd=client_fd;

        while (true) {
            

            if (client_fd > 0) {
                pthread_t pId;
                int ret;
                //ret = pthread_create(&pId,NULL,handle,(void *)&client_fd);
                ret = pthread_create(&pId, NULL, handle_prethread, (void *) &serverBucket);
                if (ret != 0) {
                    printf("create pthread error!\n");
                    exit(1);
                }
                //std::thread th(handle,client_fd);
                //th.detach();
                pthread_join(pId, NULL);
            } else {
                std::cout << "client poor communication" << std::endl;
            }

        }
    }else if(argc==3){
      const int thread_pool_size=300;//atoi(argv[2]);
        pthread_t thread_pool[300];
        int client_fd=0;
	try {
                client_fd=server.ServerAccept();
            }
            catch (GeneralException &e) {
                std::cout << e.what() << std::endl;
            }
        struct server_bucket serverBucket;
        serverBucket.bucket_size=bucket_size;
        for(int i=0;i<bucket_size;++i){
            serverBucket.bucket.push_back(0);
        }
        serverBucket.client_fd=client_fd;
        for(int i=0;i<thread_pool_size;i++){
            pthread_create(&thread_pool[i],NULL,handle_thread_pool,(void *) &serverBucket);
        }
        while (true) {
            if (client_fd > 0) {
                enqueue(client_fd);
            } else {
                std::cout << "client poor communication" << std::endl;
            }
        }
    }

    return EXIT_SUCCESS;
}
