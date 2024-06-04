#include "server.h"

#include <iostream>

void run_client(Server_ptr ptr);

void acceptClients();

int main(){
    // error opening db
    if(!editor.get_opened()){
        return 0;
    }
    try{
        boost::thread th(acceptClients);
        th.join();
    } catch(...){
        std::cerr << "Error\n";
    }
    return 0;
}

void run_client(Server_ptr ptr){
    ptr->run();
}

void acceptClients() {
    using namespace boost::asio;
    ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8001));
    while (true){ 
        Server_ptr new_(new Server);
        acceptor.accept(new_->get_socket());
        {
            std::lock_guard<std::mutex> lk(cout_mtx);
            std::cout << "Client is accepted\n";
        }  
        boost::thread th{run_client, new_};
        th.detach(); 
    }  
}

