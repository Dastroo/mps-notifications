//
// Created by dawid on 28.11.2021.
//

#pragma once

#include <string>
#include <db_helper/DBHelper.h>
#include <jsoncpp/json/writer.h>


class Firebase {
    const std::string TAG = "Firebase";

    std::string server_key;
    DBHelper db_helper;
    Json::FastWriter writer;
    int connect_timeout = 3; // in seconds

    const std::string table_modules = "modules";
    const std::string col_module = "module";
    const std::string col_id = "id";
    const std::string col_firebase_token = "firebase_token";

public:
    Firebase();

    ~Firebase() = default;

    std::string get_services(int id);

    /**
     * @param token client's firebase token
     * @param title notification title
     * @param body notification body
     * @return Json::Value().empty() on error or Json::Value on success
     */
    std::string notify_client(const std::string &token, const std::string &title, const std::string &body);

    void notify_clients(const std::vector<std::string>& firebase_tokens, const std::string &title, const std::string &body);

    void update_token(int id, const std::string &firebase_token);

    static bool validate_firebase_token(const std::string &firebase_token);

private:
    /**
     * @brief https://firebase.google.com/docs/cloud-messaging/http-server-ref
     * @param params android notification details learn more in link above
     * @returns response from google fcm firebase server
     */
    std::string request(const Json::Value &params);

    /**
     *
     * @return modules that are using firebase
     */
    std::vector<std::string> get_modules();
};