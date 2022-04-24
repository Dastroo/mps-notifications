//
// Created by dawid on 28.11.2021.
//

//#include "ServerUtils.h"
//#include "Settings.h"

#include <regex>
#include <fstream>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <jsoncpp/json/reader.h>
#include <log_helper/Log.h>
#include <mps_utils/SvrDir.h>

#include "../include/Firebase.h"

//  TODO: change every mention of "module" to "service"
Firebase::Firebase() {
    //  READ SERVER KEY FROM JSON FILE
    Json::Value value;
    std::ifstream ifstream(mps::SvrDir::usr().append("config.json"));
    Json::Reader().parse(ifstream, value, false);
    server_key = value["server_key"].asString();
    Log::i(TAG, "server_key: " + server_key);
}

std::vector<std::string> Firebase::get_modules() {
    auto query = db_helper.select(table_modules, col_module);
    std::vector<std::string> modules;
    while (query->executeStep())
        modules.emplace_back(query->getColumn(col_module.c_str()).getString());
    return modules;
}

std::string    //  TODO: consider changing to bool return type
Firebase::notify_client(const std::string &token, const std::string &title, const std::string &body) {
    Json::Value jor;
    jor["to"] = token;
    jor["priority"] = "high";
    jor["time_to_live"] = 60;
    jor["data"]["title"] = title;
    jor["data"]["body"] = body;

    return request(jor);
}

void Firebase::notify_clients(const std::vector<std::string> &firebase_tokens, const std::string &title,
                              const std::string &body) {
    for (auto &token: firebase_tokens) {
        std::string res = notify_client(token, title, body);
        Log::t(TAG, "notify_clients", "send to:\n\t" + token);
        Log::t(TAG, "notify_clients", "response:\n\t" + res);
    }
}

/**
 * @brief https://firebase.google.com/docs/cloud-messaging/http-server-ref
 * @param params android notification details learn more in link above
 * @returns response from google fcm firebase server
 */
std::string Firebase::request(const Json::Value &params) {
    try {
        curlpp::Easy request;

        //  -G url
        std::string url = "https://fcm.googleapis.com/fcm/send";
        request.setOpt<curlpp::options::Url>(url);

        //  -d parameters
        request.setOpt<curlpp::options::PostFields>(writer.write(params));

        //  -H headers
        std::list<std::string> headers;
        headers.emplace_back("Authorization: key=" + server_key);
        headers.emplace_back("Content-Type: application/json");
        request.setOpt<curlpp::options::HttpHeader>(headers);

        request.setOpt<curlpp::options::ConnectTimeout>(connect_timeout);

        //  get result as string
        std::ostringstream os;
        request.setOpt<curlpp::options::WriteStream>(&os);

        request.perform();

        return os.str();
    } catch (curlpp::RuntimeError &e) {
        Log::e(TAG, "request", "runtime error");
        e.what();
        return {};
    } catch (curlpp::LogicError &e) {
        Log::e(TAG, "request", "logic error");
        e.what();
        return {};
    }
}

bool Firebase::validate_firebase_token(const std::string &firebase_token) {
    if (firebase_token.empty())
        return false;

    if (firebase_token.size() > 4096)
        return false;

    std::regex rx(R"([0-9a-zA-Z\-\_\:]*)");
    if (!std::regex_match(firebase_token, rx))
        return false;

    return true;
}

