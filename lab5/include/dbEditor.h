#ifndef DBEDITOR_H
#define DBEDITOR_H


#include <sqlite3.h>
#include <boost/thread.hpp>
#include <vector>
#include <string>

static boost::recursive_mutex db_mtx;

using Record = std::vector<std::string>;
using Records = std::vector<Record>;

static int callback(void* data, int argc, char** argv, char** azColName);

class dbEditor{
    private:
    sqlite3* db_;
    bool opened_;

    Records select_stmt(const std::string& stmt, bool flag);

    public:
    dbEditor();

    int login(const Record& in);
    
    int choose(const Record& in);

    Records view_info(int card_id);

    bool add_record(int id_card, const Record& in);

    Records view_records(Record in, int card_id);

    ~dbEditor(){
        sqlite3_close(db_);
    }

};


#endif // DBEDITOR_H
