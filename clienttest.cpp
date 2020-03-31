#include <string>
#include <cstring>
#include "serversocket.h"
#include "clientsocket.h"
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
#include <cstdio>
#include <cctype>
#include <iostream>
#include <string>

std::mutex mtx;

/*
 * clientSocketFD: file descriptor of socket on the client side
 * isLargeVariation: 0 for small delay variation(1-3s), 1 for large(1-20s)
 * count: number of total handled requests
 */
struct handleCLientParameter {
    ClientSocket * clientSocket;
    bool isLargeVariation;
    int count;
    //int bucketSize;
};



void handleClient(void * para) {
    handleClientParameter *(para);
    // create a message in a range according to delay variation type
    srand((unsigned)time(NULL));
    int delayVariation, bucketIdx;
    // small variation
    if (!para.isLargeVariation) {
        delayVariation = rand() % 3 + 1;
    }
    // large variation
    else {
        delayVariation = rand() % 20 + 1;
    }
    // TODO
    // assume bucket size is 512
    bucketIdx = rand() % 512;
    string sendMsg = std::to_string(delayVariation) + "," + std::to_string(bucketIdx) + "\n";
    // send request to server
    para.clientSocket->sendRequest(sendMsg);
    // receive response from server
    string recvMsg = para.clientSocket->receiveResponse();
    // successfully receive the message
    if (!recvMsg.size()) {
        // update count of total handled number of requests
        std::lock_guard(std::mutex) lck (mtx);
        ++para.count;
    }
}



/*
 * func parameters: 3 parameters
 */
int main(int argc, char **argv) {
    // default parameters
    int num_requests = 10, bucket_size = 32;
    string run_prog = "thread_pool";
    bool isLargeVariation = false;

    // print some hints
    std::cout << "Please open two terminals, one for the server side and the other for the client side." << std::endl;
    std::cout << "You should enter three parameters. The first one determines the side. Enter s to run server side codes,"
                 " enter c to run client side codes." << std::endl;
    std::cout << "For the client side, the second parameter is the number of requests." << std::endl;
    std::cout << "For the server side, as second parameter enter p for opening a thread pool or t to create "
                 "thread per request." << std::endl;
    std::cout << "For the client side, as third parameter enter the variation of delay, 0 for small (1-3s) and 1 for "
                 "large (1-20s)." << std::endl;
    std::cout << "For the server side, as third parameter enter the bucket size. 0 for 32 buckets, 1 for 128 buckets, "
                 "2 for 512 buckets, 3 for 2048 buckets." << std::endl;
//    std::cout << "For the client side, as fourth parameter enter the server side bucket size. 0 for 32 buckets, 1 for 128"
//                 "buckets, 2 for 512 buckets, 3 for 2048 buckets." << std::endl;

    // run client side codes
    if (argc == 4) {
        // invalid first parameter
        if (strlen(argv[1]) != 1) {
            std::cerr << "Invalid parameter, please enter s to run server side codes or c to run client side codes." <<
            std::endl;
            return EXIT_FAILURE;
        }
        char input = tolower(argv[1]);
        if (input == 's') {
            // run server side codes
            ServerSocket serverSocket();
            serverSocket.setup();
            serverSocket.ServerAccept();
        }
        // run client codes
        else if (input == 'c') {
            // set up client socket
            ClientSocket clientSocket();
            clientSocket.setUp();

            // define arguments for handleclient function
            struct handleCLientParameter para;
            para.clientSocket = &clientSocket;
            // TODO: handle invalid input
            para.isLargeVariation = stoi(argv[3]);
            para.count = 0;
            // TODO: handle invalid input
            para.bucketSize = stoi

            while(1) {
                pthread_t pid;
                pthread_create(&pid, NULL, handleClient, &para);
            }
        }
        // error
        else {
            std::cerr << "Invalid parameter, please enter s to run server side codes or c to run client side codes." <<
                      std::endl;
            return EXIT_FAILURE;
        }
    }

    // if not 3 parameters, ignore the parameters and run server side codes
    else {
        // run server codes
    }

};