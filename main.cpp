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

/*struct client_obj{
    int delay_count;
    int bucket_item;
    int socket_fd;
    int thread_id;
    };*/
std::mutex mtx;

void delay(int req_delay) {
    struct timeval start, check, end;
    double elapsed_seconds;
    gettimeofday(&start, NULL);
    do {
        gettimeofday(&check, NULL);
        elapsed_seconds = (check.tv_sec + (check.tv_usec/1000000.0)) - \
        (start.tv_sec + (start.tv_usec/1000000.0));
    } while (elapsed_seconds < req_delay);
}

void* handle(void *client_fd){
    //try {
        // using a local lock_guard to lock mtx guarantees unlocking on destruction / exception:
        //std::lock_guard<std::mutex> lck (mtx);
        int delay_count=0;
        int bucket_item=0;
        //struct client_obj *clientObj;
        //recv(client_fd,&clientObj, sizeof(clientObj),0);
        int i=*(int*)client_fd; //void *转int

        recv(i,&delay_count, sizeof(delay_count),0);
        recv(i,&bucket_item, sizeof(bucket_item),0);
        std::cout<<"delay_count"<<delay_count<<std::endl;
        int succ=2;
        send(i,&succ, sizeof(succ),0);
        return NULL;
   // }
    //catch (std::logic_error&) {
       // std::cout << "[exception caught]\n";
	//return NULL;
    //}
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
    std::vector<double> bucket(bucket_size,0);

    try {
        server.setup();
    } catch (GeneralException &e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    int n=0;
    int client_fd;
    try {
        client_fd=server.ServerAccept();
    }
    catch (GeneralException &e) {
        std::cout << e.what() << std::endl;
    }
    while (n<100) {

        if (client_fd > 0) {
            pthread_t pId;
            int ret;
            ret = pthread_create(&pId,NULL,handle,(void *)&client_fd);
            if(ret != 0)
            {
                printf("create pthread error!\n");
                exit(1);
            }
            //std::thread th(handle,client_fd);
            //th.detach();
            pthread_join(pId,NULL);
        }else{
            std::cout<<"client poor communication"<<std::endl;
        }
        n++;
        std::cout<<"thread is "<<n<<std::endl;
        if(n==100){
            std::cout<<"finish"<<std::endl;
            close(client_fd);
        }
    }
    return EXIT_SUCCESS;
}
