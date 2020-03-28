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

int precreate_handle(){

}

int main(int argc, char **argv) {

    ServerSocket server;
    server.hostname = nullptr;
    server.port = "12345";

    try {
        server.setup();
    } catch (GeneralException &e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }


    while (1) {
        int client_fd;
        try {
            client_fd=server.ServerAccept();
        }
        catch (GeneralException &e) {
            std::cout << e.what() << std::endl;
        }
        if (client_fd > 0) {
            if(argc==2){

            }

        }
    }
    std::cout << "NOOOOOOOO" << std::endl;
    return EXIT_SUCCESS;
}