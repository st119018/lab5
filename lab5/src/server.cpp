#include "server.h"

#include <string>
#include <iostream>

dbEditor editor;
boost::asio::io_service service;
std::vector<AddressClient_ptr> clients;
boost::mutex cl_mtx;
boost::condition_variable cv;
bool changed{false};

boost::asio::ip::tcp::socket& AddressClient::get_socket(){ 
    return sock_; 
}

AddressClient::~AddressClient(){
    sock_.close();
}

void AddressClient::run(){
    while(st_){
        answer();
    }
}

bool AddressClient::get_st() const
{
    return st_;
}

void AddressClient::answer()
{
    try {
        if (sock_.available()){
        already_read_ += sock_.read_some(boost::asio::buffer(buffer_ + already_read_, bufferSize - already_read_));
        }
        
        processRequest();
        
    } catch (boost::system::system_error& ) {
        std::cerr << "Error answering\n";
        st_ = false;
    }
}

void AddressClient::processRequest(){
    
    if (!findStar()) {
        return; // message is not full
    }
    
    last_msg_ = getmsg();

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
            std::cout << "Error: wrong state!!!\n";
            full_str_.clear();
            write("Smth went wrong on server side; try again*");      
    }
}

bool AddressClient::findStar() {
    bool found = std::find(buffer_, buffer_ + already_read_, '*') < buffer_ + already_read_;
    return found;
}

std::string AddressClient::getmsg() {
    std::size_t pos = std::find(buffer_, buffer_ + already_read_, '*') - buffer_;
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
        write("You are logged in. Write 'choose' to choose patient*");
        full_str_.clear();
        state_ = 1;
    }
    else{
        write("Wrong login or password; enter 'login' and try again*");
        full_str_.clear();
        state_ = 0;
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
        write("Patient is not found; enter 'choose' and try again*");
    }

}

void AddressClient::view_patient(){
    Records records = editor.view_info(card_id_);
    
    if(!records.empty()){
        if(!records[0].empty()){
            std::string ans{};
            for (auto rec : records[0]){
                ans += rec + "\n";
            }
            full_str_.clear();
            write(ans + "*");
            state_= 2;
            return;
        }
    }
    write("Data not found; try again*");
    
}

void AddressClient::add_record(){
    if(full_str_.empty()){
        // extracting substrings from client's message
        std::size_t pos = last_msg_.find("|");
        for (int i= 0; i < 5; ++i){
            std::size_t prev_pos = ++pos;
            pos = last_msg_.find('|', prev_pos);
            full_str_.push_back(last_msg_.substr(prev_pos, pos - prev_pos));
        }
        full_str_.push_back(last_msg_.substr(pos + 1));

        write("The following record for card №" + std::to_string(card_id_) + 
              " will be added; type y/n to add/delete it: \n");
        for(auto str : full_str_){
            write(str + "\n");
        }
        write("*");
        return;
    }
    else{
        if (last_msg_[0] == 'y' or last_msg_[0] == 'Y'){            
            bool isAdded = editor.add_record(card_id_, full_str_);
            
            state_ = 2;
            full_str_.clear();
            if(isAdded){
                write("Record was added*");
            } else{
                write("Smth went wrong; record wasn't added*");
            }
            
        } 
        else{
            state_ = 2;
            full_str_.clear();
            write("Adding was not confirmed*");
        }
    }
}

void AddressClient::view_record(){
    std::stringstream msg{last_msg_};
    msg >> last_msg_ >> last_msg_;
    full_str_.push_back(last_msg_);
    msg >> last_msg_;
    full_str_.push_back(last_msg_);

    Records records = editor.view_records(full_str_, card_id_);

    if (!records.empty()){
        std::string ans{};
        for(auto recs: records){
            for (auto rec : recs){
                ans += rec + "\n";
            }
            // separate records
            ans += "-----------------------\n";
        }
        full_str_.clear();
        write(ans + "*");
    } 
    else{
        full_str_.clear();
        write("Records not found*");
    }
}

void AddressClient::quit(){
    write("You are disconnected\n*");
    st_ = 0;
}

void AddressClient::write(const std::string& ans) {
    sock_.write_some(boost::asio::buffer(ans));
}

void acceptClients() {
    using namespace boost::asio;
    ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8001));
    std::vector<std::thread> threads;
    while (true){ 
        AddressClient_ptr new_(new AddressClient);
        acceptor.accept(new_->get_socket());
        std::cout << "Accepted\n";        
        threads.emplace_back(run_client, new_);
        boost::lock_guard<boost::mutex> lk(cl_mtx);
        clients.push_back(new_);
    }
    for(auto &th : threads){
        th.join();
    }
    
}

void run_client(AddressClient_ptr ptr){
    ptr->run();
    {
        boost::lock_guard<boost::mutex> lk(cl_mtx);
        changed = true;
    }
    cv.notify_one();
}

void delClients(){
    while(true){
        boost::unique_lock<boost::mutex> lk(cl_mtx);
        cv.wait(lk, []{return changed;});

        for(auto cl = clients.begin(), e = clients.end(); cl != e; ++cl){
            if(!(*cl)->get_st()){
                clients.erase(cl);
            }
        }
        changed = false;
    }
}
