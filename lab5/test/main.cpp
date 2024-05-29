#include "client.h"

int main(){
    AddressServer cl;
    try {
        cl.connect(ep);
    } catch(boost::system::system_error & err) {
        std::cout << "Error client terminated: " << err.what() << std::endl;
        return 1;
    }
    cl.test_write("login RL 123");
    cl.test_write("choose 222");
    cl.test_write("info");
    cl.test_write("view all");
    cl.test_write("add|2024-03-10|-|-|-|-|-");
    cl.test_write("y");
    cl.test_write("quit");
    cl.close();
    return 0;
}