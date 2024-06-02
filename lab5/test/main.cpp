#include "client.h"

std::mutex mtx;

void send_requests(int i){
    AddressServer cl;
    try {
        cl.connect(ep);
    } catch(boost::system::system_error & err) {
        std::lock_guard<std::mutex> lk(mtx);
        std::cerr << "Error client terminated: " << err.what() << std::endl;
        return;
    }
    cl.test_write("login RL 123", i);
    cl.test_write("choose 222", i);
    cl.test_write("info", i);
    cl.test_write("view all", i);
    cl.test_write("add|2024-01-10|-|-|-|-|-", i); 
    cl.test_write("y", i);
    cl.test_write("quit", i);
    cl.close();
}

int main(){
    std::thread threads[8];

    for(int i = 0; i < 8; i++){
        threads[i] = std::thread(send_requests, i);
    }

    for(auto &th : threads){
        th.join();
    }

    return 0;
}