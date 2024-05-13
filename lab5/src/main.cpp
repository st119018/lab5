#include "server.h"

#include <iostream>


using namespace boost::asio;

int main(){
    boost::thread_group threads;
    threads.create_thread(acceptClients);
    threads.create_thread(handleClients);
    threads.join_all();

    return 0;
}
