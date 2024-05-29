#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
using namespace boost::asio;
io_service service;

inline bool isInteger(const std::string & s) {
    if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))){
       return false;
    }
    char * p;
    strtol(s.c_str(), &p, 10);
    return (*p == 0);
}

struct AddressServer {
    AddressServer() 
        : sock_(service), started_(true) {}
    
    void connect(ip::tcp::endpoint ep) {
        sock_.connect(ep);
    }

    void close(){
        sock_.close();
    }

    void loop() {
        std::cout << "Write:\n    'login' to login "
        "\n    'choose' to choose a patient"
        "\n    'info' to get all info of chosen patient"
        "\n    'add' to add new record to chosen patient's card"
        "\n    'view' to view records of chosen patient's card"
        "\n    'quit' to disconnect from server";
        
        std::cout << "\nStart by 'login' and 'choose'\n";
        while (started_) {
            get_request(); 
            read_answer();
        }
    }

    void test_write(const std::string s){
        write(s + '*');
        read_answer();
        std::cout << "\n";
    }

private:
    void get_request() {
        std::string request;
        std::cout << "\nYour input: ";
        getline(std::cin, request);

        if(request == "login") {
            login();
            return;
        }
        if(request == "choose") {
            choose();
            return;
        }
        if(request == "info") {
            get_info();
            return;
        }
        if(request == "add") {
            add_record();
            return;
        }
        if(request == "view") {
            view_record();
            return;
        }
        if(request == "quit"){
            write("quit*");
            started_ = 0;
            return;
        }
        write("unknown*");
    }

    void login(){
        write("login ");
        std::cout << "Write your login: ";
        std::string l, p;
        getline(std::cin, l);
        std::cout << "Write your password: ";
        getline(std::cin, p);

        write(l + " " + p + "*");
    }    

    void choose(){
        write("choose ");
        std::cout << "Write SNILS of patient to be chosen without spaces: ";
        std::string s;
        getline(std::cin, s);
        write(s + "*");
    }

    void get_info(){
        write("info*");
    }

    void add_record(){
        write("add|");
        std::string s;
        std::cout << "Write date as YYYY-MM-DD: ";
        getline(std::cin, s);
        while(!isDate(s)){
            std::cout << "Wrong format. Write again:";
            getline(std::cin, s);
        }
        write(s + "|");
        std::cout << "Write complaints of patient "
                     "(or write '-' if no complaints) and press enter: \n";
        getline(std::cin, s);
        write(s + "|");
        std::cout << "Write observation of patient "
                     "(or write '-' if no observation) and press enter: \n";
        getline(std::cin, s);
        write (s + "|");
        std::cout << "Write consultion for patient "
                     "(or write '-' if no consultion) and press enter: \n";
        getline(std::cin, s);
        write(s + "|");
        std::cout << "Write prescribed medication for patient "
                     "(or write '-' if no medication) and press enter: \n";
        getline(std::cin, s);
        write(s + "|");
        std::cout << "Write doctor's name (or write '-') and press enter:\n";
        getline(std::cin, s);
        write(s + "*");
        read_answer();
        std::cout << "Your input: ";
        getline(std::cin, s);
        write(s + "*");
    }

    void view_record(){
        write("view ");
        std::string s;
        std::cout << "View all records(type y/n): ";
        getline(std::cin, s);
        if(s == "y"){
            write("all*");
        }
        else{
            std::cout << "Write period\nStart date(YYYY-MM-DD): ";
            getline(std::cin, s);
            while(!isDate(s)){
                std::cout << "Wrong format. Write again: ";
                getline(std::cin, s);
            }
            write(s + " ");
            std::cout << "End date: ";
            getline(std::cin, s);
            while(!isDate(s)){
                std::cout << "Wrong format. Write again: ";
                getline(std::cin, s);
            }
            write(s + "*");
        }
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
    
    // check format of date
    bool isDate(std::string date) {
        if (date.size() != 10) return 0;
        if (isInteger(date.substr(0, 4)) &&
            isInteger(date.substr(5, 2)) && 
            isInteger(date.substr(8, 2)) &&
            date[4] == '-' && date[7] == '-') {
            return 1;
        } 
        else return 0;
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
    AddressServer client;
    try {
        client.connect(ep);
        std::cout << "Connected to server\n";
        client.loop();
        client.close();
    } catch(boost::system::system_error & err) {
        std::cout << "Error client terminated: " << err.what() << std::endl;
    }
}

