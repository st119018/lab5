#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "dbEditor.h"

extern boost::asio::io_service service;
extern dbEditor editor;

class AddressClient;
typedef boost::shared_ptr<AddressClient> AddressClient_ptr;

class AddressClient : public boost::enable_shared_from_this<AddressClient> {
private:
    boost::asio::ip::tcp::socket sock_;
    enum { bufferSize = 1024 };
    int already_read_;
    char buffer_ [bufferSize];
    bool st_; 
    int state_;
    int card_id_;
    int user_id_;
    Record full_;
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

    void answer();

public:
    AddressClient(): sock_(service), st_(1), already_read_(0), state_(0) {};
    
    void run();

    bool get_st() const;

    boost::asio::ip::tcp::socket& get_socket(); 

    ~AddressClient();

};

inline bool isInteger(const std::string & s);

void acceptClients();

void run_client(AddressClient_ptr ptr);

#endif // SERVER_H
