#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "dbEditor.h"

extern boost::asio::io_service service;

class AddressClient;
typedef boost::shared_ptr<AddressClient> AddressClient_ptr;

class AddressClient : public boost::enable_shared_from_this<AddressClient> {
private:
    boost::asio::ip::tcp::socket sock_;
    enum { bufferSize = 1024 };
    int already_read_;
    char buffer_ [bufferSize];
    bool started_;    
    int state_;
    int card_id_;
    int user_id_;
    Record full_str_;
    std::string last_msg_;
    
    void write(const std::string& ans);
    
    bool findStar();

    void processRequest();

    std::string getmsg();

    void login();

    void choose_patient();

    void view_record();

    void add_record();

    void view_patient();

    void quit();

public:
    AddressClient(): sock_(service), started_(true), already_read_(0), state_(0) {};

    void answer();

    boost::asio::ip::tcp::socket & get_socket(); 

    bool get_started();

    ~AddressClient(){
        sock_.close();
    }

};

inline bool isInteger(const std::string & s);

void acceptClients();

void handleClients();

#endif // SERVER_H
