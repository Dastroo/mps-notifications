//
// Created by dawid on 28.11.2021.
//

#pragma once

#include <string>
#include <jsoncpp/json/writer.h>


class Firebase {
    const std::string TAG = "Firebase";

    std::string server_key;
    Json::FastWriter writer;
    int connect_timeout = 3; // in seconds

public:
    Firebase();

    ~Firebase() = default;

    /**
     * @param token client's firebase token
     * @param title notification title
     * @param body notification body
     * @return Json::Value().empty() on error or Json::Value on success
     */
    std::string notify_client(const std::string &token, const std::string &title, const std::string &body);

    void
    notify_clients(const std::vector<std::string> &firebase_tokens, const std::string &title, const std::string &body);

    static bool validate_firebase_token(const std::string &firebase_token);

private:
    /**
     * @brief https://firebase.google.com/docs/cloud-messaging/http-server-ref
     * @param params android notification details learn more in link above
     * @returns response from google fcm firebase server
     */
    std::string request(const Json::Value &params);
};