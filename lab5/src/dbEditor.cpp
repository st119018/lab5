#include "dbEditor.h"

#include <iostream>


dbEditor::dbEditor(){
    int rc = sqlite3_open("../sqlite.db", &db_);

    if (rc != SQLITE_OK){
        std::cerr << "Error opening db: %s\n" << sqlite3_errmsg(db_);
        opened_ = false;
        sqlite3_close(db_);
    } 
    else {
        std::cout << "Opened database successfully?\n";
        opened_ = true;
    }
}

int dbEditor::login(const Record& in) {
    if(in.size() != 2) {
        std::cout << "\nError(db): wrong number in vector\n";
        return 0;
    }

    std::string query = "SELECT id FROM doctors WHERE login LIKE '" + in[0] + 
                        "' AND password LIKE '" + in[1] + "';";

    
    Records records = select_stmt(query, 1);

    if(records.size() != 1){ // if number of returned rows != 1
        return 0;
    } else {

        if (!records[0].empty()) return std::stoi(records[0][0]);
        else {
            std::cout << "\n\nerror(?) 1: empty result after login\n\n"; 
            return 0;
        }
    }
    
}

// finds given patient' card id; returns 0 if not found
int dbEditor::choose(const Record& in){
    if(in.size() != 1){
        std::cout << "\nError(db): wrong number in vector\n";
        return 0;
    }

    std::string query = "SELECT id_card FROM patients WHERE snils LIKE '" 
                        + in[0] + "';";  
    Records records = select_stmt(query, 1);

    if(records.size() != 1){ // if number of returned rows != 1
        return 0;
    } 
    else{
        if (!records[0].empty()){
            return std::stoi(records[0][0]);
        }
        else {
            std::cout << "\n\nerror(?) 1: empty result after choose??\n\n"; 
            return 0;
        }
    }
}

Records dbEditor::view_info(int card_id){
    std::string query = "SELECT * FROM patients WHERE id_card = " 
                        + std::to_string(card_id) + ";";  

    Records records = select_stmt(query, 0);/////////////////

    return records;
}

bool dbEditor::add_record(int id_card, const Record& in){
    std::string query = "INSERT INTO records(id_card, exam_date, complaints, "
                        "observation, consultion, medication, doctor) "
                        "VALUES(" + std::to_string(id_card);
    for(int i = 0; i < 5; ++i){
        query += ", '" + in[i] + "'";
    }
    query += ");";
    char* errmsg;
    int ret = sqlite3_exec(db_, query.data(), 0, 0, &errmsg);
    
    if (ret != SQLITE_OK) {
        std::cerr << "Error in inset statement " << "[" << errmsg << "]\n";
        sqlite3_free(errmsg);
        return 0;
    } 
    else{
        return 1;
    }

}

Records dbEditor::view_records(Record in, int card_id){
    Records records;
    
    if(in.size() == 2){
        std::string query;
        if(in[0] == "all"){
            query = "SELECT exam_date, complaints, observation, consultion, "
            "medication, doctor FROM records WHERE id_card = " + 
            std::to_string(card_id) + ";";
        }
        else{
            query = "SELECT exam_date, complaints, observation, consultion, "
            "medication, doctor FROM records WHERE id_card = " + std::to_string(card_id) + 
            "AND exam_date > '" + in[0] + "' AND exam_date < '" + in[1] + "';";
        }
        records = select_stmt(query, 0);
    }
    return records;
}

static int callback(void* data, int argc, char** argv, char** azColName){
    Records* records = static_cast<Records*>(data);
    try {
        records->emplace_back(argv, argv + argc);
    }
    catch (std::exception const& e){
        std::cout << "Exception: " << e.what() << "\n\n";
        return 1;
    }
    return 0;
}

static int callbackv2(void* data, int argc, char** argv, char** azColName){
    Records* records = static_cast<Records*>(data);
    try {
        Record record(argc);
        for(int i = 0; i < argc; ++i){
            std::string st = std::string(azColName[i]);
            st += ": ";
            st += argv[i] ? argv[i] : "-";
            record[i] = st;
        }
        records->emplace_back(record);
    }
    catch (std::exception const& e){
        std::cout << "Exception: " << e.what() << "\n\n";
        return 1;
    }
    return 0;
}


Records dbEditor::select_stmt(const std::string& stmt, bool flag){
    Records records;  
    char *errmsg;
    int ret;
    if(flag) ret = sqlite3_exec(db_, stmt.data(), callback, &records, &errmsg);
    else ret = sqlite3_exec(db_, stmt.data(), callbackv2, &records, &errmsg);
    
    if (ret != SQLITE_OK){
        std::cerr << "Error in select statement " << stmt << "[" << errmsg << "]\n";
        sqlite3_free(errmsg);
    }
    else{
        std::cerr << records.size() << " records returned successfully from select_stmt.\n";/////////////////////////////
    }

    return records;
}

