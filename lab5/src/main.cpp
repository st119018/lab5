#include "server.h"

#include <iostream>

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
