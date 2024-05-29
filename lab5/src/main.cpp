#include "server.h"

#include <iostream>

int main(){
    // error opening db
    if(!editor.get_opened()){
        return 0;
    }
    boost::thread_group th;
    th.create_thread(acceptClients);
    th.create_thread(delClients);
    th.join_all();
    return 0;
}
