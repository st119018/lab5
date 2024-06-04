#include "server.h"

#include <string>
#include <iostream>

dbEditor editor;                        // object for working with db
boost::asio::io_service service;

boost::asio::ip::tcp::socket& Server::get_socket(){ 
    return sock_; 
}

Server::~Server(){
    sock_.close();
    std::lock_guard<std::mutex> lk(cout_mtx);
    std::cout << "Client disconnected\n";
}

void Server::run(){
    while(st_){
        answer();
    }
}

bool Server::get_st() const{
    return st_;
}

void Server::answer(){
    try {
        if (sock_.available()){
            // reading from client
            already_read_ += sock_.read_some(boost::asio::buffer(buffer_ + already_read_, 
                                            bufferSize - already_read_));
        }
        
        processRequest();
        
    } catch (boost::system::system_error& ) {
        {
            std::lock_guard<std::mutex> lk(cout_mtx);
            std::cerr << "Error answering\n";
        }
        st_ = false;
    }
}

void Server::processRequest(){
    
    if (!findStar()) {
        return; // message is not full
    }
    // getting message from client
    last_msg_ = getmsg();
    
    // answering
    // 1st word from message and current state 
    // determine what server will do
    if(last_msg_.find("quit") == 0){
        quit();
        return;
    }
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
                view_record();
            }
            else if(last_msg_.find("choose") == 0) {
                card_id_ = 0;
                choose_patient();
            }
            else write("You can enter only 'info', 'add', 'view' or 'choose'; try again*");
            break;
        
        case 3:
            add_record();
            break;
            
        default:
            {
                std::lock_guard<std::mutex> lk(cout_mtx);
                std::cout << "Error: wrong state\n";
            }
            full_.clear();
            write("Smth went wrong on server side; try again*");      
    }
}

bool Server::findStar() {
    // finding end of message('*')
    bool found = std::find(buffer_, buffer_ + already_read_, '*') < buffer_ + already_read_;
    return found;
}

std::string Server::getmsg() {
    std::size_t pos = std::find(buffer_, buffer_ + already_read_, '*') - buffer_;
    // getting message from client
    std::string msg(buffer_, pos); 
    // deleting messege from buffer
    std::copy(buffer_ + already_read_, buffer_ + bufferSize, buffer_); 
    already_read_ -= pos + 1;
    return msg;
}

void Server::login(){
    // extracting words from message
    std::stringstream msg{last_msg_};
    msg >> last_msg_ >> last_msg_;
    full_.push_back(last_msg_);
    msg >> last_msg_;
    full_.push_back(last_msg_);
    // make request to db 
    user_id_ = editor.login(full_);
    
    if(user_id_ != 0){
        write("You are logged in. Write 'choose' to choose patient*");
        full_.clear();
        state_ = 1;
    }
    else{
        write("Wrong login or password; enter 'login' and try again*");
        full_.clear();
        state_ = 0;
    }
    
}

void Server::choose_patient(){
    // extracting words
    std::stringstream msg{last_msg_};
    msg >> last_msg_ >> last_msg_;
    full_.push_back(last_msg_);
    // make request to db
    card_id_ = editor.choose(full_);
        
    if (card_id_ != 0){
        write("Patient is chosen. Write 'info', 'add' or 'view'*");
        full_.clear();
        state_ = 2;
    } 
    else {
        full_.clear();
        write("Patient is not found; enter 'choose' and try again*");
    }

}

void Server::view_patient(){
    Records records = editor.view_info(card_id_);
    
    if(!records.empty()){
        if(!records[0].empty()){
            std::string ans = "\n";
            for (auto rec : records[0]){
                ans += rec + "\n";
            }
            full_.clear();
            write(ans + "*");
            state_= 2;
            return;
        }
    }
    write("Data not found; try again*");
    
}

void Server::add_record(){
    if(full_.empty()){
        // extracting substrings from client's message
        std::size_t pos = last_msg_.find("|");
        for (int i= 0; i < 5; ++i){
            std::size_t prev_pos = ++pos;
            pos = last_msg_.find('|', prev_pos);
            full_.push_back(last_msg_.substr(prev_pos, pos - prev_pos));
        }
        full_.push_back(last_msg_.substr(pos + 1));
        
        // ask client for consent to addition
        write("The following record for card №" + std::to_string(card_id_) + 
              " will be added; type y/n to add/delete it: \n");
        for(auto str : full_){
            write(str + "\n");
        }
        write("*");
        return;
    }
    else{
        if (last_msg_[0] == 'y' or last_msg_[0] == 'Y'){ // client agreed to add record
            // adding new record
            bool isAdded = editor.add_record(card_id_, full_);
            
            state_ = 2;
            full_.clear();
            if(isAdded){
                write("Record was added*");
            } else{
                write("Smth went wrong; record wasn't added*");
            }
            
        } 
        else{
            state_ = 2;
            full_.clear();
            write("Adding was not confirmed*");
        }
    }
}

void Server::view_record(){
    // extracting from message
    std::stringstream msg{last_msg_};
    msg >> last_msg_ >> last_msg_;
    full_.push_back(last_msg_);
    msg >> last_msg_;
    full_.push_back(last_msg_);
    // make request to db
    Records records = editor.view_records(full_, card_id_);

    if (!records.empty()){
        std::string ans = "\n";
        for(auto recs: records){
            for (auto rec : recs){
                ans += rec + "\n";
            }
            // separate records
            ans += "-----------------------\n";
        }
        full_.clear();
        write(ans + "*");
    } 
    else{
        full_.clear();
        write("Records not found*");
    }
}

void Server::quit(){
    write("You are disconnected\n*");
    st_ = 0;
}

void Server::write(const std::string& ans) {
    sock_.write_some(boost::asio::buffer(ans));
}
