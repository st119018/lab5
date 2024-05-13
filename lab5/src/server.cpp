#include "server.h"

#include <string>
#include <iostream>


boost::asio::ip::tcp::socket & AddressClient::get_socket(){ 
    return sock_; 
}

void AddressClient::answer(){
    try {
        if ( sock_.available()){
        already_read_ += sock_.read_some(boost::asio::buffer(buffer_ + already_read_, bufferSize - already_read_));
        }
        
        processRequest();
        
    } catch ( boost::system::system_error& ) {
        std::cerr << "Error answering\n";
        
        sock_.close();
        started_ = false;
    }
}

void AddressClient::processRequest(){
    
    if (!findEnter()) {
        return; // message is not full
    }
    
    last_msg_ = getmsg();

// quit logout

    switch(state_)
    {
        case 0:
            if(last_msg_.find("login") == 0) {
                login();
            }
            else write("You are not logged in*");
            break;
        
        case 1:
            if(last_msg_.find("choose") == 0) {
                choose_patient();
            } 
            else write("You need to choose patient; enter 'choose' and try again*");
            break;

        case 2:
            if(last_msg_.find("info") == 0) {
                view_patient();
            }
            else if(last_msg_.find("add") == 0) {
                state_ = 3;
                add_record();
            }
            else if(last_msg_.find("view") == 0) {
                // view_record();
            }
            else write("You can enter only 'info', 'add' or 'view'; try again*");
            break;
        
        case 3:
            add_record();
            break;
            
        default:
            std::cout << "Error: wrong state!!!\n";
            state_ = 0;
            card_id_ = 0;
            user_id_ = 0;
            write("Smth went wrong on server side.\n "
                  "You are being logged out; enter 'login' to start again*");      
    }
}

bool AddressClient::findEnter() {
    bool found = std::find(buffer_, buffer_ + already_read_, '\n') < buffer_ + already_read_;
    return found;
}

std::string AddressClient::getmsg() {
    std::size_t pos = std::find(buffer_, buffer_ + already_read_, '\n') - buffer_;
    // getting message from client
    std::string msg(buffer_, pos); 
    std::cout << "msg: " << msg << "\n";
    // deleting messege from buffer
    std::copy(buffer_ + already_read_, buffer_ + bufferSize, buffer_); 
    already_read_ -= pos + 1;
    return msg;
}

void AddressClient::login(){
    std::stringstream msg{last_msg_};
    msg >> last_msg_ >> last_msg_;
    full_str_.push_back(last_msg_);
    msg >> last_msg_;
    full_str_.push_back(last_msg_);

    user_id_ = editor.login(full_str_);
    if(user_id_ != 0){
        write("You are logged in. Write 'choose' to choose patient\n*");
        full_str_.clear();
        state_ = 1;
    }
    else{
        full_str_.clear();
        field_num_ = 0;
        write("Wrong login or password; enter 'login' and try again\n*");
        state_ = 0;
    }
    
}

void AddressClient::beautify_msg(){
    if(last_msg_.find("\n") == last_msg_.size() - 1){
        last_msg_.erase(last_msg_.size() - 1);
    }

}

void AddressClient::choose_patient(){
    std::stringstream msg{last_msg_};
    msg >> last_msg_ >> last_msg_;
    full_str_.push_back(last_msg_);

    card_id_ = editor.choose(full_str_);
        
    if (card_id_ != 0){
        write("Patient is chosen. Write 'info', 'add' or 'view'*");
        full_str_.clear();
        state_ = 2;
    } 
    else {
        full_str_.clear();
        write("Patient is not chosen; enter 'choose' and try again*");
    }

}

void AddressClient::view_patient(){
    // mutex
    Records records = editor.view_info(card_id_);

    if(!records.empty()){
        if(!records[0].empty()){
            std::string ans{};
            for (auto rec : records[0]){
                ans += rec + "\n";
            }
            full_str_.clear();
            field_num_ = 0;
            write(ans);
            state_= 4;
            return;
        }
    }
    write("smth went wrong; data not found :(*");
    
}

void AddressClient::add_record(){

    if(full_str_.empty()){
        // extract 
        std::size_t pos = last_msg_.find('|');
        for (int i= 0; i < 4; ++i){
            std::size_t prev_pos = ++pos;
            std::size_t pos = last_msg_.find('|', prev_pos);
            full_str_.push_back(last_msg_.substr(prev_pos, pos - prev_pos));
        }
        full_str_.push_back(last_msg_.substr(pos));

        if(!isDate(full_str_[0])){
            state_ = 2;
            write("Date format is wrong; enter 'add' to make new record*");
        }

        write("The following record for card №" + std::to_string(card_id_) + 
              " will be added; type y/n to add/delete: \n");
        for(auto str : full_str_){
            write(str + "\n\n");
        }
        write("*");
        return;
    }
    else{
        if (last_msg_[0] == 'y' or last_msg_[0] == 'Y'){
            editor.add_record(card_id_, full_str_);
            state_ = 2;
            full_str_.clear();
            write("Record was added*");
        } 
        else{
            state_ = 2;
            full_str_.clear();
            write("Adding was not confirmed*");
        }
    }
}


void AddressClient::write(const std::string& ans) {
        sock_.write_some(boost::asio::buffer(ans));
}

// check format of date
bool AddressClient::isDate(std::string date) {
    if (date.size() != 10) return 0;
    if (isInteger(date.substr(0, 4)) &&
        isInteger(date.substr(5, 2)) && 
        isInteger(date.substr(8, 2)) &&
        date[4] == '-' && date[7] == '-') {
            return 1;
    } 
    else return 0;
}



void acceptClients() {
    using namespace boost::asio;
    ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8001));
    std::cout << "entered accept\n";
    // int serverSize = 1;
    while (true) { 
        AddressClient_ptr new_( new AddressClient);
        acceptor.accept(new_->get_socket());
        std::cout << "Accepted\n";
        
        boost::recursive_mutex::scoped_lock lk(clients_mtx);
        clients.push_back(new_);
    }
}


void handleClients(){
    bool areopen = true;
    std::cout << "entered handleclients\n";
    while (areopen) {
        boost::recursive_mutex::scoped_lock lk(clients_mtx);
        if(clients.size()!= 0){
            for ( std::vector<AddressClient_ptr>::iterator b = clients.begin(), e = clients.end(); b != e; ++b){
                (*b)->answer();
                if ((*b)->getst() == -1){
                    clients.erase(b);
                }
            }
        }
    }
}


inline bool isInteger(const std::string & s) {
    if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))){
       return false;
    }
    char * p;
    strtol(s.c_str(), &p, 10);
    return (*p == 0);
}