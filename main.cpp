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
#include "utils.h"
#define BUFFER_SIZE 10


std::mutex mtx;

/*void* handle(void *client_fd){
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

    int n=0;
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
    serverBucket.client_fd=client_fd;

    while (n<100) {

        if (client_fd > 0) {
            pthread_t pId;
            int ret;
            //ret = pthread_create(&pId,NULL,handle,(void *)&client_fd);
            ret = pthread_create(&pId,NULL,handle_prethread,(void *)&serverBucket);
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
