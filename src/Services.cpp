//
// Created by dawid on 20.04.22.
//

#include "../include/Services.h"

Services::Services() {
    //  CREATE TABLE MODULES IF IT DOES NOT EXIST
    if (!db_helper.table_exists(table_services))
        db_helper.create(table_services,
                         col_id, DBHelper::INTEGER, DBHelper::PRIMARY_KEY, DBHelper::AUTO_INCREMENT,
                         col_service, DBHelper::TEXT,
                         col_status, DBHelper::INTEGER); // boolean
}

void Services::add_service(const std::string &service) {
    //  CREATE SERVICE TABLE FOR STORING SUBSCRIBED CLIENTS FIREBASE TOKENS
    if (!db_helper.table_exists(service))
        db_helper.create(service,
                         col_id, DBHelper::INTEGER, DBHelper::PRIMARY_KEY,
                         col_firebase_token, DBHelper::TEXT);

    //  ADD SERVICE TO SERVICES TABLE
    if (!db_helper.exists(table_services, col_service, service))
        db_helper.insert(table_services, col_service, col_status, service, true);
}

void Services::connect_service(const std::string &service) {
    if (db_helper.exists(table_services, col_service, service))
        db_helper.update(table_services, std::make_tuple(col_service, "=", service), col_status, true);
    else
        add_service(service);
}

void Services::disconnect_service(const std::string &service) {
    if (db_helper.exists(table_services, col_service, service))
        db_helper.update(table_services, std::make_tuple(col_service, "=", service), col_status, false);
}

void Services::disconnect_services() {
    for (const auto &service : get_services())
        disconnect_service(service);
}

void Services::add_to_service(const std::string &service, int client_id, const std::string &firebase_token) {
    db_helper.insert(service, col_id, col_firebase_token, client_id, firebase_token);
}

bool Services::subscribed(const std::string &service, int client_id) {
    return db_helper.exists(service, col_id, client_id);
}

void Services::remove_from_service(const std::string &service, int client_id) {
    db_helper.dele(service, col_id, client_id);
}

void Services::remove_from_services(int client_id) {
    for (auto &service: get_services())
        if (subscribed(service, client_id))
            remove_from_service(service, client_id);
}

std::vector<std::string> Services::get_firebase_tokens(const std::string &service) {
    auto query = db_helper.select(service, col_firebase_token);
    std::vector<std::string> modules;
    while (query->executeStep())
        modules.emplace_back(query->getColumn(0).getString());
    return modules;
}

void Services::update_firebase_token(int id, const std::string &new_firebase_token) {
    for (auto &service: get_services())
        if (subscribed(service, id))
            db_helper.update(service, col_id, id, col_firebase_token, new_firebase_token);
}


std::vector<std::string> Services::get_services() {
    auto query = db_helper.select(table_services, col_service);
    std::vector<std::string> modules;
    while (query->executeStep())
        modules.emplace_back(query->getColumn(0).getString());
    return modules;
}
