#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "dbEditor.h"


static dbEditor editor;

static boost::asio::io_service service;

class AddressClient;
typedef boost::shared_ptr<AddressClient> AddressClient_ptr;

static std::vector<AddressClient_ptr> clients;
static boost::recursive_mutex clients_mtx;

class AddressClient : public boost::enable_shared_from_this<AddressClient> {
private:
    boost::asio::ip::tcp::socket sock_;
    enum { bufferSize = 1024 };
    int already_read_;
    char buffer_ [bufferSize];
    bool started_;    
    int state_;
    int field_num_;
    int card_id_;
    int user_id_;
    std::vector <std::string> full_str_;
    std::string last_msg_;
    
    void write(const std::string& ans);
    
    bool findStar();

    void beautify_msg();

    void processRequest();

    std::string getmsg();

    void login();

    void choose_patient();

    void view_record();

    void add_record();

    void view_patient();

    bool isDate(std::string date);

public:
    AddressClient(): sock_(service), started_(false), already_read_(0), state_(0), field_num_(0) {};

    void answer();

    boost::asio::ip::tcp::socket & get_socket(); 

    bool getst(){
        return started_;
    }       

};

inline bool isInteger(const std::string & s);

void acceptClients();

void handleClients();

#endif // SERVER_H
