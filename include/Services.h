//
// Created by dawid on 20.04.22.
//

#pragma once


#include <db_helper/DBHelper.h>

class Services {
    DBHelper db_helper;

    const std::string table_services = "services";
    const std::string col_id = "id";
    const std::string col_service = "service";
    const std::string col_status = "status";
    const std::string col_firebase_token = "firebase_token";

public:
    Services();

    ~Services() = default;

    /// @brief enables firebase notifications for service
    void add_service(const std::string &service);

    void connect_service(const std::string &service);

    void disconnect_service(const std::string &service);

    void disconnect_services();

    void add_to_service(const std::string &service, int client_id, const std::string &firebase_token);

    bool subscribed(const std::string &service, int client_id);

    void remove_from_service(const std::string &service, int client_id);

    void remove_from_services(int client_id);

    std::vector<std::string> get_firebase_tokens(const std::string &service);

    void update_firebase_token(int id, const std::string &new_firebase_token);

    std::vector<std::string> get_services();
};
