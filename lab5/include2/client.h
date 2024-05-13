#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
using namespace boost::asio;
io_service service;


struct AddessServer {
    AddessServer() 
        : sock_(service), started_(true) {}
    
    void connect(ip::tcp::endpoint ep) {
        sock_.connect(ep);
    }

    void loop() {
        std::cout << "Write:\n    'login' to login "
        "\n    'choose' to choose a patient"
        "\n    'info' to get all info of chosen patient"
        "\n    'add' to add new record to chosen patient's card"
        "\n    'view' to view records of chosen patient's card";
        // quit logout
        std::cout << "\nStart by 'login' and 'choose'\n";
        while ( started_) {
            get_request();
            read_answer();
            
        }
    }
    
private:
    bool get_request() {
        std::string request;
        std::cout << "\nYour input: ";
        std::cin >> request;
        
        if(request == "login") {
            login();
            return 1;
        }
        else if(request == "choose") {
            choose();
            return 1;
        }
        else if(request == "info") {
            get_info();
            return 1;
        }
        else if(request == "add") {
            add_record();
            return 1;
        }
        else if(request == "view") {
             view_record();
             return 1;
        }
        else {
            return 0;
        }
    }

    void login(){
        write("login ");
        std::cout << "Write your login: ";
        std::string l, p;
        std::cin >> l;
        std::cout << "\nWrite your password: ";
        std::cin >> p;

        write(l + p + "*");
    }    

    void choose(){
        write("choose ");
        std::cout << "\nWrite SNILS of patient to be chosen without spaces: ";
        std::string s;
        std::cin >> s;
        write(s + "*");
    }

    void get_info(){
        write("info*");
    }

    void add_record(){
        write("add|");
        std::string s;
        std::cout << "\nWrite date as YYYY-MM-DD: ";
        std::cin >> s;
        write(s + "|");
        std::cout << "\nWrite complaints of patient "
                     "(or write '-' if no complaints) and press enter: \n";
        getline(std::cin, s);
        write(s + "|");
        std::cout << "\nWrite observation of patient "
                     "(or write '-' if no observation) and press enter: \n";
        getline(std::cin, s);
        write (s + "|");
        std::cout << "\nWrite consultion for patient "
                     "(or write '-' if no consultion) and press enter: \n";
        getline(std::cin, s);
        write(s + "|");
        std::cout << "\nWrite prescribed medication for patient "
                     "(or write '-' if no medication) and press enter: \n";
        getline(std::cin, s);
        write(s + "|");
        std::cout << "\nWrite doctor's name (or write '-') and press enter:\n";
        getline(std::cin, s);
        write(s + "*");
        read_answer();
        std::cin >> s;
        write(s + "*");
    }

    void view_record(){
        // write("view ");
        // std::cout << "Write ...";
    }


    void read_answer() {
        already_read_ = 0;
        while(!findStar()){
            if ( sock_.available()){
                already_read_ += sock_.read_some(boost::asio::buffer(buff_ + already_read_, buffSize_ - already_read_));
            }
        }
        std::string ans = getmsg();
        std::cout << "Server answered: " << ans;
        
    }

    bool findStar(){
        bool found = std::find(buff_, buff_ + already_read_, '*') < buff_ + already_read_;
        return found;
    }

    std::string getmsg(){
    
    std::size_t pos = std::find(buff_, buff_ + already_read_, '*') - buff_;
    // getting message
    std::string msg(buff_, pos); 
    // deleting messege from buffer
    std::copy(buff_ + already_read_, buff_ + buffSize_, buff_); 
    already_read_ -= pos + 1;

    return msg;
}

    void write(const std::string & msg) {
        sock_.write_some(buffer(msg));
    }

private:
    ip::tcp::socket sock_;
    enum { buffSize_ = 1024 };
    int already_read_;
    char buff_[buffSize_];
    bool started_;
    
};

ip::tcp::endpoint ep( ip::address::from_string("127.0.0.1"), 8001);
void run_client() {
    AddessServer client;
    try {
        client.connect(ep);
        std::cout<< "Connected to server\n";
        client.loop();
    } catch(boost::system::system_error & err) {
        std::cout << "Error client terminated: " << err.what() << std::endl;
    }
}
