#include "server.h"

#include <iostream>

int main(){
    boost::thread_group th;
    th.create_thread(acceptClients);
    th.create_thread(delClients);
    th.join_all();
    return 0;
}
