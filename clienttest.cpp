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
#include <utils.h>

std::mutex mtx;

/*
 * clientSocketFD: file descriptor of socket on the client side
 * isLargeVariation: 0 for small delay variation(1-3s), 1 for large(1-20s)
 * count: number of total handled requests
 */
// struct handleCLientParameter {
//     bool isLargeVariation;
//     int count;
//     //int bucketSize;
//     const char * hostname;
//     const char * port;
// };


/*
 * func params: a struct that includes a boolean indication variation type
 * an integer for total number of requests
 * an integer for number of buckets
 */
void * handleClient(bool isLargeVariation, const char * hostname, const char * port) {
    // struct handleCLientParameter * paraPtr = (handleCLientParameter *) para;

    // set up client socket
    ClientSocket clientSocket(hostname, port);
    clientSocket.setUp();

    // TODO: handle invalid input
    // bool isLargeVariation = paraPtr->isLargeVariation;
    // int count = paraPtr->count;

    // create a message in a range according to delay variation type
    srand((unsigned)time(NULL));
    int delayVariation, bucketIdx;
    // small variation
    if (!isLargeVariation) {
        delayVariation = rand() % 3 + 1;
    }
    // large variation
    else {
        delayVariation = rand() % 20 + 1;
    }
    // TODO
    // assume bucket size is 512
    bucketIdx = rand() % 512;
    std::string sendMsg = std::to_string(delayVariation) + "," + std::to_string(bucketIdx) + "\n";
    // send request to server
    clientSocket.sendRequest(sendMsg);
    std::cout << "client send msg: " << sendMsg;
    // receive response from server
    std::string recvMsg = clientSocket.receiveResponse();
    // successfully receive the message
    // if (!recvMsg.size()) {
    //     // update count of total handled number of requests
    //     std::lock_guard(std::mutex) lck (mtx);
    //     ++para.count;
    // }
}


int prepareServer(ServerSocket & serverSocket, std::vector<double> & bucket, int bucket_size) {
    serverSocket.hostname = nullptr;
    serverSocket.port = "12345";

    serverSocket.setup(); 
    
    int client_fd = serverSocket.ServerAccept();

    if (client_fd < 0) {
        throw GeneralException("Accept failed in prepareServer");
    }

    bucket.resize(bucket_size);

    return client_fd;
}

std::string serverRecv(int client_fd) {
    char buffer[1024];
    memset(buffer, 0, 1024);
    int numBytes = recv(client_fd, buffer, sizeof(buffer), 0);
    std::string recvMsg = "";
    if (numBytes > 0) {
        recvMsg = buffer;
        return recvMsg;
    }
    else if (numBytes == 0) {
        return recvMsg;
    }
    else {
        throw GeneralException("Server receive failed");
    }
}

void serverSend(int client_fd, std::string & sendMsg) {
    if (client_fd < 0) {
        throw GeneralException("Invalid client_fd in serverSend");
    }
    if (send(client_fd, sendMsg.c_str(), sendMsg.size(), 0) == -1) {
        throw GeneralException("Server send failed");
    }
}

void handleServer(int& request_id, int client_fd, int & count, std::vector<double> & bucket) {
    // recv string
    std::string recvMsg = serverRecv(client_fd);
    if (recvMsg.size() == 0) {
        std::cerr << request_id << ": " << "server received nothing.\n";
        return;
    }
    
    // parse string
    std::string whole, first, last;
    std::string delimiter = "\n";
    std::string whole = recvMsg.substr(0, recvMsg.find(delimiter));
    delimiter = ",";
    first = whole.substr(0, whole.find(delimiter));
    last = whole.substr(whole.find(delimiter));
    int bucketIdx;
    double delay;
    delay = std::stoi(first);
    bucketIdx = std::stoi(last);

    // update bucket and increase count
    try {
        std::lock_guard<std::mutex> lck(mtx);
        bucket[bucketIdx] += delay;
        ++count;
        // print modified buckets and current delay count
        std::cout << "Request ID: " << request_id << "count: " << count << "   " << \
        "Bucket " << bucketIdx << ": " << bucket[bucketIdx] << std::endl;
    }
    catch(GeneralException & e) {
        throw GeneralException("lock guard failed.");
    } 

    // create message to client
    std::string sendMsg = "Bucket " + std::to_string(bucketIdx + 1) + ": " + std::to_string(bucket[bucketIdx]) + "\n";
    
    // send
    serverSend(client_fd, sendMsg);
}

/*
 * func parameters: 3 parameters
 */
int main(int argc, char *argv[]) {
    // default parameters
    int num_requests = 10, bucket_size = 32;
    bool isThreadPool = false;
    bool isLargeVariation = false;

    // print some hints
    std::cout << "Please open two terminals, one for the server side and the other for the client side." << std::endl;
    std::cout << "You should enter three parameters. The first one determines the side. Enter s to run server side codes,"
                 " enter c to run client side codes." << std::endl;
    std::cout << "For the client side, the second parameter is the number of requests." << std::endl;
    std::cout << "For the server side, as second parameter enter 1 for opening a thread pool or 0 to create "
                 "thread per request." << std::endl;
    std::cout << "For the client side, as third parameter enter the variation of delay, 0 for small (1-3s) and 1 for "
                 "large (1-20s)." << std::endl;
    std::cout << "For the server side, as third parameter enter the bucket size. 1 for 32 buckets, 2 for 128 buckets, "
                 "3 for 512 buckets, 4 for 2048 buckets." << std::endl;
//    std::cout << "For the client side, as fourth parameter enter the server side bucket size. 0 for 32 buckets, 1 for 128"
//                 "buckets, 2 for 512 buckets, 3 for 2048 buckets." << std::endl;

    // run client side codes
    if (argc == 4) {
        // invalid first parameter
        if (strlen(argv[1]) != 1) {
            std::cerr << "Invalid parameter [2], please enter s to run server side codes or c to run client side codes." <<
            std::endl;
            return EXIT_FAILURE;
        }
        char checkServerOrClient = tolower((int) argv[1]);
        if (checkServerOrClient == 's') {
            // run server side codes
            std::string checkThreadPool = argv[2];
            int checkBucketSize = std::atoi(argv[3]);
            switch (checkBucketSize) {
                case 1:
                    bucket_size = 32;
                    break;
                case 2:
                    bucket_size = 128;
                    break;
                case 3:
                    bucket_size = 512;
                    break;
                case 4:
                    bucket_size = 2048;
                    break;
                default:
                    std::cerr << "Invalid parameter [4], please enter 1 for 32 buckets, 2 for 128 buckets, "
                    "3 for 512 buckets, 4 for 2048 buckets.";
                    return EXIT_FAILURE;
            }
            if (checkThreadPool == "0") {
                isThreadPool = false;
                ServerSocket serverSocket;
                std::vector<double> bucket;
                int count = 0, request_id;
                while (1) {
                    int client_fd = -1;
                    try {
                        client_fd = prepareServer(serverSocket, bucket, bucket_size);
                    }
                    catch (GeneralException & e) {
                        closeSockfd(client_fd);
                        freeSockAddrList(serverSocket.host_info_list);
                        std::cerr << e.what() << "\n";
                        continue;
                    }
                    try {
                        ++request_id;
                        handleServer(request_id, client_fd, count, bucket);
                    }
                    catch (GeneralException & e) {
                        closeSockfd(client_fd);
                        freeSockAddrList(serverSocket.host_info_list);
                    }
                }
            }
            else if (checkThreadPool == "1") {
                isThreadPool = true;
            }
            else {
                std::cerr << "Invalid parameter [3], please enter 1 to run thread pool or 0 to run thread per request.\n";
                return EXIT_FAILURE;
            }
            
        }
        // run client codes
        else if (checkServerOrClient == 'c') {
            // struct handleCLientParameter para;
            // para.isLargeVariation = isLargeVariation;
            // para.count = 0;
            // para.hostname = "vcm-13661.vm.duke.edu";
            // para.port = "12345";
            const char * hostname = "vcm-13661.vm.duke.edu";
            const char * port = "12345";
            while (1) {
                // pthread_t pid;
                try {
                    std::thread th(handleClient, isLargeVariation, hostname, port);
                    th.detach();
                }
                catch (GeneralException & e) {
                    std::cerr << e.what() << "\n";
                    continue;
                }
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